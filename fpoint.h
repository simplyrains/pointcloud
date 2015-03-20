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

#endif /* defined(__pointcloud__fpoint__) */

using namespace std;

class fpoint{
public:
    fpoint(string name);
    //map: imageholder + heading,pitch from that imageholder
    cv::Point3d position;
    string name;
    int id;
    int status; // solved or unsolved (triangulation)
    // method: add new match (imageholder, heading, pitch); << check with the exising map
    
    
    // method: solve triangulation: find position
    // method: getheading (imageholder x) [if triangulated] calculate heading of this point if it were to show up on imageholder x
    // method: getpitch (imageholder x) [if triangulated] calculate pitch of this point if it were to show up on imageholder x
private:
};