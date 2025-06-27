//#include <iostream>
#include "fitting.h"
#include <cmath>
#include <unordered_map>
#include <numeric>
#include <tuple>

string python_script = R"(









#======================================
#python script
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





/*================================================*/
//优化坐标
/*
1. **条件一处理**：
   - **x坐标分组**：将点按x坐标排序后，进行分组，确保每组内x坐标的最大差值不超过阈值。每组点的x坐标替换为该组的均值。
   - **y坐标分组**：类似x坐标处理，对y坐标进行分组和均值替换。

2. **条件二处理**：
   - 预处理所有可能的吸附位置：遍历所有点对，根据给定的比例数组m生成所有可能的吸附坐标，并存储在`all_c_x`和`all_c_y`中。
   - 使用二分查找快速找到每个点附近的候选坐标，减少时间复杂度。

3. **候选坐标选择**：
   - 收集所有可能的候选坐标（原始坐标、条件一候选、条件二候选）。
   - 计算每个候选坐标与原坐标的距离，选择最近的作为最终坐标。

*/
// 
// 
vector<Point2f> autoDocking(const vector<Point2f>& points, double threshold, const vector<double>& m) {
    int n = points.size();
    if (n == 0) return points;

    vector<Point2f> original = points;
    vector<Point2f> result(n);

    // 条件一处理：x坐标分组
    vector<double> x_means(n, 0.0);
    vector<bool> in_x_group(n, false);
    {
        vector<size_t> indices(n);
        iota(indices.begin(), indices.end(), 0);
        sort(indices.begin(), indices.end(), [&original](size_t i, size_t j) {
            return original[i].x < original[j].x;
            });

        vector<double> sorted_x;
        for (auto i : indices) sorted_x.push_back(original[i].x);

        size_t start = 0;
        while (start < n) {
            size_t end = start + 1;
            double current_min = sorted_x[start];
            double current_max = sorted_x[start];

            // 找到最大差值不超过threshold的连续区间
            while (end < n) {
                double potential_min = min(current_min, sorted_x[end]);
                double potential_max = max(current_max, sorted_x[end]);
                if (potential_max - potential_min <= threshold) {
                    current_min = potential_min;
                    current_max = potential_max;
                    end++;
                }
                else {
                    break;
                }
            }

            cout << "x group [" << start << "," << end - 1 << "]: ";
            for (size_t i = start; i < end; ++i)
                cout << sorted_x[i] << " ";
            cout << endl;

            
            if (end - start >= 2) {
                double sum = accumulate(sorted_x.begin() + start, sorted_x.begin() + end, 0.0);
                double mean = sum / (end - start);
                for (size_t i = start; i < end; ++i) {
                    x_means[indices[i]] = mean;
                    in_x_group[indices[i]] = true;
                }
            }
            start = end;
        }
    }

    // 条件一处理：y坐标分组（逻辑同x坐标）
    vector<double> y_means(n, 0.0);
    vector<bool> in_y_group(n, false);
    {
        vector<size_t> indices(n);
        iota(indices.begin(), indices.end(), 0);
        sort(indices.begin(), indices.end(), [&original](size_t i, size_t j) {
            return original[i].y < original[j].y;
            });

        vector<double> sorted_y;
        for (auto i : indices) sorted_y.push_back(original[i].y);

        size_t start = 0;
        while (start < n) {
            size_t end = start + 1;
            double current_min = sorted_y[start];
            double current_max = sorted_y[start];

            while (end < n) {
                double potential_min = min(current_min, sorted_y[end]);
                double potential_max = max(current_max, sorted_y[end]);
                if (potential_max - potential_min <= threshold) {
                    current_min = potential_min;
                    current_max = potential_max;
                    end++;
                }
                else {
                    break;
                }
            }

            cout << "y group [" << start << "," << end - 1 << "]: ";
            for (size_t i = start; i < end; ++i)
                cout << sorted_y[i] << " ";
            cout << endl;

            if (end - start >= 2) {
                double sum = accumulate(sorted_y.begin() + start, sorted_y.begin() + end, 0.0);
                double mean = sum / (end - start);
                for (size_t i = start; i < end; ++i) {
                    y_means[indices[i]] = mean;
                    in_y_group[indices[i]] = true;
                }
            }
            start = end;
        }
    }

    // 条件二处理：生成所有候选吸附点（仅当a,b差值<=threshold时生成）
    vector<double> all_c_x, all_c_y;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;

            // 处理x坐标
            double a_x = original[i].x, b_x = original[j].x;
            if (a_x < b_x && (b_x - a_x) <= threshold) {
                for (double mi : m) {
                    all_c_x.push_back(a_x + (b_x - a_x) * mi);
                }
            }

            // 处理y坐标
            double a_y = original[i].y, b_y = original[j].y;
            if (a_y < b_y && (b_y - a_y) <= threshold) {
                for (double mi : m) {
                    all_c_y.push_back(a_y + (b_y - a_y) * mi);
                }
            }
        }
    }

    // 去重并排序
    sort(all_c_x.begin(), all_c_x.end());
    all_c_x.erase(unique(all_c_x.begin(), all_c_x.end()), all_c_x.end());
    sort(all_c_y.begin(), all_c_y.end());
    all_c_y.erase(unique(all_c_y.begin(), all_c_y.end()), all_c_y.end());

    cout << "Generated all_c_x: ";
    for (double x : all_c_x) cout << x << " ";
    cout << "\nGenerated all_c_y: ";
    for (double y : all_c_y) cout << y << " ";
    cout << endl;


    // 为每个点选择最近的吸附位置
    for (int i = 0; i < n; ++i) {
        vector<Point2f> candidates;
        const Point2f& orig = original[i];
        candidates.push_back(orig);// 原始位置

        // 添加条件一的候选
        if (in_x_group[i]) candidates.emplace_back(x_means[i], orig.y);
        if (in_y_group[i]) candidates.emplace_back(orig.x, y_means[i]);
        if (in_x_group[i] && in_y_group[i]) candidates.emplace_back(x_means[i], y_means[i]);

        // 添加条件二的候选
        auto addCondition2Candidates = [&](const vector<double>& coords, bool is_x) {
            double v = is_x ? orig.x : orig.y;
            auto lower = lower_bound(coords.begin(), coords.end(), v - threshold);
            auto upper = upper_bound(coords.begin(), coords.end(), v + threshold);
            for (auto it = lower; it != upper; ++it) {
                if (is_x) candidates.emplace_back(*it, orig.y);
                else candidates.emplace_back(orig.x, *it);
            }
        };

        addCondition2Candidates(all_c_x, true);
        addCondition2Candidates(all_c_y, false);

        cout << "Point " << i << " original: " << orig << endl;
        cout << "Candidates: ";
        for (const auto& cand : candidates) cout << cand << " ";
        cout << endl;

        // 选择最近的点
        float min_x_dist = numeric_limits<float>::max(), min_y_dist = numeric_limits<float>::max();
        Point2f best = orig;
        for (const auto& cand : candidates) {
            if (cand.x == orig.x && cand.y == orig.y) continue;
            float dx = abs(cand.x - orig.x);
            float dy = abs(cand.y - orig.y);
            if (dx < threshold && dy < threshold) {
                if (dx < min_x_dist || dy < min_y_dist) {
                    min_x_dist = dx;
                    min_y_dist = dy;
                    best = cand;
                }
            }
            
        }
        result[i] = best;
    }

    return result;
}


