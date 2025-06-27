#pragma once

#include <vector>
#include<opencv2/opencv.hpp>

using namespace cv;
using namespace std;


struct Region {
    vector<Point> contour;
    //Mat mask;
    Scalar color;
    Point2f circle_center;
    float circle_radius;
    bool is_circle = false;
};

vector<Region*> fiting(String path);