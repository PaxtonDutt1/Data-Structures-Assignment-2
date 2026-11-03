#include "DataRepository.h"
#include "RegistrationManager.h"
#include <cassert>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

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

static void testDropAndPromotion(const std::string& dataDirectory) {
    TestSystem system(dataDirectory);
    RegistrationManager& manager = system.manager;

    // Failed drops change nothing.
    assert(!manager.dropStudent(999999, "PHYS165"));
    assert(lastMessageHas(manager, "unknown student"));
    assert(!manager.dropStudent(200921, "NOPE100"));
    assert(lastMessageHas(manager, "unknown course"));
    assert(!manager.dropStudent(200001, "PHYS165"));
    assert(lastMessageHas(manager, "not registered"));
    assert(manager.activeRegistrationCount() == 2222);

    // Students 200001, 200002, and 200003 wait in that order for PHYS165 (30/30).
    manager.registerStudent(200001, "PHYS165");
    manager.registerStudent(200002, "PHYS165");
    manager.registerStudent(200003, "PHYS165");
    assert(manager.waitlistSize("PHYS165") == 3);

    // A drop promotes the first student in line, and only that student.
    assert(manager.dropStudent(200921, "PHYS165"));
    assert(!manager.isRegistered(200921, "PHYS165"));
    assert(manager.isRegistered(200001, "PHYS165"));
    assert(!manager.isRegistered(200002, "PHYS165"));
    assert(manager.enrollmentCount("PHYS165") == 30);
    assert(manager.waitlistSize("PHYS165") == 2);

    // A waitlisted student who becomes ineligible is removed and skipped.
    system.repository.getStudent(200002)->setActive(false);
    assert(manager.dropStudent(200620, "PHYS165"));
    assert(!manager.isRegistered(200002, "PHYS165"));
    assert(!manager.isWaitlisted(200002, "PHYS165"));
    assert(manager.isRegistered(200003, "PHYS165"));
    assert(manager.enrollmentCount("PHYS165") == 30);
    assert(manager.waitlistSize("PHYS165") == 0);

    // With nobody waiting, a drop simply leaves an open seat.
    assert(manager.dropStudent(201171, "PHYS165"));
    assert(manager.enrollmentCount("PHYS165") == 29);

    // An over-capacity course still has no seat after one drop.
    manager.registerStudent(200001, "ENGR360");
    assert(manager.dropStudent(200026, "ENGR360"));
    assert(manager.enrollmentCount("ENGR360") == 34);
    assert(!manager.isRegistered(200001, "ENGR360"));
    assert(manager.isWaitlisted(200001, "ENGR360"));

    // Processing a waitlist directly handles bad input safely.
    manager.processWaitlist("NOPE100");
    assert(lastMessageHas(manager, "unknown course"));
    manager.processWaitlist("CPSC100");
    assert(lastMessageHas(manager, "is empty"));
}

