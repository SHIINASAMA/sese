/**
 * @file LinkedQueue.h
 * @author kaoru
 * @date 2022年5月30日
 * @brief 非阻塞线程安全队列
 * @version 0.2
 * @see https://dl.acm.org/doi/pdf/10.1145/248052.248106
 */
#pragma once

#include <atomic>

namespace sese::concurrent {

// GCOVR_EXCL_START

/// \brief 非阻塞线程安全队列
/// \tparam T 模板类型
template<class T>
class LinkedQueue {
    /// \brief 节点
    struct Node {
        std::atomic<T> value{};
        std::atomic<Node *> next{nullptr};
    };

    std::atomic<Node *> head{nullptr};
    std::atomic<Node *> tail{nullptr};

public:
    LinkedQueue() {
        auto node = new Node();
        head.store(node);
        tail.store(node);
    }

    ~LinkedQueue() {
        auto p_node = head.load();
        while (p_node) {
            auto p_next = p_node->next.load();
            if (p_next == p_node) {
                p_next = nullptr;
            }
            delete p_node;
            p_node = p_next;
        }
    }

    void push(const T &value) {
        Node *tail;
        auto node = new Node();
        node->value.store(value);
        node->next.store(nullptr);
        while (true) {
            tail = this->tail.load();
            auto next = tail->next.load();
            if (tail == this->tail) {
                if (next == nullptr) {
                    if (tail->next.compare_exchange_weak(next, node)) {
                        break;
                    }
                } else {
                    this->tail.compare_exchange_weak(tail, next);
                }
            }
        }
        this->tail.compare_exchange_weak(tail, node);
    }

    bool pop(T &value) {
        Node *head;
        while (true) {
            head = this->head.load();
            auto tail = this->tail.load();
            auto next = head->next.load();
            if (head == this->head) {
                if (head == tail) {
                    if (next == nullptr) {
                        return false;
                    }
                    this->tail.compare_exchange_weak(tail, next);
                } else {
                    value = next->value;
                    if (this->head.compare_exchange_weak(head, next)) {
                        break;
                    }
                }
            }
        }
        delete head;
        return true;
    }

    bool empty() {
        return head == tail;
    }
};

// GCOVR_EXCL_STOP

} // namespace sese::concurrent