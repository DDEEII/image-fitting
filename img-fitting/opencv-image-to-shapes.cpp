#include "EDLib.h"
#include <Eigen/Dense>
#include <iostream>
#include<opencv2/imgproc.hpp>
#include <algorithm>
#include"fitting.h"




const int MIN_area = 100; // 最小面积阈值
const int white = 240; // 白色阈值
const float COLOR_THRESHOLD = 15.0f; // Lab颜色差异阈值




// 圆形识别算法
typedef struct threshold {
    int T_l = 20;
    float T_ratio = 0.001;
    int T_o = 5;// 5 10 15 20 25
    int T_r = 5;// 5 10 15 20 25
    float T_inlier = 0.2;//0.2 0.3 0.35 0.4 0.45 0.5 (the larger the more strict)
    float T_angle = 2.0;// 
    float T_inlier_closed = 0.5;//0.5,0.6 0.7 0.8,0.9
    float sharp_angle = 60;//35 40 45 50 55 60 (turn this up to detect smaller circles)

}T;
vector<DetectedCircle> detectCircles(Mat inputImg, const Scalar& clusterColor) {
    T test_threshold; // 使用默认阈值
    cv::imshow("contour", inputImg); cv::waitKey(500);
    // 预处理图像：转为灰度图并高斯模糊
    Mat testImg;
    cvtColor(inputImg, testImg, COLOR_BGR2GRAY);
    GaussianBlur(testImg, testImg, Size(9, 9), 2, 2);

    // EDPF边缘检测
    EDPF testEDPF(testImg);
    vector<vector<Point>> EDPFsegments = testEDPF.getSegments();

    // 过滤短边缘（长度 >= 16）
    vector<vector<Point>> edgeList;
    for (const auto& seg : EDPFsegments) {
        if (seg.size() >= 16)
            edgeList.push_back(seg);
    }

    // 第一次提取闭合边缘
    closedEdgesExtract* closedAndNotClosedEdges = extractClosedEdges(edgeList);
    vector<vector<Point>> closedEdgeList = closedAndNotClosedEdges->closedEdges;

    // 使用RDP算法近似边缘为线段
    vector<vector<Point>> segList;
    for (const auto& edge : edgeList) {
        vector<Point> segTemp;
        RamerDouglasPeucker(edge, 2.5, segTemp);
        segList.push_back(segTemp);
    }

    // 拒绝尖锐转角
    sharpTurn* newSegEdgeList = rejectSharpTurn(edgeList, segList, test_threshold.sharp_angle);
    vector<vector<Point>> newEdgeList = newSegEdgeList->new_edgeList;
    vector<vector<Point>> newSegList = newSegEdgeList->new_segList;

    // 检测拐点
    InflexionPt* newSegEdgeListAfterInflexion = detectInflexPt(newEdgeList, newSegList);
    vector<vector<Point>> newEdgeListAfterInfexion = newSegEdgeListAfterInflexion->new_edgeList;

    // 删除近似直线或过短的边缘
    auto it = newEdgeListAfterInfexion.begin();
    while (it != newEdgeListAfterInfexion.end()) {
        Point edgeSt = it->front();
        Point edgeEd = it->back();
        int midIndex = it->size() / 2;
        Point edgeMid = (*it)[midIndex];

        double distStEd = norm(edgeSt - edgeEd);
        double distStMid = norm(edgeSt - edgeMid);
        double distMidEd = norm(edgeMid - edgeEd);
        double distDiff = abs((distStMid + distMidEd) - distStEd);

        if (it->size() <= test_threshold.T_l || distDiff <= test_threshold.T_ratio * (distStMid + distMidEd)) {
            it = newEdgeListAfterInfexion.erase(it);
        }
        else {
            ++it;
        }
    }

    // 第二次提取闭合边缘
    closedEdgesExtract* closedAndNotClosedEdges1 = extractClosedEdges(newEdgeListAfterInfexion);
    vector<vector<Point>> closedEdgeList1 = closedAndNotClosedEdges1->closedEdges;
    vector<vector<Point>> notclosedEdgeList1 = closedAndNotClosedEdges1->notClosedEdges;

    // 合并两次闭合边缘
    closedEdgeList.insert(closedEdgeList.end(), closedEdgeList1.begin(), closedEdgeList1.end());

    // 排序未闭合的边缘以分组
    vector<vector<Point>> sortedEdgeList = sortEdgeList(notclosedEdgeList1);

    // 分组圆弧并估计圆参数
    groupArcs* arcs = coCircleGroupArcs(sortedEdgeList, test_threshold.T_o, test_threshold.T_r);
    vector<vector<Point>> groupedArcs = arcs->arcsFromSameCircles;
    vector<vector<Point>> groupedArcsThreePt = arcs->arcsStartMidEnd;
    vector<Vec3f> groupedOR = arcs->recordOR;

    vector<Circles> groupedCircles = circleEstimateGroupedArcs(
        groupedArcs, groupedOR, groupedArcsThreePt, test_threshold.T_inlier, test_threshold.T_angle
    );

    // 处理闭合边缘的圆估计
    vector<Circles> closedCircles = circleEstimateClosedArcs(closedEdgeList, test_threshold.T_inlier_closed);

    // 合并分组和闭合的圆
    vector<Circles> totalCircles;
    totalCircles.insert(totalCircles.end(), groupedCircles.begin(), groupedCircles.end());
    totalCircles.insert(totalCircles.end(), closedCircles.begin(), closedCircles.end());

    // 聚类去重
    vector<Circles> preCircles = clusterCircles(totalCircles);
    // 转换结果格式
    vector<DetectedCircle> result;
    for (auto& c : preCircles) {
        DetectedCircle dc;
        dc.center = Point2f(c.xc, c.yc);
        dc.radius = c.r;
        dc.color = clusterColor;
        result.push_back(dc);
    }
    return result;
}
bool isPointInAnyCircle(Point2f pt, const vector<DetectedCircle>& circles) {
    for (const auto& c : circles) {
        if (norm(pt - c.center) <= c.radius+2) {
            return true;
        }
    }
    return false;
}
// 颜色差异计算
inline float deltaE(const Vec3b& a, const Vec3b& b) {
    return sqrt(pow(a[0] - b[0], 2) + pow(a[1] - b[1], 2) + pow(a[2] - b[2], 2));
}

