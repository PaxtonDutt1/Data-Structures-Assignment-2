# Assignment 2 - University Registration & Waitlist System

A command-line registration system built in C++17. It manages students, course
sections, prerequisites, registrations, waitlists, drops, and undo.

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

This requires CMake 3.16 or newer and a C++17 compiler.

## Run

Pass the path to the data folder:

```bash
./build/a2 data
```

If you run the program from inside `build/`, the default path `../data` works:

```bash
cd build
./a2
```

On Windows, the executable is `a2.exe`. Visual Studio may place it in `build\Debug`.

## Test

```bash
./build/test_structures
./build/test_registration data
```

## Saved registrations

Menu option 8 saves registrations to a file you name, or to
`saved_registrations.csv` by default. The program also saves to the default file
on exit, in the folder you run it from.

The file has the same format as `data/initial_registrations.csv`:

```text
student_id,course_code
200471,MATH251
```

To load a saved file, replace `initial_registrations.csv` in a copy of the data
folder and pass that folder to the program.

## Project layout

- `src/` - application source code and custom data structures
- `tests/` - automated tests, including the required edge cases
- `data/` - provided input files
- `reports/` - design and performance report
- `saved_registrations.csv` - output from one complete run

## Design decisions

- Any recorded grade, including F, counts as completing a course.
- Initial registrations are kept as provided, even if they would fail current rules.
- Joining a waitlist or being promoted does not create an undo record.
- Undoing a drop fails if the course has no open seat.