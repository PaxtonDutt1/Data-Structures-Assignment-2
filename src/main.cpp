#include "DataRepository.h"
#include "RegistrationManager.h"
#include <cctype>
#include <iostream>
#include <string>

namespace {

const std::string DEFAULT_SAVE_FILE = "saved_registrations.csv";

void printMenu() {
    std::cout
        << "\n=== University Registration & Waitlist System ===\n"
        << "1. Register student\n"
        << "2. Drop student\n"
        << "3. Undo last registration/drop\n"
        << "4. View student schedule\n"
        << "5. View course roster\n"
        << "6. View course waitlist\n"
        << "7. Process course waitlist\n"
        << "8. Save registrations\n"
        << "9. Show system statistics\n"
        << "10. Exit\n"
        << "Choice: ";
}

std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

// Reads one trimmed line. Returns false when input has ended.
bool readLine(const std::string& prompt, std::string& text) {
    std::cout << prompt;
    if (!std::getline(std::cin, text)) {
        return false;
    }
    text = trim(text);
    return true;
}

// Accepts only a complete non-negative integer, so "12x" is rejected.
bool parseInt(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }
    for (char c : text) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    try {
        value = std::stoi(text);
        return true;
    } catch (const std::exception&) {   // too large for int
        return false;
    }
}

bool readStudentId(int& studentId) {
    std::string text;
    if (!readLine("Student ID: ", text)) {
        return false;
    }
    if (!parseInt(text, studentId)) {
        std::cout << "'" << text << "' is not a valid student ID.\n";
        return false;
    }
    return true;
}

bool readCourseCode(std::string& courseCode) {
    if (!readLine("Course code: ", courseCode)) {
        return false;
    }
    for (char& c : courseCode) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return true;
}

void handleChoice(int choice, RegistrationManager& manager) {
    int studentId = 0;
    std::string courseCode;

    switch (choice) {
    case 1:
        if (readStudentId(studentId) && readCourseCode(courseCode)) {
            manager.registerStudent(studentId, courseCode);
        }
        break;
    case 2:
        if (readStudentId(studentId) && readCourseCode(courseCode)) {
            manager.dropStudent(studentId, courseCode);
        }
        break;
    case 3:
        manager.undoLastAction();
        break;
    case 4:
        if (readStudentId(studentId)) {
            manager.printStudentSchedule(studentId);
        }
        break;
    case 5:
        if (readCourseCode(courseCode)) {
            manager.printCourseRoster(courseCode);
        }
        break;
    case 6:
        if (readCourseCode(courseCode)) {
            manager.printWaitlist(courseCode);
        }
        break;
    case 7:
        if (readCourseCode(courseCode)) {
            manager.processWaitlist(courseCode);
        }
        break;
    case 8: {
        std::string filename;
        if (readLine("Output file [" + DEFAULT_SAVE_FILE + "]: ", filename)) {
            manager.saveRegistrations(filename.empty() ? DEFAULT_SAVE_FILE : filename);
        }
        break;
    }
    case 9:
        manager.printStatistics();
        break;
    default:
        std::cout << "Please choose a number from 1 to 10.\n";
        break;
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string dataDirectory = (argc > 1) ? argv[1] : "../data";

    try {
        DataRepository repository;
        repository.loadAll(dataDirectory);

        RegistrationManager manager(repository);
        manager.initializeFromRepository();

        while (true) {
            printMenu();

            std::string text;
            if (!std::getline(std::cin, text)) {
                std::cout << "\nInput ended.\n";
                break;
            }
            text = trim(text);

            int choice = 0;
            if (!parseInt(text, choice)) {
                std::cout << "'" << text << "' is not a menu option. "
                          << "Please choose a number from 1 to 10.\n";
                continue;
            }
            if (choice == 10) {
                break;
            }
            handleChoice(choice, manager);
        }

        // The specification requires the final state to be saved before exit.
        manager.saveRegistrations(DEFAULT_SAVE_FILE);
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        std::cerr << "Usage: a2 <data-directory>\n";
        return 1;
    }

    return 0;
}