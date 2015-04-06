//
//  fpoint.h
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/20/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#ifndef __pointcloud__fpoint__
#define __pointcloud__fpoint__

#include <stdio.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <unordered_map>
#include "imageholder.h"
#include "utility.h"
#include <map>
#include <iostream>
#include <fstream>

using namespace std;

enum fpoint_type:uint{
    STATUS_NO_POS,
    STATUS_TRIGULATED,
    STATUS_HAVE_POS
};

class fpoint{
public:
    //Use for getImageHolder
    vector<imageholder*> *all_pano;
    imageholder* getImageHolder(string name);
    
    fpoint(int id, vector<imageholder*> *all_pano_);
    //map: imageholder + heading,pitch from that imageholder
    
    void setStatus(uint status);
    uint getStatus();
    void setID(int id);
    int getID();
    void setPosition(cv::Point3d pos);
    cv::Point3d getPosition();
    
    unsigned long matchSize();
    double calcError(int id1, int id2);
    void listMatch();
    void addHP(imageholder* pano, double heading, double pitch);
    void addHP(string pano, cv::Point2d hp);
    bool remove(string pano);
    void clear();
    // method: solve triangulation: find position
    void triangulate();
    
    void saveData(std::ofstream& output);
    void loadData(std::ifstream& input);
private:
    map<string, cv::Point2d> match;
    cv::Point3d position;
    int id;
    int status; // solved or unsolved (triangulation) {0 = is not triangulated, 1 = triangulated}
};


#endif /* defined(__pointcloud__fpoint__) */