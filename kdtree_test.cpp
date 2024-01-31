#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <limits>


#include <kdtree_define.h>
#include <print_tree.h>

int main() {
    std::vector<Point> point_v {{5,25}, {10,12}, {70,70}, {50,30}, {35,45}};
    TreeNode* root = new TreeNode({Point(30,40)});
    for (auto& p : point_v) {
        insert(p, root, 0);
    }
    if (!root) {
        std::cout << "tree is empty" << std::endl;
    }
    printtree(root);

    return 0;
}