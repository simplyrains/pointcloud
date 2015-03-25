//
//  fpoint.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/20/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "fpoint.h"

fpoint::fpoint(int id){
    this->id = id;
    this->status = STATUS_NO_POS;
}

void fpoint::addHP(imageholder* pano, double heading, double pitch){
    cv::Point2d hp(heading, pitch);
    addHP(pano, hp);
}

void fpoint::addHP(imageholder* pano, cv::Point2d hp){
    if(!(match.insert(make_pair(pano,hp))).second){
        match[pano] = hp;
    }
}

bool fpoint::remove(imageholder* pano){
    auto result = match.find(pano);
    if (result != match.end()) {
        match.erase(result);
        return true;
    }
    return false;
}

void fpoint::listMatch(){
    cout<<"Listing match for P"<<id<<":"<<endl;
    if(!match.empty())
    for(auto iter=match.begin(); iter!=match.end(); ++iter) {
        cout << iter->first->getName() << "/" <<iter->second << std::endl;
    }
    else cout<<"<EMPTY>"<<endl;
    cout<<endl;
}

void fpoint::triangulate(){
    //TODO: FILL IN SOMETHING
    this->status = STATUS_TRIGULATED;
}

void fpoint::setStatus(uint status_){
    status = status_;
}


uint fpoint::getStatus(){
    return status;
}

void fpoint::setPosition(cv::Point3d pos){
    position = pos;
    status = STATUS_HAVE_POS;
}

cv::Point3d fpoint::getPosition(){
    return position;
}