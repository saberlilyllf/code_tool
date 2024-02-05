#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <limits>


#include <kdtree_define.h>
#include <print_tree.h>

int main() {
    std::vector<Point> point_v {{30,40}, {5,25}, {10,12}, {70,70}, {50,30}, {35,45}};
    TreeNode* root = nullptr;
    for (auto& p : point_v) {
        insert(p, root, 0);
    }
    if (!root) {
        std::cout << "tree is empty" << std::endl;
    }
    printtree(root);
    std::cout << "min x point node is " << FindMin(root, 0, 0)->point_ << std::endl;
    std::cout << "min y point node is " << FindMin(root, 1, 0)->point_ << std::endl;

    delete_node_safely(Point(70,70), root, 0);
    printtree(root);

    // must be final to release memory
    if (root) {
        delete root;
    }
    return 0;
}