// 洪水填充分割
Mat floodFillSegmentation(Mat& image) {
    Mat lab;
    cvtColor(image, lab, COLOR_BGR2Lab);
    Mat processed = image.clone();
    Mat visited(image.size(), CV_8UC1, Scalar(0));

    for (int y = 0; y < lab.rows; ++y) {
        for (int x = 0; x < lab.cols; ++x) {
            if (!visited.at<uchar>(y, x)) {
                // 洪水填充获取连通区域
                queue<Point> q;
                vector<Point> region;
                Vec3b seedColor = lab.at<Vec3b>(y, x);
                q.push(Point(x, y));
                visited.at<uchar>(y, x) = 1;

                while (!q.empty()) {
                    Point p = q.front();
                    q.pop();
                    region.push_back(p);

                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            int ny = p.y + dy;
                            int nx = p.x + dx;
                            if (0 <= ny && ny < lab.rows && 0 <= nx && nx < lab.cols) {
                                if (!visited.at<uchar>(ny, nx) &&
                                    deltaE(lab.at<Vec3b>(ny, nx), seedColor) < COLOR_THRESHOLD) {
                                    visited.at<uchar>(ny, nx) = 1;
                                    q.push(Point(nx, ny));
                                }
                            }
                        }
                    }
                }

                // 计算主要颜色
                vector<Vec3b> colors;
                for (auto& pt : region)
                    colors.push_back(image.at<Vec3b>(pt));

                // 统计直方图找主色
                int hist[16][16][16] = { 0 };
                for (auto& c : colors) {
                    int b = c[0] / 16, g = c[1] / 16, r = c[2] / 16;
                    hist[b][g][r]++;
                }

                Vec3b dominant(0, 0, 0);
                int max_count = 0;
                for (int b = 0; b < 16; ++b)
                    for (int g = 0; g < 16; ++g)
                        for (int r = 0; r < 16; ++r) {
                            if (hist[b][g][r] > max_count) {
                                max_count = hist[b][g][r];
                                dominant = Vec3b(b * 16 + 8, g * 16 + 8, r * 16 + 8);
                            }
                        }

                // 填充主色
                for (auto& pt : region)
                    processed.at<Vec3b>(pt) = dominant;
            }
        }
    }
    return processed;
}

