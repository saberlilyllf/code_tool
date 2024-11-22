import sys
sys.setrecursionlimit(2147483646)
import matplotlib
matplotlib.use('TkAgg')  # 或 'Qt5Agg'
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
from matplotlib.widgets import Cursor
import json
import numpy as np
import math
import rect
import point_with_arrow

class DebugData(object):
    def __init__(self, cost_map, box_center_x, box_center_y, 
                 box_half_width, box_half_length, box_heading_cos, box_heading_sin ):
        """init"""
        self.cost_map = cost_map
        self.box_center_x = box_center_x
        self.box_center_y = box_center_y
        self.box_half_width = box_half_width
        self.box_half_length = box_half_length
        self.box_heading_cos = box_heading_cos
        self.box_heading_sin = box_heading_sin

class PathRecordAnalyzer(object):
    """path record analyzer"""
    def __init__(self):
        """init"""
        self.frame_to_plot = 0
        self.frame_data_map = {}
        self.init_seq_num = 0
        self.zoom_level = 1
        self.control_ax = []
        # read & parse
        if len(sys.argv) < 2:
            self.print_help()
            exit()
        # elif len(sys.argv) == 3:
        #     if sys.argv[2].isdigit():
        #         self.init_seq_num = int(sys.argv[2])
        #     else:
        #         print("[INPUT ERROR] Start SeqNum Input Should Be A INT Number!")
        #         self.print_help()
        json_path = sys.argv[1]
        if json_path == "":
            json_path = "/home/caros/cybertron/json/57.json"

        # 读取JSON文件
        with open(json_path, 'r') as f:
            data = json.load(f)
        #提取构造cost map的数据
        self.sequence_num = data['sequence_num']
        self.x_resolution = data['x_resolution']
        self.y_resolution = data['y_resolution']
        self.transform_tool = np.array(data['transform_map_to_local_'])
        self.cost_maps = np.array(data['index_time_x_y'])

        # 提取障碍物数据
        self.box_center_x = np.array(data['index_time_box_center_x'])
        self.box_center_y = np.array(data['index_time_box_center_y'])
        self.box_half_width = np.array(data['index_time_box_half_width'])
        self.box_half_length = np.array(data['index_time_box_half_length'])
        self.box_heading_cos = np.array(data['index_time_box_heading_cos'])
        self.box_heading_sin = np.array(data['index_time_box_heading_sin'])

        # # 找到最小和最大的x值 TODO
        # self.min_x_limit = np.min(self.box_center_x)
        # self.max_x_limit = np.max(self.box_center_x)
        # # 找到最小和最大的y值
        # self.min_y_limit = np.min(self.box_center_y)
        # self.max_y_limit = np.max(self.box_center_y)

        self.vehicle_x = data["ego_center_x"]
        self.vehicle_y = data["ego_center_y"]
        self.vehicle_heading = data["ego_heading"] 
        self.ego_half_length = data["ego_half_length"]
        self.ego_half_width = data["ego_half_width"]
        print("ego_center_x: ",self.vehicle_x, "ego_center_y: ", self.vehicle_y, "heading: ", self.vehicle_heading, "ego_half_length: ", self.ego_half_length, "ego_half_width: ", self.ego_half_width)
        # 获取对应的 sin 和 cos 值
        self.ego_sin_value = math.sin(self.vehicle_heading)
        self.ego_cos_value = math.cos(self.vehicle_heading)

        self.planning_start_point_x = data["planning_start_point_x"]
        self.planning_start_point_y = data["planning_start_point_y"]
        self.planning_start_point_theta = data["planning_start_point_theta"]
        print("planning_start_point_theta: ", self.planning_start_point_theta, "planning_start_point_x: ", self.planning_start_point_x, "planning_start_point_y: ", self.planning_start_point_y)
        self.planning_start_point_cos_value = math.cos(self.planning_start_point_theta)
        self.planning_start_point_sin_value = math.sin(self.planning_start_point_theta)
        for i in range((self.cost_maps.shape[0])):
            self.init_one_frame_data(i)
        # plot
        self.plt_prepare()
        plt.show()

    def init_one_frame_data(self,i):
        """init data for every time frame, total num is 25"""
        if i <= 0 :
            self.frame_to_plot = 0
        if i >= 25:
            self.frame_to_plot = 24
        self.frame_to_plot = i
        print("time_frame_to_plot ========== ", self.frame_to_plot)

        self.frame_data_map[self.frame_to_plot] = DebugData(self.cost_maps[self.frame_to_plot],
             self.box_center_x[self.frame_to_plot],
             self.box_center_y[self.frame_to_plot],
             self.box_half_width[self.frame_to_plot],
             self.box_half_length[self.frame_to_plot],
             self.box_heading_cos[self.frame_to_plot],
             self.box_heading_sin[self.frame_to_plot]
             )
        # reset to init frame
        if 24 == self.frame_to_plot:
            self.frame_to_plot = self.init_seq_num

    def roation_and_move(self, x_grid, y_grid):
        X_flat = x_grid.flatten()
        Y_flat = y_grid.flatten()
        x_y_index_points = np.vstack((X_flat.ravel(), Y_flat.ravel())).T

        # print(x_y_index_points.shape[0])
        # 创建一个形状为 (N, 1) 的全1数组，用于扩展
        ones_column = np.ones((x_y_index_points.shape[0], 1))
        
        # 使用 numpy.concatenate 沿列方向（axis=1）拼接原数组与全1数组
        x_y_local_points = np.concatenate((x_y_index_points, ones_column), axis=1)
        # 此时，expanded_arr 的形状为 (N, 3)
        # print(x_y_local_points)
        # x_y_local_points = np.dot(rotation_matrix2, x_y_local_points.T).T
        after_transform = np.dot(self.transform_tool, x_y_local_points.T).T
        after_transform2d = after_transform[:, :2]
        # 分离旋转后的坐标
        print("after_transform2d")
        print(after_transform2d)
        X_rotated_flat = after_transform2d[:, 0]
        Y_rotated_flat = after_transform2d[:, 1]

        # 重塑回二维网格的形状
        self.X_2d_local = X_rotated_flat.reshape(x_grid.shape)
        self.Y_2d_local = Y_rotated_flat.reshape(y_grid.shape)

    def find_best_grid_size(self, num_plots):
        # 找到最接近平方数的行列组合
        side_length = int(np.ceil(np.sqrt(num_plots)))
        
        # 检查side_length的平方是否恰好等于num_plots
        if side_length * side_length == num_plots:
            return side_length, side_length  # 如果是平方数，直接返回
        
        # 如果不是平方数，则找到一个最接近的矩形解
        # 从side_length开始递减，直到找到一个能被num_plots整除的数
        for r in range(side_length, 1, -1):
            if num_plots % r == 0:
                cols = r
                rows = num_plots // cols
                break  # 一旦找到解，就跳出循环
        
        # 此时rows和cols是找到的最近似的矩形解，直接返回
        return rows, cols
    def init_mesh_grid(self):
        x_size = self.cost_maps.shape[1]
        y_size = self.cost_maps.shape[2]
        x_coords = np.linspace(0, (x_size - 1) * self.x_resolution, x_size)
        y_coords = np.linspace(0, (y_size - 1) * self.y_resolution, y_size)
        print("x_coords shape: ", x_coords.shape, ", y_coords shape: ", y_coords.shape)
        X1, Y1 = np.meshgrid(x_coords, y_coords, indexing='xy')
        print("X1 shape: ", X1.shape, "Y1 shape: ", Y1.shape)
        self.roation_and_move(X1, Y1)
        print("X_2d_local shape: ", self.X_2d_local.shape, "Y_2d_local shape: ", self.Y_2d_local.shape)

    def plt_prepare(self):
        """plt related prepare"""
        self.init_mesh_grid()
        self.fig = plt.figure(figsize=(20, 20))
        self.plot_frame()
        self.path_cursor = Cursor(self.axis, useblit=True, color='dodgerblue', linewidth=1)
        self.callback = Index(self)
        prev1frame =  plt.axes([0.2, 0.01, 0.1, 0.05])
        prev10frame = plt.axes([0.3, 0.01, 0.1, 0.05])
        next1frame =  plt.axes([0.4, 0.01, 0.1, 0.05])
        next10frame = plt.axes([0.5, 0.01, 0.1, 0.05])
        zoom_out     = plt.axes([0.6, 0.01, 0.1, 0.05])
        zoom_in     = plt.axes([0.7, 0.01, 0.1, 0.05])
        exitframe =   plt.axes([0.8, 0.01, 0.1, 0.05])
        self.bprev1 = Button(prev1frame, '-1')
        self.bprev1.on_clicked(self.callback.prev1)
        self.bprev10 = Button(prev10frame, '-10')
        self.bprev10.on_clicked(self.callback.prev10)
        self.bnext1 = Button(next1frame, '+1')
        self.bnext1.on_clicked(self.callback.next1)
        self.bnext10 = Button(next10frame, '+10')
        self.bnext10.on_clicked(self.callback.next10)
        self.zoom_out = Button(zoom_out, 'z_out')
        self.zoom_out.on_clicked(self.callback.zoom_out)
        self.zoom_in = Button(zoom_in, 'z_in')
        self.zoom_in.on_clicked(self.callback.zoom_in)
        self.bexit = Button(exitframe, 'exit')
        self.bexit.on_clicked(self.callback.exit)
    
    def plot_frame(self, zoom_level = 1):
        """plot frame_to_plot in frame_data_map"""
        plt.style.use('_mpl-gallery-nogrid')

        # title = "cost map json debug tool" + "\n" + "    json: " + str(self.sequence_num) + "    plot: " + str(self.frame_to_plot) 
        # self.fig.text(0.45, 1.0,  # 左上角的位置 (x, y)
        #     title,
        #     fontsize=12,  # 可以根据需要调整字体大小
        #     verticalalignment='top')
        for ax in self.control_ax:
            self.fig.delaxes(ax)
        self.control_ax = []
        if zoom_level == 1:
            self.axis = self.fig.add_subplot(111)
            self.control_ax.append(self.axis)
            title = "cost map json debug tool" + "\n" + "json: " + str(self.sequence_num) + "    plot: " + str(self.frame_to_plot) 
            self.axis.set_title(title)
            self.axis.set_xlabel("X Coordinate (m)")
            self.axis.set_ylabel("Y Coordinate (m)")

            single_cost_map = self.frame_data_map[self.frame_to_plot].cost_map
            self.plot_single_cost_map(self.axis, single_cost_map)

            box_center_x = self.frame_data_map[self.frame_to_plot].box_center_x
            box_center_y = self.frame_data_map[self.frame_to_plot].box_center_y
            box_heading_cos = self.frame_data_map[self.frame_to_plot].box_heading_cos
            box_heading_sin = self.frame_data_map[self.frame_to_plot].box_heading_sin
            box_half_length = self.frame_data_map[self.frame_to_plot].box_half_length
            box_half_width = self.frame_data_map[self.frame_to_plot].box_half_width
            self.plot_obs(self.axis, box_center_x, box_center_y, box_heading_cos, box_heading_sin, box_half_length, box_half_width)

            self.plot_ego_box(self.axis)
            self.plot_start_point(self.axis)
            
            self.axis.legend()
            # self.axis.set_ylim(-10.0, 10.0)
            # TODO
            # if len(lk_debug.path_opt) != 0:
            #     self.axis.set_xlim(lk_debug.path_opt[0].s - 5.0, lk_debug.path_opt[-1].s + 5.0)
            self.axis.grid(True)
            self.axis.set_aspect(1)
            plt.subplots_adjust(left=0.1, right=0.9, top=0.9, bottom=0.1)
        else:

            plot_num = zoom_level * zoom_level
            rows, cols = self.find_best_grid_size(plot_num)
            for i in range(plot_num):
                r, c = divmod(i, cols)
                print("current self.frame_to_plot + i ", self.frame_to_plot + i ," r, c ", r, c, "rows, cols ", rows, cols, "i + 1: ", i+ 1, "zoom_level: ", zoom_level)
                
                self.axis = self.fig.add_subplot(rows, cols, i + 1)
                self.control_ax.append(self.axis)
                # 设置子图的标题，以便区分它们
                ax = self.axis 

                title = "  json: " + str(self.sequence_num) + "    plot: " + str(self.frame_to_plot + i) 
                ax.set_title(title, fontsize=12 - zoom_level)  # 这里将字体大小设置为12

                # ax.set_xlabel("              X  (m)")
                # ax.set_ylabel("              Y (m)")
                # ax.contour(X_2d_local, Y_2d_local, cost_maps[i], levels=levels)
                single_cost_map = self.frame_data_map[self.frame_to_plot + i].cost_map
                self.plot_single_cost_map(ax, single_cost_map)

                box_center_x = self.frame_data_map[self.frame_to_plot + i].box_center_x
                box_center_y = self.frame_data_map[self.frame_to_plot + i].box_center_y
                box_heading_cos = self.frame_data_map[self.frame_to_plot + i].box_heading_cos
                box_heading_sin = self.frame_data_map[self.frame_to_plot + i].box_heading_sin
                box_half_length = self.frame_data_map[self.frame_to_plot + i].box_half_length
                box_half_width = self.frame_data_map[self.frame_to_plot + i].box_half_width
                self.plot_obs(ax, box_center_x, box_center_y, box_heading_cos, box_heading_sin, box_half_length, box_half_width)

                self.plot_ego_box(ax)
                self.plot_start_point(ax)

                ax.tick_params(axis='both', which='major', labelsize=12 - zoom_level)
                self.axis.grid(True)
               
                # plot_obs(i, ax)
                # plot_ego_box(ax)
                # plot_start_point(ax)
            plt.style.use('_mpl-gallery-nogrid')
            # plt.tight_layout()

        # self.axis.set_aspect(1)
        plt.subplots_adjust(left=0.1, right=0.9, top=0.9, bottom=0.1)
        print ("self.frame_to_plot11", self.frame_to_plot)

    def plot_single_cost_map(self, ax, single_cost_map):
        ax.pcolormesh(self.X_2d_local, self.Y_2d_local, single_cost_map.T)
        # self.axis.plot(self.X_2d_local, self.Y_2d_local, 'b--', label='left lane bound')
    def plot_obs(self, ax, box_center_x, box_center_y, 
                 box_heading_cos, box_heading_sin, box_half_length, box_half_width):
        obs_size = box_center_x.shape[0]
        for i in range(obs_size):
            # if i == 1:
            #     print("box_center_x[i] is  ",box_center_x[i])
            rect_obj = rect.Rectangle(box_center_x[i], box_center_y[i], box_heading_cos[i],
                                        box_heading_sin[i], box_half_length[i], box_half_width[i])
            rect_obj.draw_rectangle_and_arrow(ax, edgecolor="b")

    def plot_ego_box(self, ax):
        rect_obj = rect.Rectangle(self.vehicle_x, self.vehicle_y, self.ego_cos_value, self.ego_sin_value,
                                    self.ego_half_length,  self.ego_half_width)
        rect_obj.draw_rectangle_and_arrow(ax, edgecolor="r", label = "Ego Vehicle Box")
        
    def plot_start_point(self, ax):
        point_obj = point_with_arrow.PointWithArrow(self.planning_start_point_x, self.planning_start_point_y, 
                                    self.planning_start_point_cos_value, self.planning_start_point_sin_value)
        point_obj.draw(ax)
        
    def print_help(self):
        """print help"""
        print("****** How To Use path_record_analyzer.py ****** ")
        print("python3 cost_map_debug.py\t[json_file]")

