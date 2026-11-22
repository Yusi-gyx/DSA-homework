#pragma once
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

// ============================================================================
// XorLinkedList - XOR 双向链表
// ============================================================================
template <class T>
class XorLinkedList {
    struct Node {
        T val;
        uintptr_t xor_ptr;  // prev ^ next
        
        Node(T v = 0) : val(v), xor_ptr(0) {}
    };
    
    Node* head_;
    Node* tail_;
    std::size_t size_;
    
    // 将指针转换为 uintptr_t
    static uintptr_t ptr_to_int(Node* p) {
        return reinterpret_cast<uintptr_t>(p);
    }
    
    // 将 uintptr_t 转换为指针
    static Node* int_to_ptr(uintptr_t i) {
        return reinterpret_cast<Node*>(i);
    }
    
    // 计算 XOR
    static uintptr_t xor_ptrs(Node* a, Node* b) {
        return ptr_to_int(a) ^ ptr_to_int(b);
    }
    
    // 已知 prev 和 xor_ptr，求 next
    static Node* get_next(Node* prev, Node* cur) {
        return int_to_ptr(cur->xor_ptr ^ ptr_to_int(prev));
    }
    
    // 获取第 index 个节点，返回 (prev, cur)
    std::pair<Node*, Node*> get_node(int index) const {
        Node* prev = nullptr;
        Node* cur = head_;
        for (int i = 0; i < index; ++i) {
            Node* next = get_next(prev, cur);
            prev = cur;
            cur = next;
        }
        return {prev, cur};
    }

public:
    XorLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}
    
    ~XorLinkedList() {
        Node* prev = nullptr;
        Node* cur = head_;
        while (cur) {
            Node* next = get_next(prev, cur);
            delete prev;
            prev = cur;
            cur = next;
        }
        delete prev;
    }
    
    XorLinkedList(const XorLinkedList&) = delete;
    XorLinkedList& operator=(const XorLinkedList&) = delete;
    
    void insert(int index, T val) {
        auto [prev, next] = get_node(index);
        Node* node = new Node(val);
        node->xor_ptr = xor_ptrs(prev, next);

        if (prev) {
            prev->xor_ptr ^= ptr_to_int(next) ^ ptr_to_int(node);
        } else {
            head_ = node;
        }

        if (next) {
            next->xor_ptr ^= ptr_to_int(prev) ^ ptr_to_int(node);
        } else {
            tail_ = node;
        }

        ++size_;
    }
    
    void remove(int index) {
        auto [prev, node] = get_node(index);
        Node* next = get_next(prev, node);

        if (prev) {
            prev->xor_ptr ^= xor_ptrs(node, next);
        } else {
            head_ = next;
        }

        if (next) {
            next->xor_ptr ^= xor_ptrs(node, prev);
        } else {
            tail_ = prev;
        }

        delete node;
        --size_;
    }
    
    T query(int index) const {
        auto [prev, cur] = get_node(index);
        return cur->val;
    }
    
    std::size_t size() const { return size_; }

    void push_front(T val) { insert(0, val); }
    void push_back(T val) { insert(static_cast<int>(size_), val); }
    void pop_front() { remove(0); }
    void pop_back() { remove(static_cast<int>(size_ - 1)); }
    T front() const { return query(0); }
    T back() const { return query(static_cast<int>(size_ - 1)); }
    
    // 迭代器 - XOR list 的迭代器需要记住前一个节点
    class Iterator {
        Node* prev_;
        Node* cur_;
        
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        Iterator(Node* prev, Node* cur) : prev_(prev), cur_(cur) {}
        
        T& operator*() { return cur_->val; }
        
        Iterator& operator++() {
            Node* next = XorLinkedList::get_next(prev_, cur_);
            prev_ = cur_;
            cur_ = next;
            return *this;
        }
        
        Iterator& operator--() {
            Node* prev_prev = XorLinkedList::get_next(cur_, prev_);
            cur_ = prev_;
            prev_ = prev_prev;
            return *this;
        }
        
        bool operator==(const Iterator& o) const { return cur_ == o.cur_; }
        bool operator!=(const Iterator& o) const { return cur_ != o.cur_; }
    };
    
    Iterator begin() { return Iterator(nullptr, head_); }
    Iterator end() { return Iterator(tail_, nullptr); }
    
    // 节点大小信息
    static constexpr std::size_t node_size() { return sizeof(Node); }
    static constexpr std::size_t regular_node_size() { 
        // 普通双向链表节点：val + prev + next
        return sizeof(T) + 2 * sizeof(void*); 
    }
};
