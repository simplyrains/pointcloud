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
    void function2();
    void function3();
    
    template<typename T>
    void function4()
    {
        //function template definition should be here!
    }
}

#endif /* defined(__pointcloud__utility__) */
