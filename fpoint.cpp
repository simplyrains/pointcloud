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
    cout<<"{ TRIANGULATE POINT"<<id<<" }"<<endl;
    triangulate();
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
    
    if(match.size()>=2){
        cv::Point3d* dir[match.size()];
        cv::Point3d* st[match.size()];
        int i = 0;
        for(auto iter=match.begin(); iter!=match.end(); ++iter) {
            cout << iter->first->getName() << "/" <<iter->second << std::endl;
            cv::Point3d start;
            imageholder *imh = iter->first;
            start.x = imh->getRelativeX();
            start.y = imh->getRelativeY();
            start.z = 0;
            double heading = iter->second.x;
            double pitch = iter->second.x;
            double x,y,z;
            utility::HPtoLCS(heading, pitch, &x, &y, &z);
            cv::Point3d direction(x,y,z);
            dir[i] = &direction;
            st[i] = &start;
            i++;
            cout<<i<<"\tSTART: "<<start<<"\tDIR: "<<direction<<endl;
        }
        //Currently triangulate from point 1 and 2
        //TODO: change to triangulate from all point
        int j;
        i=0; j=1;
        cv::Point3d u = *dir[0];
        cv::Point3d v = *dir[1];
        cv::Point3d w = *(st[0])-*(st[1]);
        double a = u.x*u.x + u.y*u.y + u.z*u.z;
        double b = u.x*v.x + u.y*v.y + u.z*v.z;
        double c = v.x*v.x + v.y*v.y + v.z*v.z;
        double d = u.x*w.x + u.y*w.y + u.z*w.z;
        double e = v.x*w.x + v.y*w.y + v.z*w.z;
        double c1 = (b*e-c*d)/(a*c-b*b);
        double c2 = (a*e-b*d)/(a*c-b*b);
        cv::Point3d pc = *st[0] + (*dir[0])*c1;
        cv::Point3d qc = *st[1] + (*dir[1])*c2;
        cv::Point3d finalPos = pc*(0.5)+qc*(0.5);
        setPosition(finalPos);
        this->status = STATUS_TRIGULATED;
    }
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