import matplotlib.pyplot as plt
import numpy as np

class PointWithArrow:
    def __init__(self, x, y, cos, sin, arrow_length = 1):
        self.x = x
        self.y = y
        self.cos = cos
        self.sin = sin
        self.arrow_length = arrow_length
        
    def draw(self, ax, label = None):
        if ax is None:
            fig, ax = plt.subplots()
        
        # 绘制点
        point_radius = 0.1  # 点的大小
        circle = plt.Circle((self.x, self.y), point_radius, color='blue', fill=True, label = label)
        ax.add_patch(circle)
        
        # 计算箭头的方向向量
        dx = self.arrow_length * self.cos
        dy = self.arrow_length * self.sin
        
        # 绘制箭头
        headwidth = 0.2  # 箭头头部宽度
        headlength = 0.3  # 箭头头部长度
        
        ax.arrow(self.x, self.y, dx, dy, head_width=headwidth, head_length=headlength, fc='blue', ec='blue')
        
        # 设置轴的比例和显示范围
        # ax.set_aspect('equal')
        # ax.set_xlim(self.x - point_radius - headwidth - self.arrow_length, self.x + point_radius + headwidth + self.arrow_length)
        # ax.set_ylim(self.y - point_radius - headwidth - self.arrow_length, self.y + point_radius + headwidth + self.arrow_length)
        
        # 去掉坐标轴
        # ax.axis('off')
        if ax is None:
            plt.show()

# 示例使用
if __name__ == "__main__":
    # 定义点的坐标和箭头方向（cos, sin）
    x = 0
    y = 0
    cos = 1  # 箭头方向沿x轴正方向
    sin = 0
    
    # 创建并绘制点和箭头
    point_with_arrow = PointWithArrow(x, y, cos, sin, arrow_length = 2)
    point_with_arrow.draw()