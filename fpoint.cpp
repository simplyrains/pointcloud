//
//  fpoint.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/20/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "fpoint.h"

//WARNING: DUPLICATE CODE WITH MAIN
imageholder* fpoint::getImageHolder(string name_){
    for(auto iter=(*all_pano).begin(); iter!=(*all_pano).end(); ++iter) {
        string newName = (*iter)->getName();
        if(newName == name_) return *iter;
    }
    return NULL;
}

fpoint::fpoint(int id, vector<imageholder*> *all_pano_){
    this->id = id;
    this->all_pano = all_pano_;
    this->status = STATUS_NO_POS;
}

void fpoint::addHP(imageholder* pano, double heading, double pitch){
    cv::Point2d hp(heading, pitch);
    addHP(pano->getName(), hp);
}

void fpoint::addHP(string pano, cv::Point2d hp){
    if(!(match.insert(make_pair(pano,hp))).second){
        match[pano] = hp;
    }
    triangulate();
}

bool fpoint::remove(string pano){
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
        imageholder *imh = fpoint::getImageHolder(iter->first);
        cout << imh->getID() << "/" <<iter->second << std::endl;
    }
    else cout<<"<EMPTY>"<<endl;
    cout<<endl;
}

void fpoint::clear(){
    match.clear();
}

double calcDistanceBetweenLines(cv::Point3d u, cv::Point3d v, cv::Point3d f, cv::Point3d k, cv::Point3d *center){
    cv::Point3d w = f-k;
    //cout<<"\tU"<<u<<" V"<<v<<endl;
    //cout<<"\tSU"<<f<<" SV"<<k<<" W"<<w<<endl;
    double a = u.x*u.x + u.y*u.y + u.z*u.z;
    double b = u.x*v.x + u.y*v.y + u.z*v.z;
    double c = v.x*v.x + v.y*v.y + v.z*v.z;
    double d = u.x*w.x + u.y*w.y + u.z*w.z;
    double e = v.x*w.x + v.y*w.y + v.z*w.z;
    double c1 = (b*e-c*d)/(a*c-b*b);
    double c2 = (a*e-b*d)/(a*c-b*b);
    cv::Point3d pc = f + u*c1;
    cv::Point3d qc = k + v*c2;
    *center = pc*(0.5)+qc*(0.5);
    double dx = (pc.x-qc.x);
    double dy = (pc.y-qc.y);
    double dz = (pc.z-qc.z);
    double distance = dx*dx+dy*dy+dz*dz;
    //cout<<"DISTANCE = "<<distance<<endl;
    return distance;
}


double fpoint::calcError(int id1, int id2){
    //TODO: FILL IN SOMETHING
    
    cv::Point3d u,v,f,k;
    bool has1 = false;
    bool has2 = false;
    imageholder *i1, *i2;
    i1=NULL;
    i2=NULL;
    double h1=0, h2=0, p1=0, p2=0;
    for(auto iter=match.begin(); iter!=match.end(); ++iter) {
        double x,y,z; //tempolary variable
        imageholder *imh = fpoint::getImageHolder(iter->first);
        int id = imh->getID();
        
        if(id == id1 || id == id2){
            x = imh->getRelativeX();
            y = imh->getRelativeY();
            z = 0;
            double heading = iter->second.x;
            double pitch = iter->second.y;
            //heading += imh->getRelativeH();
            //Starting Point
            cv::Point3d s(x,y,z);
            utility::HPtoLCS(heading, pitch, &x, &y, &z);
            //Direction vector
            cv::Point3d di(x,y,z);
            
            if(id == id1){
                f = s;
                u = di;
                has1 = true;
                i1 = imh;
                h1 = heading;
                p1 = pitch;
            }
            else if(id == id2){
                k = s;
                v = di;
                has2 = true;
                i2 = imh;
                h2 = heading;
                p2 = pitch;
            }
        }
    }
    
    if(has1 && has2){
        cv::Point3d center; //dummy variable
        //Currently triangulate from point 1 and 2
        //TODO: change to triangulate from all point
        calcDistanceBetweenLines(u, v, f, k, &center);
        double delta = abs(h1 - i1->computeHeading(center.x, center.y, center.z))
        + abs(p1 - i1->computePitch(center.x, center.y, center.z))
        + abs(h2 - i2->computeHeading(center.x, center.y, center.z))
        + abs(p2 - i2->computePitch(center.x, center.y, center.z));
        return delta;
    }
    return 0;
}


void fpoint::triangulate(){
    //TODO: FILL IN SOMETHING
    
    if(match.size()>=2){
        
        cout<<"{ TRIANGULATE POINT"<<id<<" }"<<endl;
        cv::Point3d* dir[match.size()];
        cv::Point3d* st[match.size()];
        int i = 0;
        for(auto iter=match.begin(); iter!=match.end(); ++iter) {
            imageholder *imh = fpoint::getImageHolder(iter->first);
            cout << imh->getID() << "/" <<iter->second << std::endl;
            
            double x,y,z; //tempolary variable
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
                    
                    cv::Point3d finalPos;
                    double err = calcDistanceBetweenLines(u, v, f, k, &finalPos);
                    cout<<"POS:"<<finalPos<<endl;
                    setPosition(finalPos);
                    total++;
                    cout<<"Error ("<<i<<","<<j<<") = "<<err<<endl;
                    avg = avg+finalPos;
                }
            }
        }
        avg = avg*(1.0/(double)total);
        this->status = STATUS_TRIGULATED;
    }
}

void fpoint::setStatus(uint status_){
    status = status_;
}


uint fpoint::getStatus(){
    return status;
}

void fpoint::setID(int id_){
    id = id_;
}


int fpoint::getID(){
    return id;
}

void fpoint::setPosition(cv::Point3d pos){
    position = pos;
    status = STATUS_HAVE_POS;
}

cv::Point3d fpoint::getPosition(){
    return position;
}

unsigned long fpoint::matchSize(){
    return match.size();
}

#pragma mark save

void fpoint::saveData(ofstream &output){

    output << "p ";
    output << id << " ";
    output << position.x << " " << position.y << " " << position.z << " ";
    output << status << " ";
    output << match.size() << endl;
    
    for(auto iter=match.begin(); iter!=match.end(); ++iter) {
        string name_ = iter->first;
        cv::Point2d pos_ = iter->second;
        output << "  " << name_ << " " << pos_.x << " " << pos_.y << endl;
    }
}


void fpoint::loadData(ifstream &input){
    //output << "p ";
    //output << id << " ";

    //output << position.x << " " << position.y << " " << position.z << " ";
    input >> position.x;
    input >> position.y;
    input >> position.z;
    //output << status << " ";
    input >> status;
    //output << match.size() << " ";
    int size;
    input >> size;
    
    //for(auto iter=match.begin(); iter!=match.end(); ++iter) {
    for(int i=0; i<size; i++){
        //string name_ = iter->first;
        string name_;
        double x, y;
        //output << name_ << " " << pos_.x << " " << pos_.y << endl;
        //cv::Point2d pos_ = iter->second;
        input >> name_;
        input >> x;
        input >> y;
        cv::Point2d p(x, y);
        addHP(name_, p);
    }
}

