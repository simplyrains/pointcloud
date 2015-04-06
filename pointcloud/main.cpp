#include <string>
#include <iostream>
#include <fstream>
#include "fpoint.h"
#include "imageholder.h"
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace fs = boost::filesystem;
using namespace std;

#define KEY_DIR_U 63232
#define KEY_DIR_D 63233
#define KEY_DIR_L 63234
#define KEY_DIR_R 63235
#define KEY_ESC 27
#define KEY_C 99
#define KEY_T 116
#define KEY_S 115
#define KEY_L 108
#define KEY_M 109


#define RENDER_RESOLUTION 4

// GLABAL VARIABLE
// TODO:: Change to singleton
vector<fpoint*> all_fpoint;
vector<fpoint*> all_matched_point;
vector<imageholder*> all_pano;

fpoint* currentpoint;
imageholder *currentpano;
double sw_base_heading;
double sw_base_pitch;

#pragma mark imshow callback

typedef struct PARAMS{
    imageholder* holder;
    double base_heading;
    double base_pitch;
    int wh;
    fpoint* feature;
    double fov;
}params;

imageholder* getImageHolder(string name_){
    for(auto iter=all_pano.begin(); iter!=all_pano.end(); ++iter) {
        string name = (*iter)->getName();
        if(name == name_) return *iter;
    }
    return NULL;
}
imageholder* getImageHolder(int id_){
    for(auto iter=all_pano.begin(); iter!=all_pano.end(); ++iter) {
        int id = (*iter)->getID();
        if(id == id_) return *iter;
    }
    return NULL;
}

fpoint* getFPoint(int id_){
    for(auto iter=all_fpoint.begin(); iter!=all_fpoint.end(); ++iter) {
        int id = (*iter)->getID();
        if(id == id_) return *iter;
    }
    return NULL;
}

static void onMouseMiniWindow( int event, int x, int y, int f, void* param){
    params* mp = (params*)param;
    int wh = mp->wh;
    double fov = mp->fov;
    double heading = mp->base_heading - fov/2 + ((double)x/wh)*fov;
    double pitch =   mp->base_pitch   + fov/2 - ((double)y/wh)*fov;
    //cout << x << " " << y << "\t>\t"<< heading <<","<<pitch<< endl;
    if (event == cv::EVENT_LBUTTONDOWN) {
        cout<<"\nCLICK\n- Base Heading: "<<sw_base_heading<<endl<<"- Base Pitch: "<<sw_base_pitch<<endl;
        cout<<"- Heading: "<< heading<<endl<<"- Pitch: "<<pitch<<endl;
        mp->feature->addHP(mp->holder, heading, pitch);
        mp->feature->listMatch();
        //cv::Point3d p(mp->holder->getRelativeX(),mp->holder->getRelativeY(),0);
        //cout<<"DEBUG: SET POSITION "<<p<<endl;
        //mp->feature->setPosition(p);
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        mp->feature->remove(mp->holder->getName());
        mp->feature->listMatch();
    }
    //cv::imshow("Holder", *smallpic);
}


cv::Mat renderSmall(imageholder* imh, double wh, double fov, double center_heading, double center_pitch){
    //TODO: Refractor 360, 180
    int hf, pf, hi, pi;
    hi = center_heading-fov/2;
    hf = hi+fov;
    pi = center_pitch-fov/2;
    pf = pi+fov;
    cv::Mat plane(wh,wh,CV_8UC3);
    for(double row = 0; row <= wh; ++row) {
        for(double col = 0; col <= wh; ++col) {
            double heading = center_heading - fov/2 + (col/wh)*fov;
            double pitch = center_pitch + fov/2 - (row/wh)*fov;
            //cout<<row-90<<" "<<col<<endl;
            plane.at<cv::Vec3b>(row,col) = imh->getImageColorHP(heading,pitch);
        }
        
    }
    return plane;
}

