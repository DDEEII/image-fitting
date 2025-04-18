**bugs unfixed**

# image-fitting
fit an image using colored circles and polygons, and generate a python turtle script to draw them.(just to finish my homework lol)
 
circle detection using [zikai1/CircleDetection](https://github.com/zikai1/CircleDetection "zikai1/CircleDetection")

## Instructions
### 1. Requirements
VS 2022, OpenCV , and Eigen3.

### 2. Fitting
Call `fitting(string ImagePath)` to fit.
Or put image to D:/input.png and run main.cpp to generate the python turtle script.

## Suggestions
To customize your purpose, we provide some suggestions: 
#### the circle detection part:
- The inlier ratio threshold 'T_inlier', the larger the more strict. Hence, to get more circles, you can slightly tune it down.
- The sharp angle threshold 'sharp_angle'. To detect small circles, you can slightly tune it up
- The other parameters are usually fixed.

#### the fitting part:
 - The color difference threshold `COLOR_THRESHOLD` , To merge more similar colors,you can tune it up.
 - The minimize fitting area `MIN_area` ,To fit smaller shapes, you can tune it down.
