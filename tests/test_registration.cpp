#include "DataRepository.h"
#include "RegistrationManager.h"
#include <cassert>
#include <iostream>
#include <string>

static void testInitialState(const std::string& dataDirectory) {
    DataRepository repository;
    repository.loadAll(dataDirectory);
    RegistrationManager manager(repository);
    manager.initializeFromRepository();

    assert(manager.activeRegistrationCount() == 2222);
    assert(manager.isRegistered(200471, "MATH251"));
    assert(!manager.isRegistered(200471, "NOPE100"));
    assert(!manager.isRegistered(999999, "MATH251"));

    // ENGR360 intentionally starts over capacity in the provided data.
    assert(manager.enrollmentCount("ENGR360") == 35);
    assert(repository.getCourse("ENGR360")->getCapacity() == 20);
    assert(manager.enrollmentCount("NOPE100") == 0);
}

int main(int argc, char* argv[]) {
    const std::string dataDirectory = (argc > 1) ? argv[1] : "../data";
    testInitialState(dataDirectory);
    std::cout << "All registration tests passed.\n";
    return 0;
}