static void onMouse( int event, int x, int y, int f, void* param){
    params* mp = (params*)param;
    imageholder* pic = mp->holder;
    
    //TODO: Refractor 360, 180
    int hf, pf;
    int hfov = 360;
    int pfov = 180;
    double multiplication = RENDER_RESOLUTION;
    hf = multiplication*hfov;
    pf = multiplication*pfov;
    cv::Mat plane(pf+1,hf+1,CV_8UC3);
    double heading = (1.0/multiplication)*(x-hf/2);
    double pitch = -(1.0/multiplication)*(y-pf/2);
    //cout << x << " " << y << "\t Heading: "<< heading << "\tPitch: "<< pitch << endl;
    
    if (event == cv::EVENT_LBUTTONDOWN) {
        mp->wh = 400;
        mp->fov = 30;
        cv::Mat image= renderSmall(pic, mp->wh, mp->fov, heading, pitch);
        imshow("Holder", image);
        
        mp->base_heading = heading;
        mp->base_pitch = pitch;
        cv::setMouseCallback( "Holder", onMouseMiniWindow, (void*)mp );
        //putText(image, "point", Point(x,y), CV_FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,0,0));
    }
}

cv::Mat render(imageholder* imh, double hfov, double pfov, double multiplication){
    int hf, pf;
    hf = multiplication*hfov;
    pf = multiplication*pfov;

    cv::Mat plane;
    plane = (imh->getRendered()).clone();
    vector<cv::KeyPoint> keypoints = imh->getKeypoints();
    
    // If you would like to draw the detected keypoint just to check
    cv::Scalar keypointColor = cv::Scalar(255, 0, 0);     // Blue keypoints.
    //drawKeypoints(plane, keypoints, plane, keypointColor, cv::DrawMatchesFlags::DEFAULT);

    //add triangulated point
    for(auto point = all_fpoint.begin(); point!= all_fpoint.end(); point++){
        uint status = (*point)->getStatus();
        if(status==STATUS_TRIGULATED||status==STATUS_HAVE_POS){
            
            cv::Point3d lcs = (*point)->getPosition();
            cout<<"Found pos: "<<lcs<<" from point"<<(*point)->getID()<<endl;
            if(imh->is_projectable(lcs.x, lcs.y, lcs.z)){
                double p_heading = imh->computeHeading(lcs.x, lcs.y, lcs.z);
                double p_pitch = imh->computePitch(lcs.x, lcs.y, lcs.z);
                cout<<"H/P: "<< p_heading<<"/"<<p_pitch<<endl;
                
                double x =p_heading*multiplication+hf/2;
                double y =-p_pitch*multiplication+pf/2;
                cv::Point2d hp =cv::Point2d(x,y);
                
                string text = to_string((*point)->getID());
                int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
                double fontScale = 0.6;
                int thickness = 0;
                int baseline=0;
                baseline += thickness;
                cv::circle(plane, hp, 4, cv::Scalar( 0, 0, 0 ));
                cv::circle(plane, hp, 2, cv::Scalar( 0, 0, 255 ));
                cv::putText(plane, text, hp, fontFace, fontScale, cv::Scalar::all(255), thickness, 8);

            }
            else{
                cout<<"Point "<<(*point)->getID()<<" is ignored.";
            }
        }
    }
    
    
    //add triangulated keypoint
    for(auto point = all_matched_point.begin(); point!= all_matched_point.end(); point++){
        cv::Point3d lcs = (*point)->getPosition();
        if(imh->is_projectable(lcs.x, lcs.y, lcs.z)){
            double p_heading = imh->computeHeading(lcs.x, lcs.y, lcs.z);
            double p_pitch = imh->computePitch(lcs.x, lcs.y, lcs.z);
            
            double x =p_heading*multiplication+hf/2;
            double y =-p_pitch*multiplication+pf/2;
            cv::Point2d hp =cv::Point2d(x,y);
                
            cv::circle(plane, hp, 4, cv::Scalar( 0, 0, 0 ));
            cv::circle(plane, hp, 2, cv::Scalar( 0, 255, 0 ));
        }
    }

    return plane;
}

bool sortPano(imageholder *a, imageholder *b) {
    double x = atof((a->getName()).c_str());
    double y = atof((b->getName()).c_str());
    return x < y;
}

double calcPairError(int a, int b){
    double error = 0;
    
    for(auto point = all_fpoint.begin(); point!= all_fpoint.end(); point++){
        error+=(*point)->calcError(a, b);
    }
    return error;
}

