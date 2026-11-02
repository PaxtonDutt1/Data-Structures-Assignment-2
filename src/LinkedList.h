#pragma once
#include <cstddef>
#include <stdexcept>

// A small generic singly linked list.
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
        Node* node = new Node(value);
        node->next = head;
        head = node;
        ++count;
    }

    bool contains(const T& value) const {
        for (Node* current = head; current != nullptr; current = current->next) {
            if (current->value == value) {
                return true;
            }
        }
        return false;
    }

    // Removes the first node equal to value. Returns false if none exists.
    bool remove(const T& value) {
        Node* previous = nullptr;
        Node* current = head;
        while (current != nullptr) {
            if (current->value == value) {
                if (previous == nullptr) {
                    head = current->next;
                } else {
                    previous->next = current->next;
                }
                delete current;
                --count;
                return true;
            }
            previous = current;
            current = current->next;
        }
        return false;
    }

    void clear() {
        while (head != nullptr) {
            Node* next = head->next;
            delete head;
            head = next;
        }
        count = 0;
    }

    // Calls visit(value) on each element, front to back.
    template <typename Visitor>
    void forEach(Visitor visit) const {
        for (Node* current = head; current != nullptr; current = current->next) {
            visit(current->value);
        }
    }
};
