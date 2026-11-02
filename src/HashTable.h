#pragma once
#include <cstddef>
#include <functional>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

// Separate-chaining hash table.
// Each bucket holds a chain of entries whose keys hash to that bucket.
// The table grows to (2n + 1) buckets when the load factor would exceed 0.75.

template <typename Key, typename Value>
class HashTable {
private:
    using Entry = std::pair<Key, Value>;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    std::vector<std::list<Entry>> buckets;
    std::size_t elementCount{0};

    std::size_t bucketIndex(const Key& key) const {
        return std::hash<Key>{}(key) % buckets.size();
    }

    // Moves every entry into a new bucket array. splice relinks the existing
    // list nodes without copying, so Value pointers from find() remain valid.
    void rehash(std::size_t newBucketCount) {
        std::vector<std::list<Entry>> newBuckets(newBucketCount);
        for (std::list<Entry>& bucket : buckets) {
            while (!bucket.empty()) {
                std::size_t index =
                    std::hash<Key>{}(bucket.front().first) % newBucketCount;
                newBuckets[index].splice(newBuckets[index].begin(), bucket,
                                         bucket.begin());
            }
        }
        buckets.swap(newBuckets);
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

    std::size_t bucketCount() const { return buckets.size(); }

    // Inserts a new entry, or replaces the value if the key already exists.
    void insert(const Key& key, const Value& value) {
        if (Value* existing = find(key)) {
            *existing = value;
            return;
        }
        if (buckets.empty()) {
            buckets.resize(17);
        }
        const double newLoad = static_cast<double>(elementCount + 1) /
                               static_cast<double>(buckets.size());
        if (newLoad > MAX_LOAD_FACTOR) {
            rehash(buckets.size() * 2 + 1);
        }
        buckets[bucketIndex(key)].emplace_back(key, value);
        ++elementCount;
    }

    Value* find(const Key& key) {
        if (buckets.empty()) {
            return nullptr;
        }
        for (Entry& entry : buckets[bucketIndex(key)]) {
            if (entry.first == key) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    const Value* find(const Key& key) const {
        if (buckets.empty()) {
            return nullptr;
        }
        for (const Entry& entry : buckets[bucketIndex(key)]) {
            if (entry.first == key) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    bool erase(const Key& key) {
        if (buckets.empty()) {
            return false;
        }
        std::list<Entry>& bucket = buckets[bucketIndex(key)];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                --elementCount;
                return true;
            }
        }
        return false;
    }

    // Calls visit(key, value) on each entry, in no particular order.
    template <typename Visitor>
    void forEach(Visitor visit) const {
        for (const std::list<Entry>& bucket : buckets) {
            for (const Entry& entry : bucket) {
                visit(entry.first, entry.second);
            }
        }
    }
};