//DUPLICATE CODE: FPOINT CALCERROR
double calcError(imageholder *i1, imageholder *i2, cv::Point2d point1, cv::Point2d point2){
    //TODO: FILL IN SOMETHING
    
    double x,y,z;
    
    x = i1->getRelativeX();
    y = i1->getRelativeY();
    z = 0;
    double h1 = point1.x;
    double p1 = point1.y;
    cv::Point3d f(x,y,z);
    utility::HPtoLCS(h1, p1, &x, &y, &z);
    //Direction vector
    cv::Point3d u(x,y,z);
    
    x = i2->getRelativeX();
    y = i2->getRelativeY();
    z = 0;
    double h2 = point2.x;
    double p2 = point2.y;
    cv::Point3d k(x,y,z);
    utility::HPtoLCS(h2, p2, &x, &y, &z);
    //Direction vector
    cv::Point3d v(x,y,z);

    cv::Point3d center; //dummy variable
    //Currently triangulate from point 1 and 2
    utility::calcDistanceBetweenLines(u, v, f, k, &center);
    double delta = abs(h1 - i1->computeHeading(center.x, center.y, center.z))
        + abs(p1 - i1->computePitch(center.x, center.y, center.z))
        + abs(h2 - i2->computeHeading(center.x, center.y, center.z))
        + abs(p2 - i2->computePitch(center.x, center.y, center.z));
    return delta;
}

void match(int a, int b, double threshold){
    
    imageholder *imh_a = getImageHolder(a);
    imageholder *imh_b = getImageHolder(b);
    vector<cv::Point2d> key_a = imh_a->getKeyPointLocation();
    vector<cv::Point2d> key_b = imh_b->getKeyPointLocation();
    all_matched_point.clear();
    
    int i = 0;
    for(auto k = key_a.begin(); k!= key_a.end(); k++,i++){
        double min_match = -1;
        cv::Point2d pair_b;
        for(auto k_b = key_b.begin(); k_b!=key_b.end(); k_b++){
            double error = calcError(imh_a, imh_b, *k, *k_b);
            if(error<threshold){
                if(min_match<0) min_match = error;
                else if(error<min_match){
                    min_match = error;
                    pair_b = *k_b;
                }
            }
        }
        if(min_match>0){
            fpoint *f = new fpoint(i, &all_pano);
            f->addHP(imh_a, k->x, k->y);
            f->addHP(imh_b, pair_b.x, pair_b.y);
            all_matched_point.push_back(f);
        }
    }
}

//Calculate the new error if we move the moved be r_x, r_y and r_heading from the original position
double alignImh(imageholder *base, imageholder *moved, double x_offset, double y_offset, double h_offset){
    //Backup old value
    double r_x = moved->getRelativeX();
    double r_y = moved->getRelativeY();
    //double r_h = moved->getRelativeH();
    //Set new value
    moved->setRelativePos(r_x+x_offset, r_y+y_offset);
    //moved->setRelativeH(r_h+h_offset);
    //Calculate error
    double err = calcPairError(base->getID(), moved->getID());
    //Put back the old value
    moved->setRelativePos(r_x, r_y);
    //moved->setRelativeH(r_h);
    return err;
}

double align(int a, int b){
    cout<<"Aligning "<<a<<" - "<<b<<endl;
    imageholder *imh_a, *imh_b;
    imh_a = NULL;
    imh_b = NULL;
    for(auto imh = all_pano.begin(); imh!= all_pano.end(); imh++){
        if((*imh)->getID()==a) imh_a = *imh;
        if((*imh)->getID()==b) imh_b = *imh;
    }
    if(imh_a==NULL || imh_b==NULL){
        cout<<"----------------- ERROR: NOPAIR"<<endl;
        return 0;
    }
    double min_x_offset = 0, min_y_offset = 0, min_h_offset = 0, min_error = 0;
    bool firstrun = true;
    double scale_x = 0.01;
    double scale_y = 0.01;
    //double scale_h = 1;
    
    double old_x = imh_b->getRelativeX();
    double old_y = imh_b->getRelativeY();
    //double old_heading = imh_b->getRelativeH();
    
    cout<<"Old Error: "<< alignImh(imh_a, imh_b, 0, 0, 0)<<endl;
    
    cout<<"Progress: ";
    int progress = 0;
    for(double x_offset = -5; x_offset<=5; x_offset+=scale_x){
        for(double y_offset = -5; y_offset<=5; y_offset+=scale_y){
            //for(double h_offset=-15; h_offset<=15; h_offset+= scale_h){
            double h_offset = 0;
                double err = alignImh(imh_a, imh_b, x_offset, y_offset, h_offset);
                if(firstrun){
                    firstrun = false;
                    min_error = err;
                    min_x_offset = x_offset;
                    min_y_offset = y_offset;
                    min_h_offset = h_offset;
                }
                else{
                    //err < min_error
                    if(min_error>err){
                        min_error = err;
                        min_x_offset = x_offset;
                        min_y_offset = y_offset;
                        min_h_offset = h_offset;
                    }
                }
            //}
        }
        progress++;
        if(progress%50==0) cout<<"x";
    }
    cout<<endl;
    
    cout<<"New offset: ("<<min_x_offset<<",\t"<<min_y_offset<<",\t"<<min_h_offset<<")\t\tERROR:"<<min_error<<endl;
    //Set the new value
    imh_b->setRelativePos(old_x + min_x_offset, old_y+min_y_offset);
    //imh_b->setRelativeH(old_heading+min_h_offset);
    return min_error;
}

