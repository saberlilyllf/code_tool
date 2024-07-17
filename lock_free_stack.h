#pragma once
#include <iostream>
#include <limits.h>
#include <algorithm>
#include <cmath>
#include <atomic>

template<typename T>
class LockFreeStackLeakNode {
    struct Node {
        std::shared_ptr<T> data;
        Node* next;
        Node(const T& data_node) : data(std::make_shared<T>(data_node)) {}

    };
    std::atomic<Node*> head;
public:
    void Push(const T& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node));
    }
    std::shared_ptr<T> pop() {
        Node* old_head = head.load();
        while(old_head && !head.compare_exchange_weak(old_head, old_head->next));
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
};

template<typename T>
class LockFreeStackList74 {
    struct Node {
        std::shared_ptr<T> data;
        Node* next;
        Node(const T& data_node) : data(std::make_shared<T>(data_node)) {}

    };
    std::atomic<Node*> head;
    std::atomic<uint32_t> threads_in_pop;
    std::atomic<Node*> to_be_deleted;
    void ChainPendingNodes(Node* nodes);
    void ChainPendingNode(Node* nodes);
    void ChainPendingNodes(Node* first, Node* last);
    void TryReclaim(Node* old_head);
    static void DeleteNodes(Node* nodes) {
        while (nodes) {
            Node* next = nodes->next;
            delete nodes;
            nodes = next;
        }
    }
public:
    void Push(const T& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node));
    }
    std::shared_ptr<T> pop() {
        ++threads_in_pop;
        Node* old_head = head.load();
        while(old_head && !head.compare_exchange_weak(old_head, old_head->next));
        std::shared_ptr<T> res;
        if(old_head) {
            res.swap(old_head->data);
        }
        TryReclaim(old_head);
        return res;
    }
};

template<typename T>
void LockFreeStackList74<T>::ChainPendingNodes(Node* first, Node* last) {
    last->next = to_be_deleted.load();
    while (!to_be_deleted.compare_exchange_weak(last->next, first));
}

template<typename T>
void LockFreeStackList74<T>::ChainPendingNode(Node* nodes) {
    ChainPendingNodes(nodes, nodes);
}

template<typename T>
void LockFreeStackList74<T>::ChainPendingNodes(Node* nodes) {
    Node* last = nodes;
    Node* next = last->next;
    while (next) {
        last = next;
        next = last->next;
    }
    ChainPendingNodes(nodes, last);
}

template<typename T>
void LockFreeStackList74<T>::TryReclaim(Node* old_head) {
    if (threads_in_pop == 1) {
        Node* nodes_to_delete = to_be_deleted.exchange(nullptr);
        if (--threads_in_pop != 0) {
            DeleteNodes(nodes_to_delete);
        } else if (nodes_to_delete) {
            ChainPendingNodes(nodes_to_delete);
        }
        delete old_head; // why here doing in this way?
    } else {
        ChainPendingNode(old_head);
        --threads_in_pop;
    }

}