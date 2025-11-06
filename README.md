# Lex

Lex is a lightweight, fast, and powerful terminal-based text editor. It is designed for developers who want a modern, feature-rich editing experience without leaving the comfort of their terminal.

## ✨ Features

* 🎯 **Intuitive Shortcuts** - Familiar key bindings for smooth and efficient editing
* 🎨 **Syntax Highlighting** - Supports many programming languages (C, C++, Python, Rust, Java, Makefiles, and more)
* 📑 **Multi-File Tabs** - Easily open and switch between multiple files in one session
* 📁 **Integrated File Explorer** - Navigate your project and open files without leaving the editor
* 🔍 **Powerful Search** - Quickly find text within your files
* ⚙️ **Highly Customizable** - Adjust settings dynamically using the internal command prompt (`Ctrl+P`)
* 🖱️ **Mouse Support** - Navigate, select, and resize with your mouse
* ↩️ **Undo/Redo** - Fully supports undoing and redoing changes

## 📦 Installation

### Prerequisites

* A C compiler (GCC or Clang)
* CMake (version 3.15 or newer)
* Make (or another CMake-compatible build system)

### Build from Source

```bash
# 1. Clone the repository
git clone https://github.com/danko1122q/lex.git
cd lex

# 2. Create a build directory
mkdir build
cd build

# 3. Configure with CMake
cmake ..

# 4. Compile the project
cmake --build .

# 5. (Optional) Install system-wide
sudo cmake --install .
```

### Uninstallation

```bash
# Navigate to the build directory
cd build

# Run the generated uninstall script
sudo ./uninstall.sh
```

## 🚀 Quick Start

Once installed, start Lex from your terminal:

```bash
lex [filename]
```

If no filename is provided, Lex opens a new, empty buffer.

### User Interface

* **Top Bar** - Displays open file tabs
* **Editor Area** - The main space for writing and editing code
* **Left Sidebar** - (Optional) Shows line numbers and file explorer
* **Status Bar (Bottom)** - Displays shortcuts, filename, cursor position, and file details

## ⌨️ Keyboard Shortcuts

### File Operations

| Shortcut   | Action                                  |
| ---------- | --------------------------------------- |
| `Ctrl + O` | Save the current file                   |
| `Ctrl + S` | Open a file                             |
| `Ctrl + X` | Quit the editor                         |
| `Ctrl + W` | Close the current file tab              |
| `Ctrl + N` | Create a new untitled file in a new tab |

### Editing & Clipboard

| Shortcut    | Action                             |
| ----------- | ---------------------------------- |
| `Ctrl + C`  | Copy selected text or current line |
| `Ctrl + V`  | Paste text from clipboard          |
| `Alt + X`   | Cut selected text or current line  |
| `Ctrl + A`  | Select all text in the file        |
| `Ctrl + Z`  | Undo last action                   |
| `Ctrl + Y`  | Redo last undone action            |
| `Ctrl + D`  | Select current word                |
| `Backspace` | Delete previous character          |
| `Delete`    | Delete character at cursor         |

### Navigation

| Shortcut            | Action                       |
| ------------------- | ---------------------------- |
| `Arrow Keys`        | Move the cursor              |
| `Ctrl + F`          | Find text in file            |
| `Ctrl + G`          | Go to a specific line number |
| `Page Up / Down`    | Scroll up or down            |
| `Home / End`        | Move to line start or end    |
| `Ctrl + Home / End` | Move to file start or end    |
| `Ctrl + [ / ]`      | Switch between file tabs     |

### UI & View Controls

| Shortcut   | Action                       |
| ---------- | ---------------------------- |
| `Ctrl + B` | Toggle file explorer         |
| `Ctrl + E` | Focus file explorer          |
| `Ctrl + P` | Open internal command prompt |

### Prompt Mode

| Shortcut            | Action                              |
| ------------------- | ----------------------------------- |
| `Ctrl + X` or `Esc` | Cancel and return to editor         |
| `Enter`             | Confirm input or action             |
| `Arrow Up / Down`   | Navigate results or command history |

## ⚙️ Configuration

Use the internal command prompt (`Ctrl+P`) to configure Lex dynamically.

### Example Commands

```bash
# Set tab size to 4 spaces
set tabsize 4

# Hide help text in status bar
set helpinfo 0

# Enable system clipboard integration
set osc52_copy 1

# Set file explorer default width
set ex_default_width 25

# Show or hide line numbers
set lineno 1
```

## 📄 License

Lex is licensed under the MIT License. See the **LICENSE** file for full details.

## 🙏 Acknowledgments

* All original contributors and developers who helped shape the foundation of this editor
* The open-source community for continuous support and inspiration
