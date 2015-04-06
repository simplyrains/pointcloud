//
//  image_holder.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/18/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "imageholder.h"
#include "utility.h"
using namespace std;


// Will create class ImageHolder {

#define WIDTH 640
#define HEIGHT 640


#pragma mark Helper Function

    // Get color from the position in image. Right now I floor the value down. May change method later
    cv::Vec3b getColorHP(cv::Mat m, double heading, double pitch, double fov, double width, double height){
        utility::normalizeHP(&heading, &pitch);
        double py = utility::angleToPx(pitch,fov,height);
        double px = utility::angleToPx(heading,fov,width);
        if(py<height&&py>=0&&px<width&&px>0) return m.at<cv::Vec3b>((int)py,(int)px);
        else return cv::Vec3b(0,0,0);
    }

    void imageholder::prerender(double hfov, double pfov, double multiplication){
        int hf, pf;
        hf = multiplication*hfov;
        pf = multiplication*pfov;
        cv::Mat plane(pf+1,hf+1,CV_8UC3);
        //render image
        for(double row = 0; row <= pf; ++row) {
            for(double col = 0; col <= hf; ++col) {
                double heading = (1.0/multiplication)*(col-hf/2);
                double pitch = -(1.0/multiplication)*(row-pf/2);
                //cout<<row-90<<" "<<col<<endl;
                plane.at<cv::Vec3b>(row,col) = imageholder::getImageColorHP(heading,pitch);
            }
            
        }
        
        
        // Create smart pointer for SIFT feature detector.
        cv::Ptr<cv::FeatureDetector> featureDetector = new cv::SiftFeatureDetector(
                                                                                   0, // nFeatures
                                                                                   4, // nOctaveLayers
                                                                                   0.04, // contrastThreshold
                                                                                   100, //edgeThreshold
                                                                                   1.6 //sigma
                                                                                   );
        //Similarly, we create a smart pointer to the SIFT extractor.
        cv::Ptr<cv::DescriptorExtractor> featureExtractor = cv::DescriptorExtractor::create("SIFT");
        
        // Detect the keypoints
        featureDetector->detect(plane, keypoints); // NOTE: featureDetector is a pointer hence the '->'.
        
        // Compute the 128 dimension SIFT descriptor at each keypoint.
        // Each row in "descriptors" correspond to the SIFT descriptor for each keypoint
        featureExtractor->compute(plane, keypoints, descriptors);
        
        cout<<"COUNTER: "<<keypoints.size()<<endl;
        rendered = plane;
        
        //Calculate keypointsHP
        for(auto iter=keypoints.begin(); iter!=keypoints.end(); iter++){
            double x = (*iter).pt.x;
            double y = (*iter).pt.y;
            double heading = (1.0/multiplication)*(x-hf/2);
            double pitch = -(1.0/multiplication)*(y-pf/2);
            keypointsHP.push_back(*new cv::Point2d(heading, pitch));
        }
    }
