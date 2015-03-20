//
//  fpoint.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/20/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "fpoint.h"

fpoint::fpoint(string name){
    this->name = name;
}

//void fpoint::addHP(imageholder pano, double heading, double pitch){
//    cv::Point2d hp(heading, pitch);
//    addHP(pano, hp);
//}
//
//void fpoint::addHP(imageholder pano, cv::Point2d hp){
//    match.insert(make_pair(pano,hp));
//}
//
//void fpoint::listMatch(){
//    for(auto iter=match.begin(); iter!=match.end(); ++iter) {
//        cout << iter->first << "/" <<iter->second << std::endl;
//    }
//}