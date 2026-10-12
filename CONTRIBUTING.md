# Contributing to SrikanthMart

Welcome to **SrikanthMart**! This guide outlines the exact setup steps to build and run the native C++20 project.

## Prerequisites
- C++20 compliant compiler: GCC 12+, Clang 15+, or MSVC 2022+
- CMake >= 3.25
- Ninja build system
- [vcpkg](https://github.com/microsoft/vcpkg) package manager
- SQLite 3 or PostgreSQL 15+

---

## Setup & Build Steps

### 1. Clone the Repository
```bash
git clone https://github.com/srikanth/srikanthmart.git
cd srikanthmart
```

### 2. Install Dependencies via vcpkg
```bash
# If vcpkg is cloned in ~/vcpkg or C:/vcpkg:
vcpkg install --triplet=x64-linux # or x64-windows
```

### 3. Configure CMake
```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DCMAKE_BUILD_TYPE=Release
```

### 4. Build the Binary
```bash
cmake --build build --config Release
```

### 5. Run the Tests
```bash
cd build && ctest --output-on-failure
```

### 6. Run the Application
```bash
./build/srikanthmart
```
The server will boot, run database migrations automatically, and serve on `http://localhost:8080`.

---

## Instant Local Dev Runner (Python)
If working in an environment without the full C++20 toolchain configured:
```bash
python server.py
```
Open `http://localhost:8080` in your web browser.
