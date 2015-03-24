//
//  image_holder.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/18/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "imageholder.h"
#include "utility.h"
using namespace std;


// Will create class ImageHolder {

#define WIDTH 640
#define HEIGHT 640


#pragma mark Helper Function

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

    // Get color from the position in image. Right now I floor the value down. May change method later
    cv::Vec3b getColorHP(cv::Mat m, double heading, double pitch, double fov, double width, double height){
        normalizeHP(&heading, &pitch);
        double py = angleToPx(pitch,fov,height);
        double px = angleToPx(heading,fov,width);
        if(py<height&&py>=0&&px<width&&px>0) return m.at<cv::Vec3b>((int)py,(int)px);
        else return cv::Vec3b(0,0,0);
    }

#pragma mark main code

    imageholder::imageholder(double fov_in, string path){
        fraction = 1;
        fov = fov_in;
        int size = (int)ceil(360*2/fov);
        int size2 = (int)ceil(180*2/fov) + 1;
        holder = new cv::Mat*[size];
        for(int i=0;i<size;i++){
            holder[i] = new cv::Mat[size2];
            for(int j=0; j<size2; j++){
                int heading = (int)indexToHeading(i,fov);
                int pitch = (int)indexToPitch(j,fov);
                string name = path+to_string(pitch)+string(",")+to_string(heading)+string(".jpg");
                //cout<<"Reading: "<<name<<endl;
                holder[i][size2-j-1] = cv::imread(name);
                //if fail to read the image
                if ( holder[i][size2-j-1].empty() ){
                    cout << "Error loading the image: " << name;
                }
                
            }
        }
        
        relative_x = 0;
        relative_y = 0;
        cout << " : Reading Complete!"<<endl;
    }

    cv::Mat imageholder::getImage(double heading, double pitch){
        
        normalizeHP(&heading, &pitch);
        int i_h = headingToClosestIndex(heading, fov);
        int i_p = pitchToClosestIndex(pitch, fov);
        cv::Mat img = holder[i_h][i_p];
        return img;
    }

    cv::Vec3b imageholder::getImageColorHP(double heading, double pitch){
        normalizeHP(&heading, &pitch);
        int i_h = headingToClosestIndex(heading, fov);
        int i_p = pitchToClosestIndex(pitch, fov);
        double x,y,z;
        
        // Transform heading, pitch to base heading, base pitch
        HPtoLCS(heading, pitch, &x, &y, &z);
        rotateLCS(&x,&y,&z,indexToHeading(i_h, fov),- indexToHeading(i_h, fov),- indexToPitch(i_p, fov));
        heading = getHeading(x,y,z);
        pitch = getPitch(x,y,z);
        
        cv::Mat img = holder[i_h][i_p];
        cv::Vec3b color = getColorHP(img, heading, pitch, fov, WIDTH, HEIGHT);
        
        return color;
    }

    void imageholder::setRelativePos(imageholder *base_img){
        double b_lat = base_img->getLat();
        double b_lng = base_img->getLng();
        relative_x = utility::distance(b_lat, b_lng, lat, b_lng);
        relative_y = utility::distance(b_lat, b_lng, b_lat, lng);
    }


    // Calculate the heading/pitch of the following coordinate based on the position of this pano
    double imageholder::computeHeading(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z;
        double heading = getHeading(rx, ry, rz);
        return heading;
    }
    double imageholder::computePitch(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z;
        double pitch = getPitch(rx, ry, rz);
        return pitch;
    }

#pragma mark getter/setter

    void imageholder::setName(string name_){
        name = name_;
    }

    void imageholder::setPos(double lat_, double lng_){
        lat = lat_;
        lng = lng_;
    }

    double imageholder::getLat(){
        return lat;
    }

    double imageholder::getLng(){
        return lng;
    }

    double imageholder::getRelativeX(){
        return relative_x;
    }

    double imageholder::getRelativeY(){
        return relative_y;
    }

    string imageholder::getName(){
        return name;
    }