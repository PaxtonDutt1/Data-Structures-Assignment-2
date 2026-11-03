#include "RegistrationManager.h"
#include <iostream>

RegistrationManager::RegistrationManager(DataRepository& repository)
    : repository(repository) {}

void RegistrationManager::initializeFromRepository() {
    registrations.clear();
    for (const RegistrationRecord& record : repository.getInitialRegistrations()) {
        registrations.pushFront(record);
    }
    std::cout << "Active registrations initialized: "
              << registrations.size() << '\n';
}

bool RegistrationManager::registerStudent(
    int studentId, const std::string& courseCode) {
    const Student* student = repository.getStudent(studentId);
    if (student == nullptr) {
        report("Registration failed: unknown student ID " + std::to_string(studentId) + ".");
        return false;
    }

    const Course* course = repository.getCourse(courseCode);
    if (course == nullptr) {
        report("Registration failed: unknown course code " + courseCode + ".");
        return false;
    }

    std::string reason;
    if (!canRegister(*student, *course, reason)) {
        report("Registration failed: " + reason);
        return false;
    }

    const int enrolled = enrollmentCount(courseCode);
        if (enrolled >= course->getCapacity()) {
        const std::string fullText = courseCode + " is full (" +
            std::to_string(enrolled) + "/" +
            std::to_string(course->getCapacity()) + ")";

        if (isWaitlisted(studentId, courseCode)) {
            report(fullText + ": student " + std::to_string(studentId) +
                   " is already on the waitlist.");
            return false;
        }

        // No undo record: undo covers completed registrations and drops only.
        if (waitlists.find(courseCode) == nullptr) {
            waitlists.insert(courseCode, ArrayQueue<WaitlistEntry>());
        }
        ArrayQueue<WaitlistEntry>* queue = waitlists.find(courseCode);
        queue->enqueue(WaitlistEntry{studentId, courseCode});
        report(fullText + ": student " + std::to_string(studentId) +
               " added to the waitlist at position " +
               std::to_string(queue->size()) + ".");
        return false;
    }

    registrations.pushFront(RegistrationRecord{studentId, courseCode});
    undoStack.push(RegistrationAction{ActionType::Register, studentId, courseCode});
    report("Registered student " + std::to_string(studentId) + " in " + courseCode + ".");
    return true;
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
    return registrations.contains(RegistrationRecord{studentId, courseCode});
}

int RegistrationManager::enrollmentCount(const std::string& courseCode) const {
    int count = 0;
    registrations.forEach([&count, &courseCode](const RegistrationRecord& record) {
        if (record.courseCode == courseCode) {
            ++count;
        }
    });
    return count;
}

bool RegistrationManager::isWaitlisted(
    int studentId, const std::string& courseCode) const {
    const ArrayQueue<WaitlistEntry>* queue = waitlists.find(courseCode);
    if (queue == nullptr) {
        return false;
    }
    bool found = false;
    queue->forEach([&found, studentId](const WaitlistEntry& entry) {
        if (entry.studentId == studentId) {
            found = true;
        }
    });
    return found;
}

std::size_t RegistrationManager::waitlistSize(const std::string& courseCode) const {
    const ArrayQueue<WaitlistEntry>* queue = waitlists.find(courseCode);
    return queue == nullptr ? 0 : queue->size();
}

std::size_t RegistrationManager::activeRegistrationCount() const {
    return registrations.size();
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
    for (const std::string& prerequisite : course.getPrerequisites()) {
        if (!student.hasCompleted(prerequisite)) {
            return false;
        }
    }
    return true;
}

// Checks whether a student may hold a seat, apart from seat availability.
// Waitlist processing will reuse this, so capacity is checked separately.
bool RegistrationManager::canRegister(
    const Student& student, const Course& course, std::string& reason) const {
    const std::string studentText = "student " + std::to_string(student.getId());

    if (!student.isActive()) {
        reason = studentText + " is not active.";
        return false;
    }
    if (isRegistered(student.getId(), course.getCode())) {
        reason = studentText + " is already registered in " + course.getCode() + ".";
        return false;
    }
    if (!prerequisitesSatisfied(student, course)) {
        reason = studentText + " is missing prerequisite(s) for " + course.getCode() + ":";
        for (const std::string& prerequisite : course.getPrerequisites()) {
            if (!student.hasCompleted(prerequisite)) {
                reason += " " + prerequisite;
            }
        }
        reason += ".";
        return false;
    }
    return true;
}

void RegistrationManager::report(const std::string& message) {
    lastMessage = message;
    std::cout << message << '\n';
}

const std::string& RegistrationManager::getLastMessage() const {
    return lastMessage;
}