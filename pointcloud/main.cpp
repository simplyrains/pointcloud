#include <string>
#include <iostream>
#include "fpoint.h"
#include "imageholder.h"
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace fs = boost::filesystem;
using namespace std;

#define KEY_U 63232
#define KEY_D 63233
#define KEY_L 63234
#define KEY_R 63235
#define KEY_ESC 27

// GLABAL VARIABLE
// TODO:: Change to singleton
vector<imageholder*> all_pano;
vector<fpoint*> all_fpoint;
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

static void onMouseMiniWindow( int event, int x, int y, int f, void* param){
    params* mp = (params*)param;
    int wh = mp->wh;
    double fov = mp->fov;
    double heading = ((double)x/wh)*fov - fov/2 + mp->base_heading;
    double pitch = ((double)y/wh)*fov - fov/2 + mp->base_pitch;
    //cout << x << " " << y << "\t>\t"<< heading <<","<<pitch<< endl;
    if (event == cv::EVENT_LBUTTONDOWN) {
        cout<<"\nCLICK\n- Base Heading: "<<sw_base_heading<<endl<<"- Base Pitch: "<<sw_base_pitch<<endl;
        cout<<"- Heading: "<< heading<<endl<<"- Pitch: "<<pitch<<endl;
        mp->feature->addHP(mp->holder, heading, pitch);
        mp->feature->listMatch();
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        mp->feature->remove(mp->holder);
        mp->feature->listMatch();
    }
    //cv::circle( *smallpic, cv::Point( x, y ), 32.0, cv::Scalar( 0, 0, 255 ), 1, 8 );
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
            double heading = center_heading-fov/2+(col/wh)*fov;
            double pitch = center_pitch-fov/2+(row/wh)*fov;
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
    double multiplication = 4;
    hf = multiplication*hfov;
    pf = multiplication*pfov;
    cv::Mat plane(pf+1,hf+1,CV_8UC3);
    double heading = (1.0/multiplication)*(x-hf/2);
    double pitch = (1.0/multiplication)*(y-pf/2);
    //
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
    //TODO: Refractor 360, 180
    int hf, pf;
    hf = multiplication*hfov;
    pf = multiplication*pfov;
    cv::Mat plane(pf+1,hf+1,CV_8UC3);
    for(double row = 0; row <= pf; ++row) {
        for(double col = 0; col <= hf; ++col) {
            double heading = (1.0/multiplication)*(col-hf/2);
            double pitch = (1.0/multiplication)*(row-pf/2);
            //cout<<row-90<<" "<<col<<endl;
            plane.at<cv::Vec3b>(row,col) = imh->getImageColorHP(heading,pitch);
        }
        
    }
    return plane;
}

void normalrun(){
    fs::path p{"./google_output"};
    fs::directory_iterator it{p};
    
    
    //Init all pano
    while (it != fs::directory_iterator{}){
        if(fs::is_directory(it->path())){
            string path = it->path().string();
            path = path+string("/");
            
            cout<< path;
            
            imageholder *imh = new imageholder(30, path);
            //TODO: Add error handle in case imageholder can't load image
            
            string name = it->path().filename().string();
            imh->setName(name);
            
            std::vector<std::string> x;
            boost::split(x, name, boost::is_any_of("\t "));
            imh->setPos(atof(x.at(0).c_str()), atof(x.at(x.size()-1).c_str()));
            
            cout << "Name = " << imh->getName() << endl << endl;
            all_pano.push_back(imh);
        }
        it++;
    }
    cout<< "Init complete!!"<<endl<<"- - - - - - - - - - - - - -"<<endl;
    
    //View pano
    int panoindex = 0;
    //TODO::FPOINT
    all_fpoint.push_back(new fpoint(0));
    auto f_iter = all_fpoint.begin();
    bool do_loop=true;
    while(do_loop){
        imageholder* imh = all_pano[panoindex];
        cv::Mat plane = render(imh, 360, 180 ,4.0);
        
        params mp;
        mp.holder = imh;
        mp.feature = *f_iter;
        
        cv::imshow(imh->getName(),plane);
        cv::setMouseCallback(imh->getName(),onMouse, (void*)&mp );
        // Wait until user press some key
        int key = cv::waitKey(0);
        cv::destroyAllWindows();
        
        switch (key) {
            case KEY_L:{
                panoindex=(panoindex-1+(int)all_pano.size())%all_pano.size();
                break;
            }case KEY_R:{
                panoindex=(panoindex+1)%all_pano.size();
                break;
            }case KEY_U:{
                if(f_iter!=all_fpoint.begin()){
                    f_iter--;
                    cout<<"Change to point: "<< (*f_iter)->id << "." << endl;
                }
                else cout<<"This is the first feature point."<<endl;
                break;
            }case KEY_D:{
                auto next = f_iter+1;
                if(next!=all_fpoint.end()){
                    f_iter++;
                    cout<<"Change to point: "<< (*f_iter)->id << "." << endl;
                }
                else{
                    cout<<"Total f_point = " << (int)all_fpoint.size() << ", Add new point?"<<endl;
                    int newkey = cv::waitKey(0);
                    if(newkey == KEY_D){
                        int id = (int)all_fpoint.size();
                        all_fpoint.push_back(new fpoint(id));
                        f_iter = all_fpoint.end();
                        f_iter--;
                        cout<<"Point: "<< (*f_iter)->id << " was added." << endl;
                    }
                }
                break;
            }case KEY_ESC:{
                do_loop = false;
                break;
            }default:{
                break;
            }
        }
        cout<<"At point: "<<(*f_iter)->id<<", "<<(*f_iter)->match.size()<<" matche(s)."<<endl;
    }
}

