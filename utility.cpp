//
//  utility.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/23/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "utility.h"
#include "math.h"

namespace utility
{
    double degToRad(double degree){
        return degree*M_PI/180;
    }
    
    //Calculate distance using ’haversine’ formula
    //Info: http://www.movable-type.co.uk/scripts/latlong.html
    double distance(double lat1, double lng1, double lat2, double lng2){
        //code
        double R = 6371; // Radius of the earth in km
        double dLat = degToRad(lat2-lat1);  // deg2rad below
        double dLon = degToRad(lng2-lng1);
        double a =
            sin(dLat/2) * sin(dLat/2) +
            cos(degToRad(lat1)) * cos(degToRad(lat2)) *
            sin(dLon/2) * sin(dLon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double d = R * c * 1000; // Distance in m
        return d;
    }
    void function2()
    {
        //code
    }
    void function3()
    {
        //code
    }
    
    void normalizeHP(double *heading, double *pitch){
        while(*heading>360) *heading-=360;
        while(*heading<0) *heading+=360;
        if(*pitch>90) *pitch=90;
        if(*pitch<-90) *pitch=-90;
    }
    
    double getHeading(double x, double y, double z){
        double size = sqrt(x*x+y*y);
        return atan2(y/size,x/size)*180/M_PI;
    }
    
    double getPitch(double x, double y, double z){
        double size = sqrt(x*x+y*y+z*z);
        return asin(z/size)*180/M_PI;
    }
    
    void rotateZ(double *x, double *y, double *z, double angle){
        double nx = *x, ny = *y, nz = *z;
        double a = angle*M_PI/180;
        *x = nx*cos(a)-ny*sin(a);
        *y = nx*sin(a)+ny*cos(a);
        *z = nz;
    }
    
    void rotateY(double *x, double *y, double *z, double angle){
        double nx = *x, ny = *y, nz = *z;
        double a = -angle*M_PI/180;
        *x = nx*cos(a)+nz*sin(a);
        *y = ny;
        *z = -nx*sin(a)+nz*cos(a);
    }
    
    // Use to get index for the image with closest heading/pitch
    int pitchToClosestIndex(double angle, double fov){
        angle = -angle+fov/4;
        angle = angle+90; // -90~90 -> 0~180
        return (int)floor(angle*2.0/fov);
    }
    double indexToPitch(int index, double fov){
        double angle = index*fov/2;
        angle -=90;
        return -angle;
    }
    int headingToClosestIndex(double angle, double fov){
        angle = angle+fov/4+360;
        if(angle<0) angle+=360;
        int result = (int)floor(angle*2/fov);
        int numberOfIndex = (int)ceil(360*2/fov);
        // There's no 360, we need to change that to 0 instead
        return (result+numberOfIndex)%numberOfIndex;
    }
    double indexToHeading(int index, double fov){
        double angle = index*fov/2;
        if(angle>360) angle-=360;
        return angle;
    }
    
    void rotateLCS(double *x, double *y, double *z, double heading_base, double heading, double pitch){
        rotateZ(x,y,z, -heading_base);
        rotateY(x,y,z, pitch);
        rotateZ(x,y,z, heading_base+heading);
    }
    
    void HPtoLCS(double heading, double pitch, double *x, double *y, double *z){
        *x=1;
        *y=0;
        *z=0;
        rotateLCS(x,y,z,0,heading, pitch);
    }
    
    double angleToPx(double angle, double fov, int imagewh){
        //return angle*imagewh/fov+imagewh/2;
        double f_median = fov*M_PI/180;
        double a_median = angle*M_PI/180;
        double k = imagewh/2/tan(f_median/2);
        return imagewh/2 + k*tan(a_median);
        //return imagewh/2 + k*tan(a_median*fraction);
    }
    
    //is working, but nobody use this. keep it just in case ;P
    double pxToAngle(double px, double fov, int imagewh){
        double f_median = fov*M_PI/180;
        double k = imagewh/2/tan(f_median/2);
        
        double angle = atan((px-imagewh/2)/k);
        
        double a_median = angle*180/M_PI;
        return a_median;
    }
    
    
    double calcDistanceBetweenLines(cv::Point3d u, cv::Point3d v, cv::Point3d f, cv::Point3d k, cv::Point3d *center){
        cv::Point3d w = f-k;
        //cout<<"\tU"<<u<<" V"<<v<<endl;
        //cout<<"\tSU"<<f<<" SV"<<k<<" W"<<w<<endl;
        double a = u.x*u.x + u.y*u.y + u.z*u.z;
        double b = u.x*v.x + u.y*v.y + u.z*v.z;
        double c = v.x*v.x + v.y*v.y + v.z*v.z;
        double d = u.x*w.x + u.y*w.y + u.z*w.z;
        double e = v.x*w.x + v.y*w.y + v.z*w.z;
        double c1 = (b*e-c*d)/(a*c-b*b);
        double c2 = (a*e-b*d)/(a*c-b*b);
        cv::Point3d pc = f + u*c1;
        cv::Point3d qc = k + v*c2;
        *center = pc*(0.5)+qc*(0.5);
        double dx = (pc.x-qc.x);
        double dy = (pc.y-qc.y);
        double dz = (pc.z-qc.z);
        double distance = dx*dx+dy*dy+dz*dz;
        //cout<<"DISTANCE = "<<distance<<endl;
        return distance;
    }

}