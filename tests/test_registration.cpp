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

// A freshly loaded system for each test, so tests cannot affect each other.
struct TestSystem {
    DataRepository repository;
    RegistrationManager manager;

    explicit TestSystem(const std::string& dataDirectory) : manager(repository) {
        repository.loadAll(dataDirectory);
        manager.initializeFromRepository();
    }
};

static bool lastMessageHas(const RegistrationManager& manager, const std::string& text) {
    return manager.getLastMessage().find(text) != std::string::npos;
}

static void testRegistrationRules(const std::string& dataDirectory) {
    TestSystem system(dataDirectory);
    RegistrationManager& manager = system.manager;

    assert(!manager.registerStudent(999999, "CPSC100"));
    assert(lastMessageHas(manager, "unknown student"));

    assert(!manager.registerStudent(200001, "NOPE100"));
    assert(lastMessageHas(manager, "unknown course"));

    assert(!manager.registerStudent(200028, "CPSC100"));      // inactive
    assert(lastMessageHas(manager, "not active"));

    assert(!manager.registerStudent(200120, "MATH113"));      // already registered
    assert(lastMessageHas(manager, "already registered"));

    assert(!manager.registerStudent(200001, "MATH204"));      // needs CPSC100
    assert(lastMessageHas(manager, "missing prerequisite"));
    assert(lastMessageHas(manager, "CPSC100"));

    assert(!manager.registerStudent(200001, "PHYS165"));      // 30/30
    assert(lastMessageHas(manager, "full"));

    // No failed attempt may change the registration list.
    assert(manager.activeRegistrationCount() == 2222);

    const int before = manager.enrollmentCount("MATH204");
    assert(manager.registerStudent(200003, "MATH204"));       // has CPSC100
    assert(manager.isRegistered(200003, "MATH204"));
    assert(manager.enrollmentCount("MATH204") == before + 1);
    assert(manager.activeRegistrationCount() == 2223);

    assert(!manager.registerStudent(200003, "MATH204"));      // now a duplicate
    assert(lastMessageHas(manager, "already registered"));
}

static void testWaitlist(const std::string& dataDirectory) {
    TestSystem system(dataDirectory);
    RegistrationManager& manager = system.manager;

    // PHYS165 starts full at 30/30, with an empty waitlist.
    assert(manager.waitlistSize("PHYS165") == 0);
    assert(!manager.isWaitlisted(200001, "PHYS165"));

    assert(!manager.registerStudent(200001, "PHYS165"));
    assert(lastMessageHas(manager, "position 1"));
    assert(!manager.registerStudent(200002, "PHYS165"));
    assert(lastMessageHas(manager, "position 2"));
    assert(manager.isWaitlisted(200001, "PHYS165"));
    assert(manager.isWaitlisted(200002, "PHYS165"));

    // Duplicate waitlist request
    assert(!manager.registerStudent(200001, "PHYS165"));
    assert(lastMessageHas(manager, "already on the waitlist"));
    assert(manager.waitlistSize("PHYS165") == 2);

    // Waitlisting does not take a seat.
    assert(!manager.isRegistered(200001, "PHYS165"));
    assert(manager.enrollmentCount("PHYS165") == 30);
    assert(manager.activeRegistrationCount() == 2222);

    // Ineligible students cannot join a waitlist.
    assert(!manager.registerStudent(200028, "PHYS165"));      // inactive
    assert(lastMessageHas(manager, "not active"));
    assert(!manager.registerStudent(200001, "INFO503"));      // needs PHYS165
    assert(lastMessageHas(manager, "missing prerequisite"));
    assert(manager.waitlistSize("PHYS165") == 2);
    assert(manager.waitlistSize("INFO503") == 0);

    // An over-capacity course also waitlists, and each course has its own queue.
    assert(!manager.registerStudent(200001, "ENGR360"));
    assert(manager.enrollmentCount("ENGR360") == 35);
    assert(manager.waitlistSize("ENGR360") == 1);
    assert(manager.waitlistSize("PHYS165") == 2);
}

int main(int argc, char* argv[]) {
    const std::string dataDirectory = (argc > 1) ? argv[1] : "../data";
    testInitialState(dataDirectory);
    testRegistrationRules(dataDirectory);
    testWaitlist(dataDirectory);
    std::cout << "All registration tests passed.\n";
    return 0;
}