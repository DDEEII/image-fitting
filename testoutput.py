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

draw_circle(128.96631, 114.0275, 180.42355, (8,168,232))
draw_circle(-83.590256, 144.52243, 124.73856, (232,24,40))
draw_circle(-192.22867, -93.7811, 96.38419, (40,184,72))
draw_polygon([(177,-129),(232,-190),(278,-198),(403,-198),(423,-157),(352,-128),(292,-80),(278,-60)], (248,200,8))
draw_polygon([(-63,-91),(-63,-186),(131,-186),(131,-91)], (168,72,168))
draw_polygon([(343,71),(320,-40),(452,-101),(476,49)], (248,168,200))
draw_circle(403.6997, 150.65521, 67.76178, (120,152,184))
draw_polygon([(4,328),(-74,241),(6,156),(8,198),(84,199),(84,285),(8,286)], (40,184,72))
draw_circle(428.46368, 306.4255, 57.90578, (248,248,8))
draw_circle(-190.06679, -95.03775, 57.233353, (40,184,72))
draw_circle(300.963, -129.55872, 51.050835, (248,200,8))
draw_polygon([(273,376),(340,241),(378,278),(372,320),(356,319),(364,375)], (248,248,8))
draw_circle(400.7575, -0.7470703, 45.34172, (248,168,200))
draw_polygon([(-16,-121),(-16,-156),(39,-173),(91,-156),(91,-121),(36,-103)], (136,8,24))
draw_polygon([(446,165),(421,196),(380,181),(359,129),(443,160)], (184,232,24))
