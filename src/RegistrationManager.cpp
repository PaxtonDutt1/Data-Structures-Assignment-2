#include "RegistrationManager.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>

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
    if (repository.getStudent(studentId) == nullptr) {
        report("Drop failed: unknown student ID " + std::to_string(studentId) + ".");
        return false;
    }
    if (repository.getCourse(courseCode) == nullptr) {
        report("Drop failed: unknown course code " + courseCode + ".");
        return false;
    }
    if (!registrations.remove(RegistrationRecord{studentId, courseCode})) {
        report("Drop failed: student " + std::to_string(studentId) +
               " is not registered in " + courseCode + ".");
        return false;
    }

    undoStack.push(RegistrationAction{ActionType::Drop, studentId, courseCode});
    report("Dropped student " + std::to_string(studentId) + " from " + courseCode + ".");
    processWaitlist(courseCode);
    return true;
}

// Changes the registration list directly rather than calling registerStudent
// or dropStudent, so that undoing an action never creates another undo record.
// The action is consumed even if it can no longer be reversed.
bool RegistrationManager::undoLastAction() {
    if (undoStack.empty()) {
        report("Nothing to undo.");
        return false;
    }

    const RegistrationAction action = undoStack.pop();
    const std::string studentText = "student " + std::to_string(action.studentId);

    if (action.type == ActionType::Register) {
        if (!registrations.remove(RegistrationRecord{action.studentId, action.courseCode})) {
            report("Cannot undo registration: " + studentText + " is no longer registered in " +
                   action.courseCode + ".");
            return false;
        }
        report("Undid registration of " + studentText + " in " + action.courseCode + ".");
        if (waitlistSize(action.courseCode) > 0) {
            processWaitlist(action.courseCode);
        }
        return true;
    }

    // Undoing a drop checks only seat availability and duplicates. Prerequisites
    // are not re-checked because some initial registrations do not meet them.
    if (isRegistered(action.studentId, action.courseCode)) {
        report("Cannot undo drop: " + studentText + " is already registered in " +
               action.courseCode + ".");
        return false;
    }
    const Course* course = repository.getCourse(action.courseCode);
    const int enrolled = enrollmentCount(action.courseCode);
    if (course == nullptr || enrolled >= course->getCapacity()) {
        report("Cannot undo drop of " + studentText + ": " + action.courseCode +
               " has no open seat.");
        return false;
    }
    registrations.pushFront(RegistrationRecord{action.studentId, action.courseCode});
    report("Undid drop: " + studentText + " is registered in " + action.courseCode + " again.");
    return true;
}

