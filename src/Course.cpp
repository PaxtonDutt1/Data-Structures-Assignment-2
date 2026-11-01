#include "Course.h"
#include <utility>

Course::Course(std::string code, std::string section, std::string title,
               int capacity, int credits)
    : code(std::move(code)), section(std::move(section)),
      title(std::move(title)), capacity(capacity), credits(credits) {}

const std::string& Course::getCode() const { return code; }
const std::string& Course::getSection() const { return section; }
const std::string& Course::getTitle() const { return title; }
int Course::getCapacity() const { return capacity; }
int Course::getCredits() const { return credits; }

void Course::addPrerequisite(const std::string& prerequisiteCode) {
    prerequisites.push_back(prerequisiteCode);
}

const std::vector<std::string>& Course::getPrerequisites() const {
    return prerequisites;
}
