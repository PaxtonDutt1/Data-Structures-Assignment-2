#include "DataRepository.h"
#include <stdexcept>

void DataRepository::loadAll(const std::string& dataDirectory) {
    // TODO:
    // Load students.csv, courses.csv, prerequisites.csv,
    // completed_courses.csv, and initial_registrations.csv.
    //
    // Validate malformed rows and report useful errors.
    throw std::runtime_error("TODO: DataRepository::loadAll is not implemented.");
}

Student* DataRepository::getStudent(int studentId) {
    return students.find(studentId);
}

Course* DataRepository::getCourse(const std::string& courseCode) {
    return courses.find(courseCode);
}

const Student* DataRepository::getStudent(int studentId) const {
    return students.find(studentId);
}

const Course* DataRepository::getCourse(const std::string& courseCode) const {
    return courses.find(courseCode);
}

const std::vector<RegistrationRecord>& DataRepository::getInitialRegistrations() const {
    return initialRegistrations;
}

void DataRepository::loadStudents(const std::string&) { /* TODO */ }
void DataRepository::loadCourses(const std::string&) { /* TODO */ }
void DataRepository::loadPrerequisites(const std::string&) { /* TODO */ }
void DataRepository::loadCompletedCourses(const std::string&) { /* TODO */ }
void DataRepository::loadRegistrations(const std::string&) { /* TODO */ }
