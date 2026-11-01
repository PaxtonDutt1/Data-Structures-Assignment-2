#include "Student.h"
#include <utility>

Student::Student(int id, std::string name, std::string level,
                 std::string major, int earnedCredits, bool active)
    : id(id), name(std::move(name)), level(std::move(level)),
      major(std::move(major)), earnedCredits(earnedCredits), active(active) {}

int Student::getId() const { return id; }
const std::string& Student::getName() const { return name; }
const std::string& Student::getLevel() const { return level; }
const std::string& Student::getMajor() const { return major; }
int Student::getEarnedCredits() const { return earnedCredits; }
bool Student::isActive() const { return active; }

void Student::addCompletedCourse(const std::string& courseCode) {
    completedCourses.insert(courseCode);
}

bool Student::hasCompleted(const std::string& courseCode) const {
    return completedCourses.find(courseCode) != completedCourses.end();
}
