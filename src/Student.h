#pragma once
#include <string>
#include <unordered_set>

class Student {
private:
    int id{};
    std::string name;
    std::string level;
    std::string major;
    int earnedCredits{};
    bool active{true};
    std::unordered_set<std::string> completedCourses;

public:
    Student() = default;
    Student(int id, std::string name, std::string level,
            std::string major, int earnedCredits, bool active);

    int getId() const;
    const std::string& getName() const;
    const std::string& getLevel() const;
    const std::string& getMajor() const;
    int getEarnedCredits() const;
    bool isActive() const;

    void addCompletedCourse(const std::string& courseCode);
    bool hasCompleted(const std::string& courseCode) const;
};
