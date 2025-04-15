//#include <iostream>
#include "fitting.h"

string python_script = R"(
import turtle

def draw_circle(x, y, radius, color):
    turtle.penup()
    turtle.goto(x, y - radius)  # �ƶ���Բ���·�����ʼ��
    turtle.setheading(0)         # ���ú��귽�򳯶�
    turtle.pendown()
    turtle.pencolor(color)      # ���û�����ɫ
    turtle.fillcolor(color)
    turtle.begin_fill()
    turtle.circle(radius)       # ����ָ���뾶��Բ
    turtle.end_fill()
    # ʾ������
    # draw_circle(0, 0, 100, "red")

def draw_polygon(points, color):
    
    turtle.penup()
    turtle.goto(points[0])       # �ƶ�����һ������
    turtle.pendown()
    
    turtle.pencolor(color)      # ���û�����ɫ
    turtle.fillcolor(color)      # ���������ɫ
    turtle.begin_fill()          # ��ʼ���
    
    for point in points[1:]:     # �������Ӻ�������
        turtle.goto(point)
    
    turtle.goto(points[0])       # �պ�·������ʽ�ص���㣩
    turtle.end_fill()            # �������
    # ʾ������
    # draw_polygon([(0,0), (100,0), (50,100)], "blue")



turtle.speed(0)
turtle.colormode(255)

)";

std::string scalarToRgb(const Scalar& scalar) {
    // ����ͨ��ֵ�������벢������0-255֮��
    uchar blue = saturate_cast<uchar>(cvRound(scalar[0]));
    uchar green = saturate_cast<uchar>(cvRound(scalar[1]));
    uchar red = saturate_cast<uchar>(cvRound(scalar[2]));

    // ��ʽ��ΪRRGGBB����д��ĸ��
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
int main() {//��ͼƬת��Ϊturtleͼ��
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