int main(){
    normalrun();
    return 0;
}
/*
 imageholder imh = *new imageholder(30, string("google_output/"));
 
 //TODO: Add error handle in case imageholder can't load image
 
 int hf, pf, hfov, pfov;
 hfov = 360;
 pfov = 180;
 double multiplication = 4;
 hf = multiplication*hfov;
 pf = multiplication*pfov;
 
 int o=0;
 while (true) {
 o+=10;
 cv::Mat plane(pf+1,hf+1,CV_8UC3);
 for(double row = 0; row <= pf; ++row) {
 for(double col = 0; col <= hf; ++col) {
 double heading = (1.0/multiplication)*(col-hf/2);
 double pitch = (1.0/multiplication)*(row-pf/2);
 //cout<<row-90<<" "<<col<<endl;
 plane.at<cv::Vec3b>(row,col) = imh.getImageColorHP(heading+o,pitch);
 }
 
 }
 cv::imshow("x",plane);
 // Wait until user press some key
 cv::waitKey(0);
 }
 */


//}
//    fs::path directory("./google_output");
//    fs::directory_iterator iter(directory), end;
//
//    fs::path x;
//    for(;iter != end; ++iter)
//    {
//        if (iter->path().extension() == ".jpg")
//        {
//            x = iter->path();
//            break;
//        }
//    }
//    cv::Mat img = cv::imread(x.relative_path().string());
//    //if fail to read the image
//    if ( img.empty() )
//    {
//        cout << "Error loading the image" << endl;
//        return -1;
//    }
//    cv::imshow("My Window", img);
//
//    // Wait until user press some key
//    cv::waitKey(0);
//
//testColor(0, 90);
//return 0;


//    double x = 10;
//    double y = 100;
//    double z = 0;
//    cout << "\n\n\n\n";
//    cout << "(" << x << "," << y << "," << z << ") \n" << getHeading(x,y,z) << "," << getPitch(x,y,z) << "\t" << sqrt(x*x+y*y+z*z) << endl;
//    double ay = getHeading(x,y,z);
//    rotateZ(&x,&y,&z,-ay);
//    rotateY(&x,&y,&z,180);
//    cout << "(" << x << "," << y << "," << z << ") \n" << getHeading(x,y,z) << "," << getPitch(x,y,z) << "\t" << sqrt(x*x+y*y+z*z) << endl;
//    rotateZ(&x,&y,&z, ay);
//    cout << "(" << x << "," << y << "," << z << ") \n" << getHeading(x,y,z) << "," << getPitch(x,y,z) << "\t" << sqrt(x*x+y*y+z*z) << endl;

//for(double i=-30;i<30;i+=1.5)
//cout << i <<"\t"<<angleToPx(i,30,640)<<endl;
