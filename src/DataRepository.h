#pragma once
#include "Course.h"
#include "HashTable.h"
#include "RegistrationRecord.h"
#include "Student.h"
#include <string>
#include <vector>

class DataRepository {
private:
    HashTable<int, Student> students;
    HashTable<std::string, Course> courses;
    std::vector<RegistrationRecord> initialRegistrations;

public:
    void loadAll(const std::string& dataDirectory);

    Student* getStudent(int studentId);
    Course* getCourse(const std::string& courseCode);

    const Student* getStudent(int studentId) const;
    const Course* getCourse(const std::string& courseCode) const;

    const std::vector<RegistrationRecord>& getInitialRegistrations() const;

private:
    void loadStudents(const std::string& filename);
    void loadCourses(const std::string& filename);
    void loadPrerequisites(const std::string& filename);
    void loadCompletedCourses(const std::string& filename);
    void loadRegistrations(const std::string& filename);
};
