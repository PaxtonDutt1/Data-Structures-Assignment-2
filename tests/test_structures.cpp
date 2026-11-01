#include "LinkedList.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

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

int main() {
    testLinkedList();
    std::cout << "All tests passed.\n";
    return 0;
}
