#pragma once
#include <iostream>
#include <limits.h>
#include <algorithm>

struct Point{
    Point(const double& x, const double& y) : point_x(x), point_y(y) {}
    ~Point(){}

    bool operator==(const Point& p) const {
        if ((point_x == p.point_x) && (point_y == p.point_y)) {
            return true;
        }
        return false;
    }

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
        std::cout << "node delete with point" << point_ << std::endl;
        delete left_node_;
        delete right_node_;
    }
    Point point_;
    TreeNode* left_node_{nullptr};
    TreeNode* right_node_{nullptr};
};

// return the smaller point with node
TreeNode*& smaller_node(TreeNode*& node1, TreeNode*& node2, uint32_t dim) {
    if (!node1 && !node2) {
        return node1;
    }
    if (!node1) {
        return node2;
    }
    if (!node2) {
        return node1;
    }
    return ((node1->point_.get_dim(dim) < node2->point_.get_dim(dim)) ? node1 : node2);
}

TreeNode* insert(const Point& point, TreeNode*& node, uint32_t dim) {
    if(!node) {
        node = new TreeNode(point);
    } else if (point.get_dim(dim) <= node->point_.get_dim(dim)) {
        node->left_node_ = insert(point, node->left_node_, (++dim) % 2);
    } else {
     	node->right_node_ = insert(point, node->right_node_,( ++dim) % 2);
	}
	return node;
}

// return the node with dim which is min
TreeNode* FindMin(TreeNode*& node, uint32_t dim, uint32_t current_dim) {
    if (!node) {
        return nullptr;
    }

    if (dim == current_dim) {
        if (node->left_node_) {
            return FindMin(node->left_node_, dim, ++current_dim % 2);
        } else {
            return node;
        }
    } else {
        double min1 = std::numeric_limits<double>::max();
        double min2 = std::numeric_limits<double>::max();
        TreeNode* node_1 = nullptr;
        TreeNode* node_2 = nullptr;
        if (node->left_node_) {
            node_1 = FindMin(node->left_node_, dim, ++current_dim % 2);
        }
        if (node->right_node_) {
            node_2 = FindMin(node->right_node_, dim, ++current_dim % 2);
        }
        return smaller_node(smaller_node(node_1, node_2, dim), node, dim);
    }
}

TreeNode* find_node(const Point& data, TreeNode*& node, uint32_t dim) {
    if (!node) {
        return nullptr;
    }
    if (data == node->point_) {
        return node;
    }
    uint32_t next_dim = dim + 1;
    if (node->left_node_ && (data.get_dim(dim) < node->point_.get_dim(dim))) {
        return find_node(data, node->left_node_, next_dim % 2);
    } else if (node->right_node_ && (data.get_dim(dim) >= node->point_.get_dim(dim))) {
        return find_node(data, node->right_node_, next_dim % 2);
    }
    return nullptr;
}

TreeNode* delete_node(const Point& data, TreeNode* node, uint32_t dim) {
    if (!node) {
        return nullptr;
    }
    uint32_t next_dim = (dim + 1) % 2;
    if (data == node->point_) {
        if (node->right_node_) {
            TreeNode* node_temp_left = node->left_node_;
            auto new_node = FindMin(node->right_node_, dim, next_dim);
            auto delete_node_1 =  delete_node(new_node->point_, node->right_node_, next_dim);
            new_node->right_node_ = delete_node_1;
            if (delete_node_1) {
                std::cout  << "   delete node is " << delete_node_1->point_ << "      " << __LINE__  << std::endl;
            }
            // new_node->right_node_ = delete_node(new_node->point_, node->right_node_, next_dim);
            std::cout << "dim is " << dim << "  next_dim is  " << next_dim << "  min node is " << new_node->point_ << __LINE__ <<std::endl;
            new_node->left_node_ = node_temp_left;
            node->left_node_ = nullptr;
            node->right_node_ = nullptr;
            // delete node;
            return new_node;
        }
        if (node->left_node_) {
            auto new_node = FindMin(node->left_node_, dim, next_dim);
            std::cout << "dim is " << dim << "  next_dim is  " << next_dim << "   min node is " << new_node->point_ << "      " << __LINE__  << std::endl;
            // auto delete_node_1 =  delete_node(new_node->point_, node->right_node_, next_dim);
            // new_node->right_node_ = delete_node_1;
            // if (delete_node_1) {
            //     std::cout  << "   delete node is " << delete_node_1->point_ << "      " << __LINE__  << std::endl;
            // }
            new_node->right_node_ = delete_node(new_node->point_, node->left_node_, next_dim);
            std::cout << "dim is " << dim << "  next_dim is  " << next_dim << "   min node is " << node->left_node_->point_ << "      " << __LINE__  << std::endl;
             
            node->left_node_ = nullptr;
            node->right_node_ = nullptr;
            // delete node;
            return new_node;
        }
        // delete node;
        return nullptr;
    }

    if (data.get_dim(dim) < node->point_.get_dim(dim)) {
        node->left_node_ = delete_node(data, node->left_node_, next_dim);
        std::cout << "dim is " << dim << "  next_dim is  " << next_dim << "   min node is " << node->left_node_->point_ << "      " << __LINE__  << std::endl;
    } else {
        node->right_node_ = delete_node(data, node->right_node_, next_dim);
        std::cout << "dim is " << dim << "  next_dim is  " << next_dim << "   min node is " << node->left_node_->point_ << "      " << __LINE__  << std::endl;
    }
    return node;
}

void delete_node_safely(const Point& data, TreeNode*& node, uint32_t dim) {
    TreeNode* found_node = find_node(data, node, dim);
    if (!found_node) {
        std::cout << "node is not found" << std::endl;
        return;
    }
    std::cout << "dim is " << dim << "   find node is " << found_node->point_ << "      " << __LINE__  << std::endl;

    delete_node(data, node, dim);
    delete found_node;
    found_node = nullptr;

}
