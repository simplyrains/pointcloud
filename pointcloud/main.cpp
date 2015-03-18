#include <string>
#include <iostream>
#include "imageholder.h"
using namespace std;


int main()
{
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
    return 0;
    
}