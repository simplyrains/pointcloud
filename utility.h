//
//  utility.h
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/23/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#ifndef __pointcloud__utility__
#define __pointcloud__utility__

#include <stdio.h>


namespace utility
{
    double distance(double lat1, double lng1, double lat2, double lng2);
    
    void normalizeHP(double *heading, double *pitch);
    double getHeading(double x, double y, double z);
    double getPitch(double x, double y, double z);
    void rotateZ(double *x, double *y, double *z, double angle);
    void rotateY(double *x, double *y, double *z, double angle);
    // Use to get index for the image with closest heading/pitch
    int pitchToClosestIndex(double angle, double fov);
    double indexToPitch(int index, double fov);
    int headingToClosestIndex(double angle, double fov);
    double indexToHeading(int index, double fov);
    void rotateLCS(double *x, double *y, double *z, double heading_base, double heading, double pitch);
    void HPtoLCS(double heading, double pitch, double *x, double *y, double *z);
    double angleToPx(double angle, double fov, int imagewh);
    double pxToAngle(double px, double fov, int imagewh);
    
    void function2();
    void function3();
    
    template<typename T>
    void function4()
    {
        //function template definition should be here!
    }
}

#endif /* defined(__pointcloud__utility__) */
