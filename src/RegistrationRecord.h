#pragma once
#include <string>

struct RegistrationRecord {
    int studentId{};
    std::string courseCode;

    bool operator==(const RegistrationRecord& other) const {
        return studentId == other.studentId && courseCode == other.courseCode;
    }
};

struct WaitlistEntry {
    int studentId{};
    std::string courseCode;
};

enum class ActionType {
    Register,
    Drop
};

struct RegistrationAction {
    ActionType type;
    int studentId{};
    std::string courseCode;
};
