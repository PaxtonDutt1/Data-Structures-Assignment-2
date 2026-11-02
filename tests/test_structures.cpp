#include "LinkedList.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "ArrayQueue.h"
#include <stdexcept>
#include "HashTable.h"

static void testLinkedList() {
    LinkedList<int> list;
    assert(list.empty());
    assert(!list.remove(1));          // remove from empty list

    list.pushFront(1);
    list.pushFront(2);
    list.pushFront(3);                // list is 3, 2, 1
    assert(list.size() == 3);
    assert(list.contains(2));
    assert(!list.contains(99));

    std::vector<int> seen;
    list.forEach([&seen](int value) { seen.push_back(value); });
    assert((seen == std::vector<int>{3, 2, 1}));

    assert(list.remove(2));           // middle
    assert(list.remove(3));           // head
    assert(list.remove(1));           // last remaining node
    assert(!list.remove(1));          // already gone
    assert(list.empty() && list.size() == 0);

    LinkedList<std::string> names;    // non-trivial type, checks cleanup
    for (int i = 0; i < 1000; ++i) {
        names.pushFront("student-" + std::to_string(i));
    }
    names.clear();
    assert(names.empty());
    names.pushFront("still usable after clear");
    assert(names.size() == 1);        // destructor frees this one
}

static void testArrayQueue() {
    // Empty-state handling
    ArrayQueue<int> queue(4);
    assert(queue.empty());
    bool threw = false;
    try { queue.dequeue(); } catch (const std::runtime_error&) { threw = true; }
    assert(threw);
    threw = false;
    try { queue.front(); } catch (const std::runtime_error&) { threw = true; }
    assert(threw);

    // Wrap-around: move the front forward, then fill past the array's end
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    assert(queue.dequeue() == 1);
    assert(queue.dequeue() == 2);
    queue.enqueue(4);
    queue.enqueue(5);
    queue.enqueue(6);                 // array is now 5, 6, 3, 4 with front at 3
    assert(queue.size() == 4);
    assert(queue.capacity() == 4);    // wrapped without growing
    assert(queue.front() == 3);

    // Growth while wrapped must preserve FIFO order
    queue.enqueue(7);
    assert(queue.capacity() == 8);
    std::vector<int> seen;
    queue.forEach([&seen](int value) { seen.push_back(value); });
    assert((seen == std::vector<int>{3, 4, 5, 6, 7}));
    for (int expected = 3; expected <= 7; ++expected) {
        assert(queue.dequeue() == expected);
    }
    assert(queue.empty());

    // Non-trivial type, large growth, zero starting capacity
    ArrayQueue<std::string> names(0);
    for (int i = 0; i < 1000; ++i) {
        names.enqueue("student-" + std::to_string(i));
    }
    for (int i = 0; i < 1000; ++i) {
        assert(names.dequeue() == "student-" + std::to_string(i));
    }
    assert(names.empty());
}

static void testHashTable() {
    // Basic operations, including replacing an existing value
    HashTable<std::string, int> table;
    assert(table.empty());
    assert(table.find("CPSC100") == nullptr);
    assert(!table.erase("CPSC100"));
    table.insert("CPSC100", 50);
    table.insert("MATH113", 40);
    table.insert("CPSC100", 55);
    assert(table.size() == 2);
    assert(*table.find("CPSC100") == 55);
    assert(table.erase("CPSC100"));
    assert(table.find("CPSC100") == nullptr);
    assert(!table.erase("CPSC100"));
    assert(table.size() == 1);

    // Collisions: 1 and 4 share a bucket when there are 3 buckets
    HashTable<int, std::string> colliding(3);
    colliding.insert(1, "one");
    colliding.insert(4, "four");
    assert(colliding.bucketCount() == 3);        // no rehash yet
    assert(*colliding.find(1) == "one");
    assert(*colliding.find(4) == "four");
    assert(colliding.erase(1));
    assert(colliding.find(1) == nullptr);
    assert(*colliding.find(4) == "four");

    // Forced rehash from a small bucket count keeps every entry
    HashTable<int, int> small(2);
    int* firstValue = nullptr;
    std::size_t rehashes = 0;
    for (int key = 0; key < 1000; ++key) {
        const std::size_t before = small.bucketCount();
        const double loadBefore = small.loadFactor();
        small.insert(key, key * 10);
        if (key == 0) {
            firstValue = small.find(0);
        }
        if (small.bucketCount() != before) {
            ++rehashes;
            std::cout << "Rehash: " << before << " -> " << small.bucketCount()
                      << " buckets, load " << loadBefore << " -> "
                      << small.loadFactor() << '\n';
        }
        assert(small.loadFactor() <= 0.75);
    }
    assert(rehashes > 0);
    assert(small.size() == 1000);
    for (int key = 0; key < 1000; ++key) {
        assert(small.find(key) != nullptr && *small.find(key) == key * 10);
    }
    assert(firstValue == small.find(0));   // pointer survived every rehash

    std::size_t visited = 0;
    small.forEach([&visited](int, int) { ++visited; });
    assert(visited == 1000);

    // The type combination RegistrationManager uses
    HashTable<std::string, ArrayQueue<int>> waitlists;
    waitlists.insert("CPSC100", ArrayQueue<int>());
    waitlists.find("CPSC100")->enqueue(200001);
    assert(waitlists.find("CPSC100")->front() == 200001);
}

int main() {
    testLinkedList();
    testArrayQueue();
    testHashTable();
    std::cout << "All tests passed.\n";
    return 0;
}
