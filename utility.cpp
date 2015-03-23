//
//  utility.cpp
//  pointcloud
//
//  Created by Sarin Achawaranont on 3/23/2558 BE.
//  Copyright (c) 2558 Sarin Achawaranont. All rights reserved.
//

#include "utility.h"
#include "math.h"

namespace utility
{
    double degToRad(double degree){
        return degree*M_PI/180;
    }
    
    //Calculate distance using ’haversine’ formula
    //Info: http://www.movable-type.co.uk/scripts/latlong.html
    double distance(double lat1, double lng1, double lat2, double lng2){
        //code
        double R = 6371; // Radius of the earth in km
        double dLat = degToRad(lat2-lat1);  // deg2rad below
        double dLon = degToRad(lng2-lng1);
        double a =
            sin(dLat/2) * sin(dLat/2) +
            cos(degToRad(lat1)) * cos(degToRad(lat2)) *
            sin(dLon/2) * sin(dLon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double d = R * c * 1000; // Distance in m
        return d;
    }
    void function2()
    {
        //code
    }
    void function3()
    {
        //code
    }
}