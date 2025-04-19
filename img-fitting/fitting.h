#pragma once

#include <vector>
#include<opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct DetectedCircle {
    Point2f center;
    float radius;
    Scalar color;
};

struct Region {
    vector<Point> contour;
    //Mat mask;
    Scalar color;
    Point2f circle_center;
    float circle_radius;
    bool is_circle = false;
};

struct floodedRegion {
    vector<Point> region;
    Scalar color;
};

inline bool isPointInAnyCircle(Point2f pt, const vector<DetectedCircle>& circles);
inline float deltaE(const Vec3b& a, const Vec3b& b);
inline vector<floodedRegion> floodFillSegmentation(Mat& image);
inline vector<DetectedCircle> detectCircles(Mat inputImg, const Scalar& clusterColor);
vector<Region*> fiting(String path);