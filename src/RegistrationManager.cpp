#include "RegistrationManager.h"
#include <iostream>

RegistrationManager::RegistrationManager(DataRepository& repository)
    : repository(repository) {}

void RegistrationManager::initializeFromRepository() {
    // TODO: Populate registrations from repository.getInitialRegistrations().
}

bool RegistrationManager::registerStudent(
    int studentId, const std::string& courseCode) {
    // TODO:
    // Validate student/course.
    // Check active status, duplicates, prerequisites, and capacity.
    // If full, place the student in the FIFO waitlist.
    // If successful, add to registrations and push undo information.
    return false;
}

bool RegistrationManager::dropStudent(
    int studentId, const std::string& courseCode) {
    // TODO:
    // Remove registration.
    // Push undo information.
    // Then attempt to move the first eligible waitlisted student into the course.
    return false;
}

bool RegistrationManager::undoLastAction() {
    // TODO:
    // Reverse the most recent successful REGISTER or DROP operation.
    // Avoid creating another undo record while performing the reversal.
    return false;
}

void RegistrationManager::processWaitlist(const std::string& courseCode) {
    // TODO:
    // Process entries in FIFO order until the course is full
    // or no eligible entries remain.
}

bool RegistrationManager::isRegistered(
    int studentId, const std::string& courseCode) const {
    // TODO
    return false;
}

int RegistrationManager::enrollmentCount(const std::string& courseCode) const {
    // TODO
    return 0;
}

void RegistrationManager::printStudentSchedule(int studentId) const {
    // TODO
}

void RegistrationManager::printCourseRoster(const std::string& courseCode) const {
    // TODO
}

void RegistrationManager::printWaitlist(const std::string& courseCode) const {
    // TODO
}

void RegistrationManager::saveRegistrations(const std::string& filename) const {
    // TODO: Write active registrations in CSV format.
}

bool RegistrationManager::prerequisitesSatisfied(
    const Student& student, const Course& course) const {
    // TODO
    return false;
}

bool RegistrationManager::canRegister(
    const Student& student, const Course& course, std::string& reason) const {
    // TODO:
    // centralize validation and set a useful reason on failure
    return false;
}
