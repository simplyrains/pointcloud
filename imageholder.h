//
//  image_holder.h
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/18/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#ifndef __pointcloud__imageholder__
#define __pointcloud__imageholder__

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

class imageholder {
public:
    
    imageholder(double fov_in,string path, int id_, double hfov, double pfov, double multiplication);                         // constructor; initialize the list to be empty
    //void AddToEnd(int k);              // add k to the end of the list
    //void Print(ostream &output) const; // print the list to output
    
    cv::Vec3b getImageColorHP(double heading, double pitch);
    //get the Mat that matches the following heading/pitch
    cv::Mat getImage(double heading, double pitch);
    
    // Set the Latitude/Longitude of this pano
    void setPos(double lat_, double lng_);
    void setName(string name);
    double getLat();
    double getLng();
    string getName();
    void setID(int id_);
    int getID();
    
    // Set the relative position (meter) of this pano based on the first pano
    void setRelativePos(imageholder* base_img);
    
    double getRelativeX();
    double getRelativeY();
    //double getRelativeH();
    void setRelativePos(double x, double y);
    //void setRelativeH(double heading_);
    cv::Mat getRendered();
    vector<cv::KeyPoint> getKeypoints();
    vector<cv::Point2d> getKeyPointLocation();
    
    // Compute the heading/pitch of the following coordinate based on this pano
    double computeHeading(double x, double y, double z);
    double computePitch(double x, double y, double z);
    bool is_projectable(double x, double y, double z);
    
    void saveData(std::ofstream& output);
    void loadData(std::ifstream& input);
    // Compute the 128 dimension SIFT descriptor at each keypoint.
    // Each row in "descriptors" correspond to the SIFT descriptor for each keypoint
    cv::Mat descriptors;

private:
    double lat;
    double lng;
    int id;
    double fov;
    cv::Mat **holder;
    double fraction;
    cv::Mat rendered;
    
    vector<cv::KeyPoint> keypoints;
    vector<cv::Point2d> keypointsHP;
    void prerender(double hfov, double pfov, double multiplication);
    
    double relative_x;
    double relative_y;
    double relative_heading;
    string name;
    //static const int SIZE = 10;      // initial size of the array
};


#endif /* defined(__pointcloud__imageholder__) */