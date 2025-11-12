# LEX Editor Installation & Uninstallation Guide

## 🔧 Prerequisites

Ensure the project includes `cmake/uninstall.cmake.in` and that you have `CMake >= 3.15` installed.

## 📦 Build & Install

```bash
# 1. Create build directory
mkdir -p build
cd build

# 2. Generate build files with CMake
cmake ..

# 3. Compile the project
make -j$(nproc)

# 4. Install to system (requires sudo)
sudo make install
```

> Default install location: `/usr/local/bin/lex`

## 🔍 Check Installation

```bash
# Check lex binary location
which lex

# Check specific locations
ls -lh /usr/local/bin/lex
ls -lh /usr/bin/lex
```

## 🗑️ Uninstall (3 Methods)

### Method 1: Normal Uninstall (Recommended)

Uses the `install_manifest.txt` created during installation:

```bash
cd build
sudo make uninstall
```

> Works only if `build/` exists and `install_manifest.txt` is present.

### Method 2: Force Uninstall

If method 1 fails:

```bash
cd build
sudo make force-uninstall
rm -rf build
cd ..
```

Removes binary from:

* `/usr/local/bin/lex`
* `/usr/bin/lex`
* `${CMAKE_INSTALL_PREFIX}/bin/lex`

### Method 3: Manual Uninstall

```bash
sudo rm -f /usr/local/bin/lex
sudo rm -f /usr/bin/lex
which lex # Should show nothing
```

## 🧹 Purge (Uninstall + Delete Build)

```bash
# Run from build or root
sudo make -C build purge
sudo make purge
cd ..
```

> This runs uninstall first, then deletes the `build/` folder.

## 📍 Change Installation Location

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
sudo make install
# Binary will be at: /usr/bin/lex
```

## ❓ Troubleshooting

### Binary still exists

```bash
sudo find / -name "lex" -type f 2>/dev/null
which lex
sudo rm -f /path/to/lex
```

### Permission denied

```bash
sudo make uninstall
# or
sudo make force-uninstall
```

### install_manifest.txt missing

```bash
cd build
sudo make force-uninstall
```

### Conflict with system 'lex'

* Rename binary in `CMakeLists.txt`:

```cmake
project(mylex VERSION 0.5.2 LANGUAGES C)
```

* Or install locally:

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make
make install
export PATH="$HOME/.local/bin:$PATH"
```

## 📝 Complete Workflow

```bash
# Build & Install
mkdir build && cd build
cmake ..
make
sudo make install

# Check installation
make check-install
which lex
lex --version
lex myfile.txt

# Uninstall
sudo make uninstall
# Or force uninstall
sudo make force-uninstall

# Rebuild after code changes
cd build
make
sudo make install

# Full clean
sudo make purge
```

## ✅ Verify Successful Uninstall

```bash
which lex       # Should show nothing
lex             # bash: lex: command not found
ls /usr/local/bin/lex  # No such file or directory
```

## 🎯 Available CMake Targets

| Target          | Command                     | Description                                |
| --------------- | --------------------------- | ------------------------------------------ |
| Build           | `make`                      | Compile the project                        |
| Install         | `sudo make install`         | Install binary to system                   |
| Check           | `make check-install`        | Verify installation                        |
| Uninstall       | `sudo make uninstall`       | Remove installed files (requires manifest) |
| Force Uninstall | `sudo make force-uninstall` | Force remove binary                        |
| Purge           | `sudo make purge`           | Uninstall + delete build folder            |

## 🔐 Security Notes

* Use `sudo` when installing/uninstalling to system paths.
* Local installation without sudo:

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make
```