class MouseEventManager(object):
    """MouseEventManager"""
    x, y = 0.0, 0.0
    xoffset, yoffset = -20, 20
    text_template = 'x: %0.2f\ny: %0.2f'
    annotation = False

    def on_click(self, event):
        """click callback"""
        # if mouse button is not right, return
        # 1: left, 2: middle, 3: right
        if event.button != 3:
            return
        self.x, self.y = event.xdata, event.ydata
        if self.x is not None:
            print('mouse click x: %.2f, y: %.2f' % (event.xdata, event.ydata))
            if self.annotation:
                self.annotation.set_visible(False)
            label_text = self.text_template % (self.x, self.y)
            self.annotation = event.inaxes.annotate(label_text,
                xy=(self.x, self.y), xytext=(self.xoffset, self.yoffset),
                textcoords='offset points', ha='right', va='bottom',
                bbox=dict(boxstyle='round,pad=0.5', fc='lightcyan', alpha=0.5),
                arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0')
                )
            self.annotation.set_visible(True)
            self.annotation.figure.canvas.draw()

class Index(object):
    """button callback function"""
    def __init__(self, path_record_analyzer):
        self.path_record_analyzer = path_record_analyzer
        self.reset_mouse_event()

    def reset_mouse_event(self):
        """reset mouse event"""
        self.mouse_manager = MouseEventManager()
        self.path_record_analyzer.fig.canvas.mpl_connect('button_release_event', self.mouse_manager.on_click)

    def next(self, step):
        """next button callback function"""
        self.path_record_analyzer.frame_to_plot += step
        if self.path_record_analyzer.frame_to_plot not in self.path_record_analyzer.frame_data_map:
            print("[ERROR][next] target frame " + str(self.path_record_analyzer.frame_to_plot) + " not found!")
            self.path_record_analyzer.frame_to_plot -= step
            return
        self.path_record_analyzer.plot_frame()
        plt.show()
        self.reset_mouse_event()

    def internal_zoom_out(self):
        """zoom out button callback function"""
        zoom_level = self.path_record_analyzer.zoom_level + 1
        if zoom_level > 5:
            print("[ERROR][internal_zoom_out] max zoom level reached: ", zoom_level)
            zoom_level = zoom_level - 1
        self.path_record_analyzer.zoom_level = zoom_level
        num =  zoom_level * zoom_level
        frame_to_plot = self.path_record_analyzer.frame_to_plot + num - 1
        if frame_to_plot not in self.path_record_analyzer.frame_data_map:
            print("[ERROR][next] target frame " + str(frame_to_plot) + " not found! ")
            self.path_record_analyzer.frame_to_plot = 25 - num
        
        print("self.path_record_analyzer.frame_to_plot," , self.path_record_analyzer.frame_to_plot)
        self.path_record_analyzer.plot_frame(self.path_record_analyzer.zoom_level)

        if frame_to_plot not in self.path_record_analyzer.frame_data_map:
            print("[ERROR][next] target frame " + str(frame_to_plot) + " not found! ")
            self.path_record_analyzer.frame_to_plot = 24
        else:
            self.path_record_analyzer.frame_to_plot = frame_to_plot
        plt.show()
        self.reset_mouse_event()

    def internal_zoom_in(self):
        """zoom in button callback function"""
        zoom_level = self.path_record_analyzer.zoom_level - 1
        if zoom_level < 1:
            print("[ERROR][internal_zoom_in] max zoom level reached: ", zoom_level)
            zoom_level = zoom_level + 1
        self.path_record_analyzer.zoom_level = zoom_level
        num =  zoom_level * zoom_level
        frame_to_plot = self.path_record_analyzer.frame_to_plot + num
        if frame_to_plot not in self.path_record_analyzer.frame_data_map:
            print("[ERROR][next] target frame " + str(frame_to_plot) + " not found! ")
            self.path_record_analyzer.frame_to_plot = 25 - num
        self.path_record_analyzer.plot_frame(self.path_record_analyzer.zoom_level)
        if frame_to_plot not in self.path_record_analyzer.frame_data_map:
            print("[ERROR][next] target frame " + str(frame_to_plot) + " not found! ")
            self.path_record_analyzer.frame_to_plot = 24
        else:
            self.path_record_analyzer.frame_to_plot = frame_to_plot
        plt.show()
        self.reset_mouse_event()

    def next1(self, event):
        """next 1 button callback function"""
        self.next(1)

    def next10(self, event):
        """next 10 button callback function"""
        self.next(10)
    def zoom_out(self, event):
        self.internal_zoom_out()
    def zoom_in(self, event):
        self.internal_zoom_in()
    def prev(self, step):
        """prev button callback function"""
        self.path_record_analyzer.frame_to_plot -= step
        if self.path_record_analyzer.frame_to_plot not in self.path_record_analyzer.frame_data_map:
            print("[ERROR][prev] target frame " + str(self.path_record_analyzer.frame_to_plot) + " not found!")
            self.path_record_analyzer.frame_to_plot += step
            return
        self.path_record_analyzer.plot_frame()
        plt.show()
        self.reset_mouse_event()

    def prev1(self, event):
        """prev button callback function"""
        self.prev(1)

    def prev10(self, event):
        """prev button callback function"""
        self.prev(10)

    def exit(self, event):
        """next thread button callback function"""
        sys.exit(0)


if __name__ == "__main__":
    foo = PathRecordAnalyzer()