static void testUndo(const std::string& dataDirectory) {
    {
        TestSystem system(dataDirectory);
        RegistrationManager& manager = system.manager;

        // Empty undo stack
        assert(!manager.undoLastAction());
        assert(lastMessageHas(manager, "Nothing to undo"));

        // Failed operations and waitlist requests create no undo record.
        manager.registerStudent(999999, "CPSC100");
        manager.dropStudent(200001, "PHYS165");
        manager.registerStudent(200001, "PHYS165");           // waitlisted
        assert(manager.undoCount() == 0);
        manager.processWaitlist("PHYS165");
        assert(manager.undoCount() == 0);
    }
    {
        TestSystem system(dataDirectory);
        RegistrationManager& manager = system.manager;

        // LIFO order, and undo itself creates no new record.
        assert(manager.registerStudent(200003, "MATH204"));
        assert(manager.dropStudent(200921, "PHYS165"));
        assert(manager.undoCount() == 2);

        assert(manager.undoLastAction());                     // reverses the drop
        assert(manager.isRegistered(200921, "PHYS165"));
        assert(manager.isRegistered(200003, "MATH204"));
        assert(manager.undoCount() == 1);

        assert(manager.undoLastAction());                     // reverses the register
        assert(!manager.isRegistered(200003, "MATH204"));
        assert(manager.undoCount() == 0);
        assert(manager.activeRegistrationCount() == 2222);

        assert(!manager.undoLastAction());
        assert(lastMessageHas(manager, "Nothing to undo"));
    }
    {
        TestSystem system(dataDirectory);
        RegistrationManager& manager = system.manager;

        // Undoing a drop fails if a promoted student took the seat.
        manager.registerStudent(200001, "PHYS165");           // waitlisted
        assert(manager.dropStudent(200921, "PHYS165"));
        assert(manager.isRegistered(200001, "PHYS165"));      // promoted
        assert(manager.undoCount() == 1);                     // promotion adds none
        assert(!manager.undoLastAction());
        assert(lastMessageHas(manager, "no open seat"));
        assert(!manager.isRegistered(200921, "PHYS165"));
        assert(manager.enrollmentCount("PHYS165") == 30);
        assert(manager.undoCount() == 0);
    }
    {
        TestSystem system(dataDirectory);
        RegistrationManager& manager = system.manager;

        // Undoing a register opens a seat, which promotes the next student.
        assert(manager.dropStudent(200921, "PHYS165"));       // 29/30
        assert(manager.registerStudent(200003, "PHYS165"));   // 30/30
        manager.registerStudent(200001, "PHYS165");           // waitlisted
        assert(manager.undoCount() == 2);
        assert(manager.undoLastAction());
        assert(!manager.isRegistered(200003, "PHYS165"));
        assert(manager.isRegistered(200001, "PHYS165"));
        assert(manager.enrollmentCount("PHYS165") == 30);
        assert(manager.undoCount() == 1);
    }
    {
        TestSystem system(dataDirectory);
        RegistrationManager& manager = system.manager;

        // Over-capacity course: restoring would keep it above capacity.
        assert(manager.dropStudent(200026, "ENGR360"));       // 34/20
        assert(!manager.undoLastAction());
        assert(!manager.isRegistered(200026, "ENGR360"));

        // A restored seat is not re-checked against prerequisites, because
        // initial registrations may not meet them.
        assert(manager.dropStudent(200471, "ENGR498"));
        assert(manager.undoLastAction());
        assert(manager.isRegistered(200471, "ENGR498"));
    }
}

static void testSaving(const std::string& dataDirectory) {
    TestSystem system(dataDirectory);
    RegistrationManager& manager = system.manager;

    manager.saveRegistrations("no_such_folder/registrations.csv");
    assert(lastMessageHas(manager, "Save failed"));

    assert(manager.registerStudent(200003, "MATH204"));
    const std::string filename = "test_saved_registrations.csv";
    manager.saveRegistrations(filename);
    assert(lastMessageHas(manager, "Saved 2223"));

    std::ifstream file(filename);
    assert(file);
    std::string line;
    std::getline(file, line);
    assert(line == "student_id,course_code");
    int rows = 0;
    bool foundNewRegistration = false;
    while (std::getline(file, line)) {
        ++rows;
        foundNewRegistration = foundNewRegistration || line == "200003,MATH204";
    }
    assert(rows == 2223);
    assert(foundNewRegistration);
    file.close();
    std::remove(filename.c_str());

    // Display functions must handle unknown IDs and empty results safely.
    manager.printStudentSchedule(999999);
    manager.printCourseRoster("NOPE100");
    manager.printWaitlist("NOPE100");
    manager.printWaitlist("CPSC100");
    manager.printStatistics();
}

int main(int argc, char* argv[]) {
    const std::string dataDirectory = (argc > 1) ? argv[1] : "../data";
    testInitialState(dataDirectory);
    testRegistrationRules(dataDirectory);
    testWaitlist(dataDirectory);
    testDropAndPromotion(dataDirectory);
    testUndo(dataDirectory);
    testSaving(dataDirectory);
    std::cout << "All registration tests passed.\n";
    return 0;
}