/*================================*/
//增强坐标可读性
struct Score {
    int decimal_places;
    int trailing_zeros;
    bool operator>(const Score& other) const {
        if (decimal_places != other.decimal_places)
            return decimal_places < other.decimal_places;
        return trailing_zeros > other.trailing_zeros;
    }
};

Score compute_score(float value) {
    
    const int precision = 20;
    stringstream ss;
    ss << fixed;
    ss.precision(precision);
    ss << value;
    string s = ss.str();
    cout << "Computing score for " << s << endl;

    size_t dot_pos = s.find('.');
    string int_part, dec_part;
    if (dot_pos != string::npos) {
        int_part = s.substr(0, dot_pos);
        dec_part = s.substr(dot_pos + 1);
        while (!dec_part.empty() && dec_part.back() == '0')
            dec_part.pop_back();
        if (dec_part.empty())
            dec_part = "";
    }
    else {
        int_part = s;
        dec_part = "";
    }

    int decimal = dec_part.length();
    if (dot_pos != string::npos && dec_part.empty())
        decimal = 0;

    int trailing = 0;
    if (!int_part.empty()) {
        int i = int_part.size() - 1;
        while (i >= 0 && int_part[i] == '0') {
            trailing++;
            i--;
        }
    }

    cout<< "Score: decimal=" << decimal << ", trailing=" << trailing << endl;
    return { decimal, trailing };
}

