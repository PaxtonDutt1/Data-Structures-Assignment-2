#pragma once
#include <cstddef>
#include <functional>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

// Separate-chaining hash table.
// TODO: Implement insert/update, find, erase, rehash, and load-factor logic.
// Do not replace this class with std::unordered_map for the required implementation.

template <typename Key, typename Value>
class HashTable {
private:
    using Entry = std::pair<Key, Value>;
    std::vector<std::list<Entry>> buckets;
    std::size_t elementCount{0};

    std::size_t bucketIndex(const Key& key) const {
        return std::hash<Key>{}(key) % buckets.size();
    }

    void rehash(std::size_t newBucketCount) {
        // TODO
    }

public:
    explicit HashTable(std::size_t initialBuckets = 17)
        : buckets(initialBuckets) {}

    std::size_t size() const { return elementCount; }
    bool empty() const { return elementCount == 0; }

    double loadFactor() const {
        return buckets.empty() ? 0.0 :
            static_cast<double>(elementCount) / static_cast<double>(buckets.size());
    }

    void insert(const Key& key, const Value& value) {
        // TODO
    }

    Value* find(const Key& key) {
        // TODO
        return nullptr;
    }

    const Value* find(const Key& key) const {
        // TODO
        return nullptr;
    }

    bool erase(const Key& key) {
        // TODO
        return false;
    }
};
