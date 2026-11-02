#include "DataRepository.h"
#include "RegistrationManager.h"
#include <iostream>
#include <string>

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
        << "0. Exit\n"
        << "Choice: ";
}

int main(int argc, char* argv[]) {
    std::string dataDirectory = (argc > 1) ? argv[1] : "../data";

    try {
        DataRepository repository;
        repository.loadAll(dataDirectory);

        RegistrationManager manager(repository);
        // manager.initializeFromRepository();

        // TODO: implement the complete menu loop.
        printMenu();
        std::cout << "\nStarter project loaded. Complete the TODO sections.\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
