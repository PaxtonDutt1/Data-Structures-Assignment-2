#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>

// Circular-array queue.
// Elements occupy count slots starting at frontIndex, wrapping past the end.
template <typename T>
class ArrayQueue {
private:
    std::vector<T> data;
    std::size_t frontIndex{0};
    std::size_t count{0};

    // Doubles the capacity and moves elements so the front is at index 0.
    void grow() {
        const std::size_t newCapacity = data.empty() ? 8 : data.size() * 2;
        std::vector<T> larger(newCapacity);
        for (std::size_t i = 0; i < count; ++i) {
            larger[i] = data[(frontIndex + i) % data.size()];
        }
        data.swap(larger);
        frontIndex = 0;
    }

public:
    explicit ArrayQueue(std::size_t initialCapacity = 8)
        : data(initialCapacity) {}

    bool empty() const { return count == 0; }
    std::size_t size() const { return count; }
    std::size_t capacity() const { return data.size(); }

    void enqueue(const T& value) {
        if (count == data.size()) {
            grow();
        }
        data[(frontIndex + count) % data.size()] = value;
        ++count;
    }

    T dequeue() {
        if (empty()) {
            throw std::runtime_error("Queue is empty.");
        }
        T value = data[frontIndex];
        frontIndex = (frontIndex + 1) % data.size();
        --count;
        return value;
    }

    const T& front() const {
        if (empty()) {
            throw std::runtime_error("Queue is empty.");
        }
        return data[frontIndex];
    }

    // Calls visit(value) on each element, front to back.
    template <typename Visitor>
    void forEach(Visitor visit) const {
        for (std::size_t i = 0; i < count; ++i) {
            visit(data[(frontIndex + i) % data.size()]);
        }
    }
};