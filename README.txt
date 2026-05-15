# Building

# Prerequisites

- Qt 6 (recommended) or *Qt 5.15+
- A C++17-capable compiler (GCC 9+, Clang 11+, MSVC 2019+)
- `qmake` or `cmake`

### Compiling with qmake

git clone https://github.com/your-username/JournalDB.git
cd JournalDB
qmake JournalDB.pro
make with Linux/macOS
OR
nmake with Windows (MSVC)
OR
mingw32-make with Windows (MinGW)

The binary appears as *JournalDB* (Linux/macOS) or *JournalDB.exe* on (Windows).

### With Qt Creator

1. Open Qt Creator.
2. File --> Open File or Project --> select `JournalDB.pro`.
3. Configure a kit (Qt 6 or Qt 5.15+).
4. Press *Ctrl+R* to build and run.

... and voila! 