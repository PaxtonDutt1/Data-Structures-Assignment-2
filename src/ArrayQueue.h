#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>

// Circular-array queue.
// TODO: Implement enqueue, dequeue, front, resize, and wrap-around logic.
template <typename T>
class ArrayQueue {
private:
    std::vector<T> data;
    std::size_t frontIndex{0};
    std::size_t count{0};

    void grow() {
        // TODO: Allocate a larger logical queue and preserve FIFO order.
    }

public:
    explicit ArrayQueue(std::size_t initialCapacity = 8)
        : data(initialCapacity) {}

    bool empty() const { return count == 0; }
    std::size_t size() const { return count; }

    void enqueue(const T& value) {
        // TODO
    }

    T dequeue() {
        // TODO
        throw std::runtime_error("TODO: ArrayQueue::dequeue");
    }

    const T& front() const {
        // TODO
        throw std::runtime_error("TODO: ArrayQueue::front");
    }
};