vector<Point2f> apply_translation(const vector<Point2f>& points, float tx, float ty) {
    vector<Point2f> res;
    for (const auto& p : points)
        res.emplace_back(p.x - tx, p.y - ty);
    return res;
}

vector<Point2f> apply_scaling(const vector<Point2f>& points, float scale) {
    vector<Point2f> res;
    for (const auto& p : points)
        res.emplace_back(p.x * scale, p.y * scale);
    return res;
}

vector<Point2f> apply_adjustment(const vector<Point2f>& points, int index, bool adjust_x, float new_val) {
    vector<Point2f> res = points;
    if (adjust_x)
        res[index].x = new_val;
    else
        res[index].y = new_val;
    return res;
}

float compute_max_abs(const vector<Point2f>& points) {
    float max_abs = 0;
    for (const auto& p : points) {
        max_abs = max(max_abs, max(abs(p.x), abs(p.y)));
    }
    return max_abs;
}

vector<Point2f> optimize_points(const vector<Point2f>& original_points, float lower_threshold, float upper_threshold, float delta) {
    vector<Point2f> best_points = original_points;
    Score best_score;
    bool best_initialized = false;

    auto update_best = [&](const vector<Point2f>& points) {
        if (compute_max_abs(points) < lower_threshold || compute_max_abs(points) > upper_threshold)
            return;
        Score current = { 0, 0 };
        for (const auto& p : points) {
            Score sx = compute_score(p.x);
            Score sy = compute_score(p.y);
            current.decimal_places += sx.decimal_places + sy.decimal_places;
            current.trailing_zeros += sx.trailing_zeros + sy.trailing_zeros;
        }
        if (!best_initialized || (current > best_score)) {
            best_points = points;
            best_score = current;
            best_initialized = true;
        }
    };

    // 处理原始点集
    update_best(original_points);
    for (size_t i = 0; i < original_points.size(); ++i) {
        for (int coord = 0; coord < 2; ++coord) {
            float old_val = coord == 0 ? original_points[i].x : original_points[i].y;
            float min_val = max(old_val - delta, -upper_threshold);
            float max_val = min(old_val + delta, upper_threshold);

            vector<float> candidates;
            for (int steps : {1, 10, 100}) { // 优先尝试10的幂次
                float step = 1.0f / steps;
                float candidate = round(old_val * steps) / steps;
                if (candidate >= min_val && candidate <= max_val)
                    candidates.push_back(candidate);
            }
            candidates.push_back(floor(old_val));
            candidates.push_back(ceil(old_val));
            candidates.push_back(round(old_val));

            for (float candidate : candidates) {
                vector<Point2f> temp = original_points;
                if (coord == 0)
                    temp[i].x = candidate;
                else
                    temp[i].y = candidate;
                update_best(temp);
            }
        }
    }

    // 平移和缩放操作
    float min_x = numeric_limits<float>::max(), max_x = -numeric_limits<float>::max();
    float min_y = numeric_limits<float>::max(), max_y = -numeric_limits<float>::max();
    for (const auto& p : original_points) {
        min_x = min(min_x, p.x);
        max_x = max(max_x, p.x);
        min_y = min(min_y, p.y);
        max_y = max(max_y, p.y);
    }
    float tx = (min_x + max_x) / 2.0f;
    float ty = (min_y + max_y) / 2.0f;

    vector<Point2f> translated = apply_translation(original_points, tx, ty);
    float translated_max_abs = compute_max_abs(translated);

    // 生成候选缩放因子（优先10的幂次）
    vector<float> scale_candidates;
    if (translated_max_abs > 1e-6) {
        float scale_min = lower_threshold / translated_max_abs;
        float scale_max = upper_threshold / translated_max_abs;
        for (int exp = -3; exp <= 3; ++exp) { // 10^-3到10^3
            float scale = pow(10, exp);
            if (scale >= scale_min && scale <= scale_max)
                scale_candidates.push_back(scale);
        }
        for (float s : {0.5f, 2.0f, 5.0f, 20.0f, 50.0f}) { // 其他常见倍数
            if (s >= scale_min && s <= scale_max)
                scale_candidates.push_back(s);
        }
        scale_candidates.push_back(scale_max); // 确保包含上限
    }

    // 评估所有候选缩放因子
    for (float scale : scale_candidates) {
        cout << "Testing scale: " << scale << endl;
        vector<Point2f> scaled = apply_scaling(translated, scale);
        float current_max = compute_max_abs(scaled);
        if (current_max < lower_threshold || current_max > upper_threshold)
            continue;

        // 对缩放后的点进行微调
        vector<Point2f> current_best = scaled;
        for (size_t i = 0; i < scaled.size(); ++i) {
            for (int coord = 0; coord < 2; ++coord) {
                float old_val = coord == 0 ? scaled[i].x : scaled[i].y;
                float min_val = max(old_val - delta, -upper_threshold);
                float max_val = min(old_val + delta, upper_threshold);

                vector<float> candidates;
                for (int steps : {1, 10, 100}) { // 优先整数和半整数
                    float step = 1.0f / steps;
                    float candidate = round(old_val * steps) / steps;
                    if (candidate >= min_val && candidate <= max_val)
                        candidates.push_back(candidate);
                }
                candidates.push_back(floor(old_val));
                candidates.push_back(ceil(old_val));
                candidates.push_back(round(old_val));

                for (float candidate : candidates) {
                    vector<Point2f> temp = scaled;
                    if (coord == 0)
                        temp[i].x = candidate;
                    else
                        temp[i].y = candidate;
                    update_best(temp);
                }
            }
        }
    }

    return best_points;
}/*bugs unfixed */




