#pragma once
#include <iostream>

struct Point{
    Point(const double& x, const double& y) : point_x(x), point_y(y) {}
    ~Point(){}

    double get_dim(uint32_t dim) const {
        if (dim == 0) {
            return point_x;
        } else {
            return point_y;
        }

    }
    double point_x;
    double point_y;
    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

std::ostream& operator << (std::ostream& os, const Point& p) {
    os << '(' << p.point_x << ',' << p.point_y << ')';
    return os;
}

struct TreeNode {
    TreeNode(const Point& p) : point_(p){}
    ~TreeNode() {
        if (!left_node_) {
            delete left_node_;

        }
        if (!right_node_) {
            delete right_node_;
        }
    }
    Point point_;
    TreeNode* left_node_{nullptr};
    TreeNode* right_node_{nullptr};
};

TreeNode* insert(const Point& point, TreeNode*& node, uint32_t dim) {
    if(!node) {
        node = new TreeNode(point);
    } else if (point.get_dim(dim) <= node->point_.get_dim(dim)) {
        node->left_node_ = insert(point, node->left_node_, ++dim % 2);
    } else {
     	node->right_node_ = insert(point, node->right_node_, ++dim % 2);
	}
	return node;
}