#include "DataRepository.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }
    // getline drops an empty final field, as in "a,b,"
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }
    return fields;
}

// Accepts only a complete non-negative integer, so "12x" and "-5" are rejected.
bool parseNonNegativeInt(const std::string& text, int& result) {
    if (text.empty()) {
        return false;
    }
    try {
        std::size_t used = 0;
        const int value = std::stoi(text, &used);
        if (used != text.size() || value < 0) {
            return false;
        }
        result = value;
        return true;
    } catch (const std::exception&) {   // not a number, or too large for int
        return false;
    }
}

bool isValidGrade(const std::string& grade) {
    static const std::vector<std::string> grades = {
        "A", "A-", "B+", "B", "B-", "C+", "C", "C-", "D", "F"};
    for (const std::string& valid : grades) {
        if (grade == valid) {
            return true;
        }
    }
    return false;
}

// Reads a CSV file after its header row. processRow returns an empty string
// for a good row, or the reason it was rejected. Bad rows are reported and
// skipped. A file that cannot be opened is a fatal error.
template <typename RowHandler>
void readCsv(const std::string& filename, std::size_t fieldCount,
             RowHandler processRow) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open " + filename);
    }

    std::string line;
    std::getline(file, line);   // header row

    int lineNumber = 1;
    int loaded = 0;
    int rejected = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue;
        }

        const std::vector<std::string> fields = splitCsvLine(line);
        std::string problem;
        if (fields.size() != fieldCount) {
            problem = "expected " + std::to_string(fieldCount) + " fields, found " +
                      std::to_string(fields.size());
        } else {
            problem = processRow(fields);
        }

        if (problem.empty()) {
            ++loaded;
        } else {
            ++rejected;
            std::cerr << "  " << filename << " line " << lineNumber
                      << " rejected: " << problem << '\n';
        }
    }
    std::cout << filename << ": " << loaded << " loaded, "
              << rejected << " rejected\n";
}

}  // namespace

void DataRepository::loadAll(const std::string& dataDirectory) {
    const std::string prefix = dataDirectory + "/";

    // Order matters: later files refer to students and courses.
    loadStudents(prefix + "students.csv");
    loadCourses(prefix + "courses.csv");
    loadPrerequisites(prefix + "prerequisites.csv");
    loadCompletedCourses(prefix + "completed_courses.csv");
    loadRegistrations(prefix + "initial_registrations.csv");
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

void DataRepository::loadStudents(const std::string& filename) {
    readCsv(filename, 6, [this](const std::vector<std::string>& fields) -> std::string {
        int id = 0;
        int earnedCredits = 0;
        if (!parseNonNegativeInt(fields[0], id)) {
            return "invalid student ID '" + fields[0] + "'";
        }
        if (fields[1].empty()) {
            return "missing student name";
        }
        if (!parseNonNegativeInt(fields[4], earnedCredits)) {
            return "invalid earned credits '" + fields[4] + "'";
        }
        if (fields[5] != "Y" && fields[5] != "N") {
            return "active status must be Y or N";
        }
        if (students.find(id) != nullptr) {
            return "duplicate student ID " + fields[0];
        }
        students.insert(id, Student(id, fields[1], fields[2], fields[3],
                                    earnedCredits, fields[5] == "Y"));
        return "";
    });
}

void DataRepository::loadCourses(const std::string& filename) {
    readCsv(filename, 5, [this](const std::vector<std::string>& fields) -> std::string {
        int capacity = 0;
        int credits = 0;
        if (fields[0].empty()) {
            return "missing course code";
        }
        if (!parseNonNegativeInt(fields[3], capacity) || capacity == 0) {
            return "invalid capacity '" + fields[3] + "'";
        }
        if (!parseNonNegativeInt(fields[4], credits) || credits == 0) {
            return "invalid credits '" + fields[4] + "'";
        }
        if (courses.find(fields[0]) != nullptr) {
            return "duplicate course code " + fields[0];
        }
        courses.insert(fields[0], Course(fields[0], fields[1], fields[2],
                                         capacity, credits));
        return "";
    });
}

void DataRepository::loadPrerequisites(const std::string& filename) {
    readCsv(filename, 2, [this](const std::vector<std::string>& fields) -> std::string {
        Course* course = courses.find(fields[0]);
        if (course == nullptr) {
            return "unknown course " + fields[0];
        }
        if (courses.find(fields[1]) == nullptr) {
            return "unknown prerequisite " + fields[1];
        }
        if (fields[0] == fields[1]) {
            return "course cannot be its own prerequisite";
        }
        for (const std::string& existing : course->getPrerequisites()) {
            if (existing == fields[1]) {
                return "duplicate prerequisite " + fields[1];
            }
        }
        course->addPrerequisite(fields[1]);
        return "";
    });
}

void DataRepository::loadCompletedCourses(const std::string& filename) {
    readCsv(filename, 3, [this](const std::vector<std::string>& fields) -> std::string {
        int studentId = 0;
        if (!parseNonNegativeInt(fields[0], studentId)) {
            return "invalid student ID '" + fields[0] + "'";
        }
        Student* student = students.find(studentId);
        if (student == nullptr) {
            return "unknown student " + fields[0];
        }
        if (courses.find(fields[1]) == nullptr) {
            return "unknown course " + fields[1];
        }
        if (!isValidGrade(fields[2])) {
            return "invalid grade '" + fields[2] + "'";
        }
        // Policy: any recorded grade, including F, counts as completing the course.
        student->addCompletedCourse(fields[1]);
        return "";
    });
}

void DataRepository::loadRegistrations(const std::string& filename) {
    HashTable<std::string, bool> seen;
    readCsv(filename, 2, [this, &seen](const std::vector<std::string>& fields) -> std::string {
        int studentId = 0;
        if (!parseNonNegativeInt(fields[0], studentId)) {
            return "invalid student ID '" + fields[0] + "'";
        }
        if (students.find(studentId) == nullptr) {
            return "unknown student " + fields[0];
        }
        if (courses.find(fields[1]) == nullptr) {
            return "unknown course " + fields[1];
        }
        const std::string key = fields[0] + "|" + fields[1];
        if (seen.find(key) != nullptr) {
            return "duplicate registration " + fields[0] + " in " + fields[1];
        }
        seen.insert(key, true);

        // Existing state is kept as given, even if it would fail today's rules.
        initialRegistrations.push_back(RegistrationRecord{studentId, fields[1]});
        return "";
    });
}