void triangulateAll(){
    
    for(auto f = all_fpoint.begin(); f!= all_fpoint.end(); f++){
        (*f)->triangulate();
    }
}

void triangulateAllKeypoint(){
    
    for(auto f = all_matched_point.begin(); f!= all_matched_point.end(); f++){
        (*f)->triangulate();
    }
}

void initialization(string pathstring){
    
    fs::path p{pathstring};
    fs::directory_iterator it{p};
    
    int count = 0;
    //Init all pano
    while (it != fs::directory_iterator{}){
        if(fs::is_directory(it->path())){
            string path = it->path().string();
            path = path+string("/");
            
            cout<< path<< endl;
            
            imageholder *imh = new imageholder(30, path, count, 360, 180, RENDER_RESOLUTION);
            //TODO: Add error handle in case imageholder can't load image
            
            string fname = it->path().filename().string();
            
            std::vector<std::string> x;
            boost::split(x, fname, boost::is_any_of(","));
            imh->setPos(atof(x.at(0).c_str()), atof(x.at(1).c_str()));
            
            //Use image relative heading as name
            imh->setName(x.at(4));
            
            if(count!=0) {
                imh->setRelativePos(all_pano[0]);
                
                cout<<"Relative pos: ("<<imh->getRelativeX()<<","<<imh->getRelativeY()<<")"<<endl;
            }
            
            all_pano.push_back(imh);
            count++;
        }
        it++;
    }
    sort(all_pano.begin(), all_pano.end(), sortPano);

    cout<< endl<<"(づ￣ ³￣)づ <3 Init complete!!"<<endl<<"- - - - - - - - - - - - - -"<<endl;
}

void loadData(){
    string filename;
    cout<<"Load operation! Enter file name: ";
    cin >> filename;
    ifstream myfile (filename+".txt");

    string temp;
    if (myfile.is_open())
    {
        while ( myfile.good() )
        {
            myfile >> temp;
            cout << temp << " ";
            if(temp.substr(0,1)=="i"){
                string name;
                myfile >> name;
                cout << name << endl;
                imageholder *imh = getImageHolder(name);
                if(imh == NULL){
                    continue;
                    //TODO: FIX THIS, the last line appear 2 times
                    //cout<<"Error loading file name: " << name << ";_______;"<<endl;
                    //return;
                }
                imh->loadData(myfile);

            }
            else if(temp.substr(0,1)=="p"){
                int id;
                myfile >> id;
                cout << id << endl;
                //Create new Fpoint with this data
                fpoint *newPoint = new fpoint(id, &all_pano);
                newPoint->loadData(myfile);
                all_fpoint.push_back(newPoint);
            }
            else{
                cout<<"ERROR!?!? <"<<temp<<">"<<endl<<endl;
                return;
            }
        }
        myfile.close();
        cout << "Load complete!" << endl;
    }
    else cout << "Unable to load ;w;" << endl;
}

void saveData(){
    string filename;
    cout<<"Save operation! Enter file name: ";
    cin >> filename;
    ofstream myfile (filename+".txt");
    if (myfile.is_open())
    {
        for(auto f = all_fpoint.begin(); f!= all_fpoint.end(); f++){
            (*f)->saveData(myfile);
        }
        for(auto p = all_pano.begin(); p!= all_pano.end(); p++){
            (*p)->saveData(myfile);
        }
        myfile.close();
        cout << "Save complete!" << endl;
    }
    else cout << "Unable to save ;w;" << endl;
}

