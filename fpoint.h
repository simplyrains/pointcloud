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

using namespace std;

enum fpoint_type:uint{
    STATUS_NO_POS,
    STATUS_TRIGULATED,
    STATUS_HAVE_POS
};

class fpoint{
public:
    fpoint(int id);
    //map: imageholder + heading,pitch from that imageholder
    map<imageholder*, cv::Point2d> match;
    string name;
    int id;
    int status; // solved or unsolved (triangulation) {0 = is not triangulated, 1 = triangulated}
    // method: add new match (imageholder, heading, pitch); << check with the exising map
    
    void setStatus(uint status);
    uint getStatus();
    void setPosition(cv::Point3d pos);
    cv::Point3d getPosition();
    
    double calcError(string name1, string name2);
    void listMatch();
    void addHP(imageholder* pano, double heading, double pitch);
    void addHP(imageholder* pano, cv::Point2d hp);
    bool remove(imageholder* pano);
    void clear();
    // method: solve triangulation: find position
    void triangulate();
private:
    cv::Point3d position;
};


#endif /* defined(__pointcloud__fpoint__) */