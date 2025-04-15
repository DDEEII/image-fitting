//#include <iostream>
#include "fitting.h"

string python_script = R"(
import turtle

def draw_circle(x, y, radius, color):
    turtle.penup()
    turtle.goto(x, y - radius)  # 移动到圆心下方的起始点
    turtle.setheading(0)         # 设置海龟方向朝东
    turtle.pendown()
    turtle.pencolor(color)      # 设置画笔颜色
    turtle.fillcolor(color)
    turtle.begin_fill()
    turtle.circle(radius)       # 绘制指定半径的圆
    turtle.end_fill()
    # 示例调用
    # draw_circle(0, 0, 100, "red")

def draw_polygon(points, color):
    
    turtle.penup()
    turtle.goto(points[0])       # 移动到第一个顶点
    turtle.pendown()
    
    turtle.pencolor(color)      # 设置画笔颜色
    turtle.fillcolor(color)      # 设置填充颜色
    turtle.begin_fill()          # 开始填充
    
    for point in points[1:]:     # 依次连接后续顶点
        turtle.goto(point)
    
    turtle.goto(points[0])       # 闭合路径（显式回到起点）
    turtle.end_fill()            # 结束填充
    # 示例调用
    # draw_polygon([(0,0), (100,0), (50,100)], "blue")



turtle.speed(0)
turtle.colormode(255)

)";

std::string scalarToRgb(const Scalar& scalar) {
    // 将各通道值四舍五入并限制在0-255之间
    uchar blue = saturate_cast<uchar>(cvRound(scalar[0]));
    uchar green = saturate_cast<uchar>(cvRound(scalar[1]));
    uchar red = saturate_cast<uchar>(cvRound(scalar[2]));

    // 格式化为RRGGBB（大写字母）
    return std::format("({},{},{})", red, green, blue);
}

std::string format_points(const std::vector<cv::Point>& points) {
    std::string formatted_points;
    formatted_points.reserve(points.size());

    for (const auto& pt : points) {
        formatted_points+=(std::format("({},{})", pt.x-300, 400-pt.y)+",");
    }
    formatted_points.pop_back();
    return std::format("[{}]", formatted_points);
}
int main() {//将图片转换为turtle图形
	vector<Region*> shapes = fiting("D:/input.png");
    for (auto& reg : shapes) {
        if (reg->is_circle) {
            python_script += std::format("draw_circle({}, {}, {}, {})\n", reg->circle_center.x-300, 400-reg->circle_center.y, reg->circle_radius ,scalarToRgb(reg->color));
            //circle(display, reg->circle_center, reg->circle_radius, reg->color, -1);
        }
        else {
            python_script += std::format("draw_polygon({}, {})\n",format_points(reg->contour) , scalarToRgb(reg->color));
            //fillPoly(display, { reg->contour }, reg->color);
        }
    }cout<<python_script;
	return 0;
}
