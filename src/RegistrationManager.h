#pragma once
#include "ArrayQueue.h"
#include "ArrayStack.h"
#include "DataRepository.h"
#include "LinkedList.h"
#include "RegistrationRecord.h"
#include <string>

class RegistrationManager {
private:
    DataRepository& repository;

    // Active registrations are stored in a linked structure.
    LinkedList<RegistrationRecord> registrations;

    // One FIFO queue per course, created the first time a course needs it.
    HashTable<std::string, ArrayQueue<WaitlistEntry>> waitlists;

    // Stack used for undoing the most recent successful registration/drop.
    ArrayStack<RegistrationAction> undoStack;
    
    // The most recent result, kept so tests can check the reason given.
    std::string lastMessage;

public:
    explicit RegistrationManager(DataRepository& repository);

    void initializeFromRepository();

    bool registerStudent(int studentId, const std::string& courseCode);
    bool dropStudent(int studentId, const std::string& courseCode);
    bool undoLastAction();

    void processWaitlist(const std::string& courseCode);

    bool isRegistered(int studentId, const std::string& courseCode) const;
    int enrollmentCount(const std::string& courseCode) const;
    std::size_t activeRegistrationCount() const;
    const std::string& getLastMessage() const;
    bool isWaitlisted(int studentId, const std::string& courseCode) const;
    std::size_t waitlistSize(const std::string& courseCode) const;
    std::size_t undoCount() const;

    void printStudentSchedule(int studentId) const;
    void printCourseRoster(const std::string& courseCode) const;
    void printWaitlist(const std::string& courseCode) const;

    void saveRegistrations(const std::string& filename) const;

private:
    bool prerequisitesSatisfied(const Student& student, const Course& course) const;
    bool canRegister(const Student& student, const Course& course, std::string& reason) const;

    void report(const std::string& message);
};
