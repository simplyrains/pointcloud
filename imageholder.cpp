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

    // Get color from the position in image. Right now I floor the value down. May change method later
    cv::Vec3b getColorHP(cv::Mat m, double heading, double pitch, double fov, double width, double height){
        utility::normalizeHP(&heading, &pitch);
        double py = utility::angleToPx(pitch,fov,height);
        double px = utility::angleToPx(heading,fov,width);
        if(py<height&&py>=0&&px<width&&px>0) return m.at<cv::Vec3b>((int)py,(int)px);
        else return cv::Vec3b(0,0,0);
    }

#pragma mark main code

    imageholder::imageholder(double fov_in, string path, int id_){
        fraction = 1;
        fov = fov_in;
        int size = (int)ceil(360*2/fov);
        int size2 = (int)ceil(180*2/fov) + 1;
        holder = new cv::Mat*[size];
        for(int i=0;i<size;i++){
            holder[i] = new cv::Mat[size2];
            for(int j=0; j<size2; j++){
                int heading = (int)utility::indexToHeading(i,fov);
                int pitch = (int)utility::indexToPitch(j,fov);
                string name = path+to_string(pitch)+string(",")+to_string(heading)+string(".jpg");
                //cout<<"Reading: "<<name<<endl;
                holder[i][size2-j-1] = cv::imread(name);
                //if fail to read the image
                if ( holder[i][size2-j-1].empty() ){
                    cout << "Error loading the image: " << name << " from "<<path<<endl;
                    holder[i][size2-j-1] =cv::Mat(640, 640, CV_8UC3);
                }
                
            }
        }
        relative_heading = 0;
        relative_x = 0;
        relative_y = 0;
        id = id_;
        cout << " : Reading Complete!"<<endl;
    }

    cv::Mat imageholder::getImage(double heading, double pitch){
        
        utility::normalizeHP(&heading, &pitch);
        int i_h = utility::headingToClosestIndex(heading, fov);
        int i_p = utility::pitchToClosestIndex(pitch, fov);
        cv::Mat img = holder[i_h][i_p];
        return img;
    }

    cv::Vec3b imageholder::getImageColorHP(double heading, double pitch){
        // change from -90,90 to 90,-90
        pitch = -pitch;
        utility::normalizeHP(&heading, &pitch);
        int i_h = utility::headingToClosestIndex(heading, fov);
        int i_p = utility::pitchToClosestIndex(pitch, fov);
        double x,y,z;
        
        // Transform heading, pitch to base heading, base pitch
        utility::HPtoLCS(heading, pitch, &x, &y, &z);
        utility::rotateLCS(&x,&y,&z,utility::indexToHeading(i_h, fov),- utility::indexToHeading(i_h, fov),- utility::indexToPitch(i_p, fov));
        heading = utility::getHeading(x,y,z);
        pitch = utility::getPitch(x,y,z);
        
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

    //computeHeading,computePitch, and is_projectable needs to take relative_x,relative_y, and relative_heading into account
    // ^ used to triangulate

    // Calculate the heading/pitch of the following coordinate based on the position of this pano
    double imageholder::computeHeading(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z;
        double heading = relative_heading + utility::getHeading(rx, ry, rz);
        return heading;
    }
    double imageholder::computePitch(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z;
        double pitch = relative_heading + utility::getPitch(rx, ry, rz);
        return pitch;
    }

    // return false if the coordinate is too close to the pano center
    bool imageholder::is_projectable(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z - relative_y;
        if(rx*rx+ry*ry+rz*rz < 1.0) return 0;
        return 1;
    }

#pragma mark save

    void imageholder::saveData(ofstream &output){
        output << "i " << name << " " << relative_x << " " << relative_y << " " << relative_heading << endl;
    }

    void imageholder::loadData(ifstream &input){
        //output << "i " << name << " " << relative_x << " " << relative_y << " " << relative_heading << endl;
        double x, y, h;
        input >> x;
        input >> y;
        input >> h;
        setRelativePos(x, y);
        //setRelativeHeading(h);
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

    void imageholder::setRelativePos(double x, double y){
        relative_x = x;
        relative_y = y;
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

    int imageholder::getID(){
        return id;
    }

    void imageholder::setID(int id_){
        id = id_;
    }

//    void imageholder::setRelativeH(double heading_){
//        relative_heading = heading_;
//    }
//
//    double imageholder::getRelativeH(){
//        return relative_heading;
//    }