# Lex

Lex is a lightweight, fast, and powerful terminal-based text editor. Inspired by the user-friendliness of nano, Lex is designed for developers who want a modern, feature-rich editing experience without leaving the comfort of their terminal.

## ✨ Features

* 🎯 **Intuitive Shortcuts** - Familiar shortcuts like `Ctrl+X` to quit and `Ctrl+O` to save, inspired by nano
* 🎨 **Syntax Highlighting** - Supports a wide range of programming languages (C, C++, Python, Rust, Java, Makefiles, and more)
* 📑 **Multi-File Tabs** - Seamlessly open and switch between multiple files in a single session
* 📁 **Integrated File Explorer** - Navigate your project and open files without leaving the editor
* 🔍 **Powerful Search** - Find text within your files with ease
* ⚙️ **Highly Customizable** - Tweak editor behavior on-the-fly using the internal command prompt (`Ctrl+P`)
* 🖱️ **Mouse Support** - Navigate, select text, and resize windows with your mouse
* ↩️ **Undo/Redo** - Full support for undoing and redoing your changes

## 📦 Installation

### Prerequisites

* A C compiler (GCC or Clang)
* CMake (version 3.15 or newer)
* Make (or another build system supported by CMake)

### Build from Source

```bash
# 1. Clone the repository
git clone https://github.com/danko1122q/lex.git
cd lex

# 2. Create a build directory
mkdir build
cd build

# 3. Configure the project with CMake
cmake ..

# 4. Compile the project
cmake --build .

# 5. (Optional) Install system-wide for access from any directory
sudo cmake --install .
```

## 🚀 Quick Start

Once installed, you can start Lex from your terminal:

```bash
lex [filename]
```

If no filename is provided, Lex will open with a new, empty buffer.

### User Interface

* **Top Bar** - Displays open file tabs
* **Editor Area** - The main area for writing and editing code
* **Left Sidebar** - (If enabled) Shows line numbers and the file explorer
* **Status Bar (Bottom)** - Shows essential shortcuts, the current filename, cursor position, and file information

## ⌨️ Keyboard Shortcuts

Lex uses shortcuts designed to be familiar to users of nano and other modern editors.

### File Operations

| Shortcut | Action |
|----------|--------|
| `Ctrl + O` | Save the current file. Press `Enter` to confirm |
| `Ctrl + S` | Open a file |
| `Ctrl + X` | Quit the editor |
| `Ctrl + W` | Close the current file tab |
| `Ctrl + N` | Create a new, untitled file in a new tab |

### Editing & Clipboard

| Shortcut | Action |
|----------|--------|
| `Ctrl + C` | Copy selected text or the current line |
| `Ctrl + V` | Paste text from the clipboard |
| `Alt + X` | Cut selected text or the current line |
| `Ctrl + A` | Select all text in the current file |
| `Ctrl + Z` | Undo the last action |
| `Ctrl + Y` | Redo the last undone action |
| `Ctrl + D` | Select the current word |
| `Backspace` | Delete the character before the cursor |
| `Delete` | Delete the character at the cursor |

### Navigation

| Shortcut | Action |
|----------|--------|
| `Arrow Keys` | Move the cursor |
| `Ctrl + F` | Find text in the file |
| `Ctrl + G` | Go to a specific line number |
| `Page Up / Page Down` | Scroll up or down one page |
| `Home / End` | Move to the beginning or end of the line |
| `Ctrl + Home / End` | Move to the beginning or end of the file |
| `Ctrl + [ / ]` | Switch to the previous / next file tab |

### UI & View Controls

| Shortcut | Action |
|----------|--------|
| `Ctrl + B` | Toggle the file explorer on/off |
| `Ctrl + E` | Set focus to the file explorer |
| `Ctrl + P` | Open the internal command prompt for configuration |

### Prompt Mode

When a prompt is active (e.g., after `Ctrl+P`, `Ctrl+F`, or `Ctrl+G`):

| Shortcut | Action |
|----------|--------|
| `Ctrl + X` or `Esc` | Cancel the prompt and return to the editor |
| `Enter` | Confirm the action or input |
| `Arrow Up / Down` | Navigate through search results or command history |

## ⚙️ Configuration

Lex can be configured on-the-fly using its internal command prompt. Press `Ctrl+P` to open the prompt and type a command.

### Example Commands

```bash
# Set the tab size to 4 spaces
set tabsize 4

# Hide the help text in the status bar for a cleaner view
set helpinfo 0

# Enable system clipboard integration (requires a terminal that supports OSC 52)
set osc52_copy 1

# Set the default width of the file explorer
set ex_default_width 25

# Show or hide line numbers
set lineno 1
```

## 🤝 About This Project

**Lex is a rebranded fork of the nino text editor.** This project maintains the same codebase and functionality as nino, with updated branding and documentation. All credit for the original implementation goes to the nino project and its contributors.

The nino project was inspired by the simplicity and user-friendliness of GNU nano, bringing that familiar experience to modern developers with enhanced features and performance.

## 📄 License

This project maintains the original MIT License from nino. See the **LICENSE** file for details.

**Important:** Lex is a rebranded fork of nino. Please see the **NOTICE** file for full attribution and third-party acknowledgments.

## 🙏 Acknowledgments

* **All original contributors of the nino project** - This fork would not exist without their excellent work
* The nino project for creating a solid foundation for this editor
* The nano community for inspiring the design philosophy
* Inspired by the simplicity and power of GNU nano