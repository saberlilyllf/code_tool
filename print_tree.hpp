#pragma once
#include <kdtree_define.h>
#include <queue>
#include <unordered_map>
class TreeNode;
class Point;

//二叉树打印
int tree_depth  = 0;
void count_tree_depth(TreeNode * node, int deep)
{
	tree_depth = deep > tree_depth ? deep : tree_depth;
    if (!node) {
        return;
    }
	if (node->left_node_ != nullptr)
		count_tree_depth(node->left_node_, deep + 1);
	if (node->right_node_ != nullptr)
		count_tree_depth(node->right_node_, deep + 1);
}

void printtree(TreeNode* root) {
	//确定二叉树深度
	tree_depth = -1;
	if (root!=nullptr) {
		count_tree_depth(root, tree_depth+1);
    } else {
		std::cout << "****" << std::endl << "nullptr" << std::endl << "****" << std::endl;
		return;
	}
	//层序遍历二叉树
	std::vector<std::vector<std::pair<TreeNode*, int>>> node;
	node.resize(tree_depth+1);
	std::queue<std::pair<TreeNode*, int>> lque;
	lque.push(std::make_pair(root, 0));
	TreeNode* Nuptr = nullptr;
	while (1) {
		TreeNode* cur = lque.front().first;
		int floor = lque.front().second;
		if (floor == tree_depth + 1)
			break;
		lque.pop();
		if (cur == nullptr)
		{
			lque.push(std::make_pair(Nuptr, floor + 1));
			lque.push(std::make_pair(Nuptr, floor + 1));
			node[floor].emplace_back(std::make_pair(nullptr, 0));
			continue;
		}
		if (cur->left_node_ != nullptr)
			lque.push(std::make_pair(cur->left_node_, floor + 1));
		else
			lque.push(std::make_pair(Nuptr, floor + 1));
		if (cur->right_node_ != nullptr)
			lque.push(std::make_pair(cur->right_node_, floor + 1));
		else
			lque.push(std::make_pair(Nuptr, floor + 1));
		node[floor].emplace_back(std::make_pair(cur, 0));
	}

	//构造最后一列
	std::unordered_map<int, std::vector<std::pair<TreeNode*, int>>> hash_map;
	std::vector<std::pair<TreeNode*, int>>::iterator it1;
	for (it1 = node[tree_depth].begin(); it1 != node[tree_depth].end(); it1++)
	{
		int num1 = it1 - node[tree_depth].begin() + 1;
		if (num1 % 2 == 1)
		{
			hash_map[(3 * num1 - 1) / 2].emplace_back(std::make_pair(((*it1).first), tree_depth));
			(*it1).second = (3 * num1 - 1) / 2;
		}
		else
		{
			hash_map[(3 * num1) / 2].emplace_back(std::make_pair(((*it1).first), tree_depth));
			(*it1).second = (3 * num1) / 2;
		}
	}
	it1--;
	int maxline = (*it1).second;
	//构造其余列
	std::vector<std::pair<TreeNode* , int>>::iterator it2;
	for (int i = tree_depth - 1 ; i >= 0; i--)
	{
		it1 = node[i + 1].begin();
		it2 = node[i].begin();
		while(it1 != node[i+1].end())
		{
			int line1 = (*it1).second;
			it1++;
			int line2 = (*it1).second;
			it1++;
			if (it2 - node[i].begin() < node[i].size()/2)//向上取整
				(*it2).second = (line1 + line2 + 1) / 2;
			else//向下取整
				(*it2).second = (line1 + line2) / 2;
			hash_map[(*it2).second].emplace_back(std::make_pair((*it2).first, i));
			it2++;
		}
	}
	std::cout << "***************" << std::endl;
	for (int i = maxline; i >= 1; i--)
	{
		std::vector<std::pair<TreeNode*, int>> vep = hash_map[i];
		int col = 0;
		int vsize = vep.size();
		for (int k = vsize - 1; k >= 0; k--)
		{
			for (; col < vep[k].second; col++)
				std::cout << "\t";
			if (vep[k].first != nullptr)
				std::cout << (vep[k].first)->point_;
			else
				std::cout << "nullptr";
		}
		std::cout << std::endl;
	}
	std::cout << "***************" << std::endl;
}
