#pragma once
#include <string>
#include <vector>

class Course {
private:
    std::string code;
    std::string section;
    std::string title;
    int capacity{};
    int credits{};
    std::vector<std::string> prerequisites;

public:
    Course() = default;
    Course(std::string code, std::string section, std::string title,
           int capacity, int credits);

    const std::string& getCode() const;
    const std::string& getSection() const;
    const std::string& getTitle() const;
    int getCapacity() const;
    int getCredits() const;

    void addPrerequisite(const std::string& prerequisiteCode);
    const std::vector<std::string>& getPrerequisites() const;
};