void RegistrationManager::processWaitlist(const std::string& courseCode) {
    const Course* course = repository.getCourse(courseCode);
    if (course == nullptr) {
        report("Cannot process waitlist: unknown course code " + courseCode + ".");
        return;
    }

    ArrayQueue<WaitlistEntry>* queue = waitlists.find(courseCode);
    if (queue == nullptr || queue->empty()) {
        report("The waitlist for " + courseCode + " is empty.");
        return;
    }

    int enrolled = enrollmentCount(courseCode);
    while (!queue->empty() && enrolled < course->getCapacity()) {
        const WaitlistEntry entry = queue->dequeue();
        const std::string studentText = "student " + std::to_string(entry.studentId);

        const Student* student = repository.getStudent(entry.studentId);
        std::string reason;
        if (student == nullptr) {
            report("Removed unknown " + studentText + " from the " + courseCode + " waitlist.");
        } else if (!canRegister(*student, *course, reason)) {
            report("Removed from the " + courseCode + " waitlist: " + reason);
        } else {
            // No undo record: the user's own register or drop is what undo reverses.
            registrations.pushFront(RegistrationRecord{entry.studentId, courseCode});
            ++enrolled;
            report("Promoted " + studentText + " from the waitlist into " + courseCode + ".");
        }
    }

    if (!queue->empty()) {
        report(courseCode + " has no open seat (" + std::to_string(enrolled) + "/" +
               std::to_string(course->getCapacity()) + "); " +
               std::to_string(queue->size()) + " student(s) still waiting.");
    }
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

std::size_t RegistrationManager::undoCount() const {
    return undoStack.size();
}

std::size_t RegistrationManager::activeRegistrationCount() const {
    return registrations.size();
}

void RegistrationManager::printStudentSchedule(int studentId) const {
    const Student* student = repository.getStudent(studentId);
    if (student == nullptr) {
        std::cout << "Unknown student ID " << studentId << ".\n";
        return;
    }

    std::vector<std::string> courseCodes;
    registrations.forEach([&courseCodes, studentId](const RegistrationRecord& record) {
        if (record.studentId == studentId) {
            courseCodes.push_back(record.courseCode);
        }
    });
    std::sort(courseCodes.begin(), courseCodes.end());

    std::cout << "Schedule for " << student->getName() << " (" << studentId << ", "
              << student->getLevel() << ", " << student->getMajor() << ", "
              << (student->isActive() ? "active" : "inactive") << ")\n";
    if (courseCodes.empty()) {
        std::cout << "  No current registrations.\n";
        return;
    }

    int totalCredits = 0;
    for (const std::string& code : courseCodes) {
        const Course* course = repository.getCourse(code);
        if (course != nullptr) {
            std::cout << "  " << code << " section " << course->getSection() << ": "
                      << course->getTitle() << " (" << course->getCredits() << " credits)\n";
            totalCredits += course->getCredits();
        }
    }
    std::cout << "  Total: " << courseCodes.size() << " course(s), "
              << totalCredits << " credits\n";
}

void RegistrationManager::printCourseRoster(const std::string& courseCode) const {
    const Course* course = repository.getCourse(courseCode);
    if (course == nullptr) {
        std::cout << "Unknown course code " << courseCode << ".\n";
        return;
    }

    std::vector<int> studentIds;
    registrations.forEach([&studentIds, &courseCode](const RegistrationRecord& record) {
        if (record.courseCode == courseCode) {
            studentIds.push_back(record.studentId);
        }
    });
    std::sort(studentIds.begin(), studentIds.end());

    std::cout << "Roster for " << courseCode << " section " << course->getSection() << ": "
              << course->getTitle() << "\n  Enrollment: " << studentIds.size() << "/"
              << course->getCapacity() << "\n";
    for (int id : studentIds) {
        const Student* student = repository.getStudent(id);
        std::cout << "  " << id << " " << (student != nullptr ? student->getName() : "(unknown)")
                  << '\n';
    }
}

void RegistrationManager::printWaitlist(const std::string& courseCode) const {
    const Course* course = repository.getCourse(courseCode);
    if (course == nullptr) {
        std::cout << "Unknown course code " << courseCode << ".\n";
        return;
    }

    std::cout << "Waitlist for " << courseCode << " (enrollment "
              << enrollmentCount(courseCode) << "/" << course->getCapacity() << ")\n";
    const ArrayQueue<WaitlistEntry>* queue = waitlists.find(courseCode);
    if (queue == nullptr || queue->empty()) {
        std::cout << "  The waitlist is empty.\n";
        return;
    }

    int position = 0;
    queue->forEach([this, &position](const WaitlistEntry& entry) {
        const Student* student = repository.getStudent(entry.studentId);
        std::cout << "  " << ++position << ". " << entry.studentId << " "
                  << (student != nullptr ? student->getName() : "(unknown)") << '\n';
    });
}

void RegistrationManager::printStatistics() const {
    std::size_t nonEmptyWaitlists = 0;
    std::size_t waitlistedStudents = 0;
    waitlists.forEach([&nonEmptyWaitlists, &waitlistedStudents](
                          const std::string&, const ArrayQueue<WaitlistEntry>& queue) {
        if (!queue.empty()) {
            ++nonEmptyWaitlists;
            waitlistedStudents += queue.size();
        }
    });

    std::cout << "System statistics\n"
              << "  Students: " << repository.studentCount() << '\n'
              << "  Courses: " << repository.courseCount() << '\n'
              << "  Active registrations: " << registrations.size() << '\n'
              << "  Non-empty waitlists: " << nonEmptyWaitlists << '\n'
              << "  Waitlisted students: " << waitlistedStudents << '\n'
              << "  Actions available to undo: " << undoStack.size() << '\n'
              << "  Student table load factor: " << repository.studentLoadFactor() << '\n'
              << "  Course table load factor: " << repository.courseLoadFactor() << '\n';
}

void RegistrationManager::saveRegistrations(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        report("Save failed: could not open " + filename + " for writing.");
        return;
    }

    file << "student_id,course_code\n";
    registrations.forEach([&file](const RegistrationRecord& record) {
        file << record.studentId << ',' << record.courseCode << '\n';
    });
    file.flush();

    if (!file) {
        report("Save failed: an error occurred while writing " + filename + ".");
        return;
    }
    report("Saved " + std::to_string(registrations.size()) + " registrations to " + filename + ".");
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