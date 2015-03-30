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

void fpoint::clear(){
    match.clear();
}

void fpoint::triangulate(){
    //TODO: FILL IN SOMETHING
    
    if(match.size()>=2){
        cv::Point3d* dir[match.size()];
        cv::Point3d* st[match.size()];
        int i = 0;
        for(auto iter=match.begin(); iter!=match.end(); ++iter) {
            cout << iter->first->getName() << "/" <<iter->second << std::endl;
            
            double x,y,z; //tempolary variable
            imageholder *imh = iter->first;
            x = imh->getRelativeX();
            y = imh->getRelativeY();
            z = 0;
            st[i] = new cv::Point3d(x,y,z);

            double heading = iter->second.x;
            double pitch = iter->second.y;
            utility::HPtoLCS(heading, pitch, &x, &y, &z);
            dir[i] = new cv::Point3d(x,y,z);
            cout<<i<<"\tSTART: "<<*st[i]<<"\tDIR: "<<*dir[i]<<endl;
            i++;
        }
        //Currently triangulate from point 1 and 2
        //TODO: change to triangulate from all point
        
        cv::Point3d avg = cv::Point3d(0,0,0);
        int total = 0;
        for(int i=0; i< match.size(); i++){
            for(int j=0; j< match.size(); j++){
                if(i!=j){
                    cv::Point3d u = *dir[i];
                    cv::Point3d v = *dir[j];
                    cv::Point3d f = *st[i];
                    cv::Point3d k = *st[j];
                    cv::Point3d w = f-k;
                    cout<<"i = "<<i<<" j = "<<j;
                    cout<<"\tU"<<u<<" V"<<v<<endl;
                    cout<<"\tSU"<<f<<" SV"<<k<<" W"<<w<<endl;
                    double a = u.x*u.x + u.y*u.y + u.z*u.z;
                    double b = u.x*v.x + u.y*v.y + u.z*v.z;
                    double c = v.x*v.x + v.y*v.y + v.z*v.z;
                    double d = u.x*w.x + u.y*w.y + u.z*w.z;
                    double e = v.x*w.x + v.y*w.y + v.z*w.z;
                    double c1 = (b*e-c*d)/(a*c-b*b);
                    double c2 = (a*e-b*d)/(a*c-b*b);
                    cv::Point3d pc = f + u*c1;
                    cv::Point3d qc = k + v*c2;
                    cv::Point3d finalPos = pc*(0.5)+qc*(0.5);
                    cout<<"POS:"<<finalPos<<endl;
                    setPosition(finalPos);
                    this->status = STATUS_TRIGULATED;
                    total++;
                    avg = avg+finalPos;
                }
            }
        }
        avg = avg*(1.0/(double)total);
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