int main() {//将图片转换为turtle图形
    
    //{ 10,31 }, { 14,29 }, { 4,29 }, { 6,31 }
    /*vector<Point2f> points = { {0,0},{1,50},{-1,30} };
    double threshold = 2.0;
    vector<double> m = { };
    auto result = autoDocking(points, threshold, m);
    for (auto i : result) { cout << i.x << "," << i.y << endl; }
    
    
    vector<Point2f> points = { {2.3, 5.3}, {4.5, 5.5} };
    double delta = 2.0;
    double upper_threshold = 600.0, lower_threshold =100.0;

    vector<Point2f> optimized = optimize_points(points, lower_threshold,upper_threshold , delta);
    for (auto i : optimized) { cout << i << endl; }
    */
    Mat display2(600, 800, CV_8UC3, Scalar(255, 255, 255));
    
	vector<Region*> shapes = fiting("D:/input.png");
    vector<Point2f> points = {};
    for (auto& reg : shapes) {
        if (reg->is_circle) {
            points.push_back(reg->circle_center);
        }
        else {
            points.insert(points.end(), reg->contour.begin(), reg->contour.end());
        }
    }
    double threshold = 2.0;
    vector<double> m = { 0.5 };
    auto result = autoDocking(points, threshold, m);
    int it=-1;
    for (auto& reg : shapes) {
        if (reg->is_circle) {
            reg->circle_center = result[++it];
        }
        else {
            for (int i = 0; i < reg->contour.size(); ++i) {
                reg->contour[i] = result[++it];
            }
        }
    }
    for (auto& reg : shapes) {
        if (reg->is_circle) {
            python_script += std::format("draw_circle({}, {}, {}, {})\n", reg->circle_center.x-300, 400-reg->circle_center.y, reg->circle_radius ,scalarToRgb(reg->color));
            circle(display2, reg->circle_center, reg->circle_radius, reg->color, -1);
        }
        else {
            python_script += std::format("draw_polygon({}, {})\n",format_points(reg->contour) , scalarToRgb(reg->color));
            fillPoly(display2, { reg->contour }, reg->color);
        }
    }cout<<python_script;
    //cv::imshow("display2", display2);
    //cv::waitKey(0);
	return 0;
}