#pragma mark main code

    imageholder::imageholder(double fov_in, string path, int id_, double hfov, double pfov, double multiplication){
        fraction = 1;
        fov = fov_in;
        int size = (int)ceil(360*2/fov);
        int size2 = (int)ceil(180*2/fov) + 1;
        holder = new cv::Mat*[size];
        
        for(int i=0;i<size;i++){
            cout<<(i*100/size)<<"%, ";
            holder[i] = new cv::Mat[size2];
            for(int j=0; j<size2; j++){
                int heading = (int)utility::indexToHeading(i,fov);
                int pitch = (int)utility::indexToPitch(j,fov);
                string name = path+to_string(pitch)+string(",")+to_string(heading)+string(".jpg");
                //cout<<"Reading: "<<name<<endl;
                cv::Mat input = cv::imread(name);
                
                holder[i][size2-j-1] = input.clone();
                //if fail to read the imagex
                if ( input.empty() ){
                    cout << "Error loading the image: " << name << " from "<<path<<endl;
                    holder[i][size2-j-1] =cv::Mat(640, 640, CV_8UC3);
                }
            }
        }
        relative_heading = 0;
        relative_x = 0;
        relative_y = 0;
        id = id_;
        prerender(hfov, pfov, multiplication);
        cout << " : Reading Complete!"<<endl;
    }

    cv::Mat imageholder::getImage(double heading, double pitch){
        
        utility::normalizeHP(&heading, &pitch);
        int i_h = utility::headingToClosestIndex(heading, fov);
        int i_p = utility::pitchToClosestIndex(pitch, fov);
        cv::Mat img = holder[i_h][i_p];
        return img;
    }

    cv::Vec3b imageholder::getImageColorHP(double heading, double pitch){
        // change from -90,90 to 90,-90
        pitch = -pitch;
        utility::normalizeHP(&heading, &pitch);
        int i_h = utility::headingToClosestIndex(heading, fov);
        int i_p = utility::pitchToClosestIndex(pitch, fov);
        double x,y,z;
        
        // Transform heading, pitch to base heading, base pitch
        utility::HPtoLCS(heading, pitch, &x, &y, &z);
        utility::rotateLCS(&x,&y,&z,utility::indexToHeading(i_h, fov),- utility::indexToHeading(i_h, fov),- utility::indexToPitch(i_p, fov));
        heading = utility::getHeading(x,y,z);
        pitch = utility::getPitch(x,y,z);
        
        cv::Mat img = holder[i_h][i_p];
        cv::Vec3b color = getColorHP(img, heading, pitch, fov, WIDTH, HEIGHT);
        
        return color;
    }

    void imageholder::setRelativePos(imageholder *base_img){
        double b_lat = base_img->getLat();
        double b_lng = base_img->getLng();
        relative_x = utility::distance(b_lat, b_lng, lat, b_lng);
        relative_y = utility::distance(b_lat, b_lng, b_lat, lng);
    }

    //computeHeading,computePitch, and is_projectable needs to take relative_x,relative_y, and relative_heading into account
    // ^ used to triangulate

    // Calculate the heading/pitch of the following coordinate based on the position of this pano
    double imageholder::computeHeading(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z;
        double heading = relative_heading + utility::getHeading(rx, ry, rz);
        return heading;
    }
    double imageholder::computePitch(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z;
        double pitch = relative_heading + utility::getPitch(rx, ry, rz);
        return pitch;
    }

    // return false if the coordinate is too close to the pano center
    bool imageholder::is_projectable(double x, double y, double z){
        double rx = x - relative_x;
        double ry = y - relative_y;
        double rz = z - relative_y;
        if(rx*rx+ry*ry+rz*rz < 1.0) return 0;
        return 1;
    }

#pragma mark save

    void imageholder::saveData(ofstream &output){
        output << "i " << name << " " << relative_x << " " << relative_y << " " << relative_heading << endl;
    }

    void imageholder::loadData(ifstream &input){
        //output << "i " << name << " " << relative_x << " " << relative_y << " " << relative_heading << endl;
        double x, y, h;
        input >> x;
        input >> y;
        input >> h;
        setRelativePos(x, y);
        //setRelativeHeading(h);
    }


#pragma mark getter/setter

    void imageholder::setName(string name_){
        name = name_;
    }

    void imageholder::setPos(double lat_, double lng_){
        lat = lat_;
        lng = lng_;
    }
    double imageholder::getLat(){
        return lat;
    }

    double imageholder::getLng(){
        return lng;
    }

    void imageholder::setRelativePos(double x, double y){
        relative_x = x;
        relative_y = y;
    }

    double imageholder::getRelativeX(){
        return relative_x;
    }

    double imageholder::getRelativeY(){
        return relative_y;
    }

    string imageholder::getName(){
        return name;
    }

    int imageholder::getID(){
        return id;
    }

    void imageholder::setID(int id_){
        id = id_;
    }

    cv::Mat imageholder::getRendered(){
        return rendered;
    }

    vector<cv::KeyPoint> imageholder::getKeypoints(){
        return keypoints;
    }

    vector<cv::Point2d> imageholder::getKeyPointLocation(){
        return keypointsHP;
    }
//    void imageholder::setRelativeH(double heading_){
//        relative_heading = heading_;
//    }
//
//    double imageholder::getRelativeH(){
//        return relative_heading;
//    }