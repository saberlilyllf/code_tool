#pragma once
#include <iostream>
#include <limits.h>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <thread>
#include <exception>
#include <functional>

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

uint32_t const max_hazard_pointers = 100;
struct HazardPointer {
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};
HazardPointer hazard_pointers[max_hazard_pointers];

class HpOwner {
    HazardPointer* hp;
public:
    HpOwner(const HpOwner&) = delete;
    HpOwner() : hp (nullptr) {
        for (uint32_t i = 0; i < max_hazard_pointers; ++i) {
            std::thread::id old_id;
            if (hazard_pointers[i].id.compare_exchange_strong(old_id, std::this_thread::get_id())) {
                hp = &hazard_pointers[i];
                break;
            }
        }
        if (!hp) {
            throw std::runtime_error("No Hazard Pointers avaliable");
        }
    }
    ~HpOwner() {
        hp->pointer.store(nullptr);
        hp->id.store(std::thread::id());
    }
    std::atomic<void*>& GetPointer() {
        return hp->pointer;
    }
};

std::atomic<void*>& GetHazardPointerForCurrentThread() {
    thread_local static HpOwner hazard;
    return hazard.GetPointer();
}

bool OutstandingHazardPointersFor(void* p) {
    for (uint32_t i = 0; i < max_hazard_pointers; ++i) {
        if (hazard_pointers[i].pointer.load() == p) {
            return true;
        }
    }
    return false;
}

template<typename T>
void do_delete(void* p) {
    delete static_cast<T*>(p);
}

struct DataToReclaim {
    void* data;
    std::function<void(void*)> deleter;
    DataToReclaim* next;
    template<typename T>
    DataToReclaim(T* p) : data (p), deleter(&do_delete<T>), next(nullptr) {}
    ~DataToReclaim() {
        deleter(data);
    }
};

std::atomic<DataToReclaim*> nodes_to_reclaim;
void add_to_reclaim_list(DataToReclaim* node) {
    if (!node) {
        return;
    }
    node->next = nodes_to_reclaim.load();
    while (!nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

void DeleteNodesWithNoHazards() {
    DataToReclaim* current = nodes_to_reclaim.exchange(nullptr);
    while (current) {
        DataToReclaim* next = current->next;
        if (!OutstandingHazardPointersFor(current->data)) {
            delete current;
        } else {
            add_to_reclaim_list(current);
        }
        current = next;
    }
}

template<typename T>
void ReclaimLater(T* data) {
    add_to_reclaim_list(new DataToReclaim(data));
}
template<typename T>
class LockFreeStackList77 {
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
        std::atomic<void*>& hp = GetHazardPointerForCurrentThread();
        Node* old_head = head.load();
        do {
            Node* temp = nullptr;
            do {
                temp = old_head;
                hp.store(old_head);
                old_head = head.load();
            } while (old_head != temp);
        } while(old_head && !head.compare_exchange_weak(old_head, old_head->next));
        hp.store(nullptr);
        std::shared_ptr<T> res;
        if(old_head) {
            res.swap(old_head->data);
            if (OutstandingHazardPointersFor(old_head)) {
                ReclaimLater(old_head);
            } else {
                delete old_head;
            }
            DeleteNodesWithNoHazards();
        }
        return res;
    }
};

// using book 7.11 page 229
template<typename T>
class LockFreeStackCount {
private:
    struct Node;
    struct CountedNodePtr {
        int external_count;
        Node* ptr;
    };

    struct Node {
        std::shared_ptr<T> data;
        std::atomic<int32_t> internal_count;
        Node (const T& data_) : data(data_), internal_count(0) {}
    };
    std::atomic<CountedNodePtr> head;
    void IncreaseHeadCount(CountedNodePtr& old_counter) {
        CountedNodePtr new_counter;
        do {
            new_counter = old_counter;
            ++new_counter.excternal_count;
        } while (!head.compare_exchange_strong(old_counter, new_counter));
        old_counter.external_count = new_counter.external_count;
    }
public:
    ~LockFreeStackCount() {
        while (Pop());
    }
    void Push(const T& data) {
        CountedNodePtr new_node;
        new_node.ptr = new Node(data);
        new_node.external_count = 1;
        new_node.ptr->next = head.load();
        while(head.compare_exchange_weak(new_node.ptr->next, new_node));
    }
    std::shared_ptr<T> Pop() {
        CountedNodePtr old_head = head.load();
        Node* ptr = old_head.ptr;
        if (!ptr) {
            return std::shared_ptr<T>();
        }
        if (head.compare_exchande_strong(old_head, ptr->next)) {
            std::shared_ptr<T> res;
            res.swap(ptr->data);
            int32_t count_increase = old_head.external_count - 2;
            if (ptr->internal_count.fetch_add(count_increase) == -count_increase) {
                delete ptr;
            }
            return res;
        } else if (ptr->internal_count.fetch_sub(1) == 1) {
            delete ptr;
        }
    }
};