void normalrun(){
    //View pano
    int panoindex = 0;
    //TODO::FPOINT
    
    //Create 1 fpoint if there are no fpoint
    //= Will not create any if the fpoint is loaded from the save :P
    if(all_fpoint.size() == 0){
        all_fpoint.push_back(new fpoint(0, &all_pano));
    }
    auto f_iter = all_fpoint.begin();
    bool do_loop=true;
    while(do_loop){
        imageholder* imh = all_pano[panoindex];
        cv::Mat plane = render(imh, 360, 180 ,RENDER_RESOLUTION);
        
        params mp;
        mp.holder = imh;
        mp.feature = *f_iter;
        cout<<"\tPano"<<all_pano[panoindex]->getID()<<" id: "<<imh->getID()<<" pos:"<<imh->getRelativeX()<<","<<imh->getRelativeY()<<endl;
        cv::imshow(to_string(imh->getID()),plane);
        cv::setMouseCallback(to_string(imh->getID()),onMouse, (void*)&mp );
        // Wait until user press some key
        int key = cv::waitKey(0);
        
        
        switch (key) {
            case KEY_DIR_R:{
                panoindex=(panoindex-1+(int)all_pano.size())%all_pano.size();
                break;
            }case KEY_DIR_L:{
                panoindex=(panoindex+1)%all_pano.size();
                break;
            }case KEY_DIR_U:{
                if(f_iter!=all_fpoint.begin()){
                    f_iter--;
                    cout<<"Change to point: "<< (*f_iter)->getID()<< "." << endl;
                }
                else cout<<"This is the first feature point."<<endl;
                break;
            }case KEY_DIR_D:{
                auto next = f_iter+1;
                if(next!=all_fpoint.end()){
                    f_iter++;
                    cout<<"Change to point: "<< (*f_iter)->getID()<< "." << endl;
                }
                else{
                    cout<<"Total f_point = " << (int)all_fpoint.size() << ", Add new point?"<<endl;
                    int newkey = cv::waitKey(0);
                    if(newkey == KEY_DIR_D){
                        int id = (int)all_fpoint.size();
                        all_fpoint.push_back(new fpoint(id, &all_pano));
                        f_iter = all_fpoint.end();
                        f_iter--;
                        cout<<"Point: "<< (*f_iter)->getID()<< " was added." << endl;
                    }
                }
                break;
            }case KEY_ESC:{
                (*f_iter)->clear();
                cout<<"Clear all match from point"<<(*f_iter)->getID()<<endl;
                break;
            }case KEY_T:{
                //Triangulate
                //(*f_iter)->triangulate();
                //cout<<"Triangulate point"<<(*f_iter)->id<<endl;
                triangulateAll();
                break;
            }case KEY_S:{
                saveData();
                break;
            }case KEY_C:{
                cout<<"Calibrate Pair: ";
                int a, b;
                cin >> a;
                cout<<", ";
                cin >> b;
                double err = 0;
                for(int i=0;i<5;i++){
                    double new_err = align(a,b);
                    if(new_err<1) break;
                    if(abs(new_err-err)<0.1) break;
                    err=new_err;
                }
                triangulateAll();
                break;
            }case KEY_M:{
                cout<<"Match keypoint from Pair: ";
                int a, b;
                cin >> a;
                cout<<", ";
                cin >> b;
                match(a, b, 0.8);
                triangulateAllKeypoint();
                break;
            }default:{
                cout<<"Type:"<<key<<endl;
                break;
            }
        }
        
        cv::destroyAllWindows();
        cout<<"At point: "<<(*f_iter)->getID()<<", "<<(*f_iter)->matchSize()<<" matche(s)."<<endl;
    }
}

int main(){
    cout<<"Enter folder name: ";
    string pathstring;
    cin >> pathstring;
    pathstring = string("./")+pathstring;
    //Init imageholder data
    initialization(pathstring);
    //Load imageholder data/point data
    
    cout<<"Load existing data ;w;? (y/n): ";
    string ans;
    cin >> ans;
    if(ans=="y") loadData();
    
    normalrun();
    return 0;
}
