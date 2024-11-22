import matplotlib.pyplot as plt
import numpy as np

class Rectangle:
    def __init__(self, center_x, center_y, cos_heading, sin_heading, half_length, half_width):
        self.center_x = center_x
        self.center_y = center_y
        self.cos_heading = cos_heading
        self.sin_heading = sin_heading
        self.half_length = half_length
        self.half_width = half_width
        
        # Calculate the four corners of the rectangle in the local coordinate system
        self.local_corners = np.array([
            [-half_length, -half_width],
            [ half_length, -half_width],
            [ half_length,  half_width],
            [-half_length,  half_width]
        ])
        
        # Rotation matrix
        self.rotation_matrix = np.array([
            [cos_heading, -sin_heading],
            [sin_heading,  cos_heading]
        ])
        
        # Transform the local corners to the global coordinate system
        self.global_corners = (self.rotation_matrix  @ self.local_corners.T ).T + np.array([center_x, center_y])
        
    def draw_rectangle_and_arrow(self, ax, edgecolor, label = None, **kwargs):
        """
        Draw the rectangle and an arrow indicating the heading.
        
        Args:
            ax (matplotlib.axes.Axes, optional): The matplotlib axes to draw on. If None, a new figure will be created.
            **kwargs: Additional keyword arguments to pass to the polygon drawing function.
        """
        if ax is None:
            fig, ax = plt.subplots()
        
        # Draw the rectangle
        polygon = plt.Polygon(self.global_corners,  fill=False, edgecolor = edgecolor, label = label , **kwargs)
        ax.add_patch(polygon)
        
        # Calculate the arrow length (two-thirds of the rectangle's length)
        arrow_length = self.half_length * 2 * 2 / 3  # Rectangle's full length * 2/3
        
        # Calculate the endpoint of the arrow
        arrow_end_x = self.center_x + self.cos_heading * arrow_length
        arrow_end_y = self.center_y + self.sin_heading * arrow_length
        
        # Draw the arrow
        ax.arrow(self.center_x, self.center_y, arrow_end_x - self.center_x, arrow_end_y - self.center_y,
                 head_width=0.2, head_length=0.3, fc='r', ec='r')
        
        # Set the aspect to equal and display the plot
        ax.set_aspect('equal', adjustable='box')
        ax.grid(True)
        if ax is None:
            fig, ax = plt.subplots()
            plt.show()

# # Example usage:
# center_x, center_y = 0, 0
# cos_heading, sin_heading = np.cos(np.pi / 4), np.sin(np.pi / 4)  # 45 degrees
# half_length, half_width = 2, 1  # Rectangle half-length and half-width

# rect = Rectangle(center_x, center_y, cos_heading, sin_heading, half_length, half_width)
# rect.draw_rectangle_and_arrow()