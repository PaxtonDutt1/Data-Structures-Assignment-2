#pragma once
#include <cstddef>
#include <stdexcept>

// A small generic singly linked list.
// TODO: Complete the operations marked below.
// You may add iterators or helper functions if useful.

template <typename T>
class LinkedList {
private:
    struct Node {
        T value;
        Node* next;
        explicit Node(const T& value) : value(value), next(nullptr) {}
    };

    Node* head{nullptr};
    std::size_t count{0};

public:
    LinkedList() = default;
    ~LinkedList() { clear(); }

    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    bool empty() const { return head == nullptr; }
    std::size_t size() const { return count; }

    void pushFront(const T& value) {
        // TODO
    }

    bool contains(const T& value) const {
        // TODO
        return false;
    }

    bool remove(const T& value) {
        // TODO
        return false;
    }

    void clear() {
        // TODO
    }

    // TODO: Add a traversal method that your project can use
    // without exposing Node publicly.
};
