#pragma once
#include <stdexcept>
#include <vector>

template <typename T>
class ArrayStack {
private:
    std::vector<T> data;

public:
    bool empty() const { return data.empty(); }
    std::size_t size() const { return data.size(); }

    void push(const T& value) { data.push_back(value); }

    T pop() {
        if (data.empty()) {
            throw std::runtime_error("Stack is empty.");
        }
        T value = data.back();
        data.pop_back();
        return value;
    }

    const T& top() const {
        if (data.empty()) {
            throw std::runtime_error("Stack is empty.");
        }
        return data.back();
    }
};