//拟合
vector<Region*> fiting(String path) {
    // 1. 读取图像
    Mat image = imread(path);
    resize(image, image, Size(800, 600));

    Mat display(image.rows, image.cols, image.type(), Scalar(255, 255, 255));
    //resize(display, display, Size(2000, 2000));

    //imshow("Fitting Process", display);

    // 颜色聚类部分：
    Mat processed = floodFillSegmentation(image);

    // 提取唯一颜色
    vector<Scalar> colors;
    for (int y = 0; y < processed.rows; ++y) {
        for (int x = 0; x < processed.cols; ++x) {
            Vec3b c = processed.at<Vec3b>(y, x);
            if (c[0] < white || c[1] < white || c[2] < white) {
                Scalar color(c[0], c[1], c[2]);
                if (find(colors.begin(), colors.end(), color) == colors.end())
                    colors.push_back(color);
            }
        }
    }

    vector<Region*> allRegions;

    for (Scalar clusterColor : colors) {
        //if (clusterColor[0] >= white && clusterColor[1] >= white && clusterColor[2] >= white)continue;
        // 生成颜色掩膜
        Mat colorMask;
        inRange(processed, clusterColor, clusterColor, colorMask);
        
        
        // 第一阶段：处理圆形
        Mat regionImage;
        image.copyTo(regionImage, colorMask);
        vector<DetectedCircle> circles = detectCircles(regionImage, clusterColor);
        
        // 处理检测到的圆形
        for (auto& c : circles) {
            Region* reg = new Region;
            reg->is_circle = true;
            reg->circle_center = c.center;
            reg->circle_radius = c.radius;
            reg->color = c.color;
            allRegions.push_back(reg);
        }
        // 第二阶段：处理多边形
        vector<vector<Point>> contours;
        findContours(colorMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (auto& contour : contours) {
            if (contourArea(contour) < MIN_area) continue;

            // 多边形近似
            vector<Point> approx;
            double epsilon = 0.01 * arcLength(contour, true);
            approxPolyDP(contour, approx, epsilon, true);
            if (approx.size() < 3) continue;

            // 过滤位于圆形区域内的顶点
            vector<Point> filtered_points;
            const int n = approx.size();
            for (int i = 0; i < n; ++i) {
                const Point& pt = approx[i];
                const Point2f pt_f(pt);  // 转换为浮点坐标

                // 检查当前点是否在任意圆形内
                bool current_in_circle = isPointInAnyCircle(pt_f, circles);

                if (!current_in_circle) {
                    filtered_points.push_back(pt);
                    continue;
                }

                // 获取相邻顶点（考虑循环边界）
                const int prev_idx = (i - 1 + n) % n;
                const int next_idx = (i + 1) % n;
                const Point& prev_pt = approx[prev_idx];
                const Point& next_pt = approx[next_idx];

                // 检查相邻顶点是否都不在圆形内
                bool prev_outside = !isPointInAnyCircle(Point2f(prev_pt), circles);
                bool next_outside = !isPointInAnyCircle(Point2f(next_pt), circles);

                // 如果至少有一个相邻点在圆外，则保留当前点
                if (prev_outside || next_outside) {
                    filtered_points.push_back(pt);
                }
            }

            if (filtered_points.size() >= 3) {
                Region* reg = new Region;
                reg->contour = filtered_points;
                reg->color = clusterColor;
                allRegions.push_back(reg);
            }
        }
    }

    // 深度排序（按面积降序）
    std::sort(allRegions.begin(), allRegions.end(), [](Region* a, Region* b) {
        double areaA = a->is_circle ? CV_PI * pow(a->circle_radius, 2) : contourArea(a->contour);
        double areaB = b->is_circle ? CV_PI * pow(b->circle_radius, 2) : contourArea(b->contour);
        return areaA > areaB;
        });
    
    // 绘制结果
    for (auto& reg : allRegions) {
        if (reg->is_circle) {
            circle(display, reg->circle_center, reg->circle_radius, reg->color, -1);
        }
        else {
            fillPoly(display, { reg->contour }, reg->color);
        }//imshow("Fitting Process", display); cv::waitKey(0);
    }
    
    imshow("result", display);

    return allRegions;
}