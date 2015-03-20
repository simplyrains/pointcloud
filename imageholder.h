//
//  image_holder.h
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/18/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#ifndef __pointcloud__image_holder__
#define __pointcloud__image_holder__

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#endif /* defined(__pointcloud__image_holder__) */


using namespace std;

class imageholder {
public:
    
    imageholder(double fov_in,string path);                         // constructor; initialize the list to be empty
    //void AddToEnd(int k);              // add k to the end of the list
    //void Print(ostream &output) const; // print the list to output
    
    cv::Vec3b getImageColorHP(double heading, double pitch);
    void setPos(double lat_, double lng_);
    void setName(string name);
    double getLat();
    double getLng();
    string getName();
    cv::Mat getImage(double heading, double pitch);
private:
    double lat;
    double lng;
    double fov;
    string name;
    cv::Mat **holder;
    double fraction;
    //static const int SIZE = 10;      // initial size of the array
};