# Lex

Lex is a **lightweight, simple, and terminal-based text editor**, inspired by GNU Nano. It aims to provide a clean, responsive, and distraction-free editing experience for developers who prefer to stay in the terminal. Unlike full-featured IDEs, Lex focuses on simplicity and essential features only.

---

## ✨ Features

* 🧩 **Familiar Interface** — Simple and minimal layout inspired by Nano
* 🎨 **Syntax Highlighting** — Basic syntax support for common languages (C, C++, Python, etc.)
* 🧭 **Lightweight & Fast** — Runs smoothly on any terminal without heavy dependencies
* 💾 **Basic File Operations** — Create, open, edit, and save files easily
* 🧠 **Undo/Redo Support** — Undo mistakes with a single shortcut
* 🖱️ **Mouse Support (Optional)** — Click to move or select text if supported by your terminal
* ⚙️ **Customizable Settings** — Change tab width, line numbers, and more using simple commands

---

## 📦 Installation

### Requirements

* C compiler (GCC or Clang)
* CMake 3.15+
* Make or other CMake-compatible build tool

### Build from Source
```bash
# 1. Clone the repository
git clone https://github.com/danko1122q/lex.git
cd lex

# 2. Create and enter build directory
mkdir build && cd build

# 3. Configure the project
cmake ..

# 4. Build using all CPU cores
make -j$(nproc)

# 5. (Optional) Install system-wide
sudo cmake --install .
```

### Uninstallation

```bash
cd build
sudo ./uninstall.sh
```

---

## 🚀 Usage

Run Lex from your terminal:

```bash
lex [filename]
```

If no filename is given, a new empty buffer will open.

### Interface Overview

* **Editor Area** — Main text input and editing area
* **Status Bar** — Displays file name, cursor position, and quick help
* **Command Line (Optional)** — Access basic configuration commands

---

## ⌨️ Keyboard Shortcuts

| Shortcut   | Action           |
| ---------- | ---------------- |
| `Ctrl + O` | Save file        |
| `Ctrl + X` | Exit editor      |
| `Ctrl + G` | Show help        |
| `Ctrl + W` | Search text      |
| `Ctrl + K` | Cut current line |
| `Ctrl + U` | Paste text       |
| `Ctrl + Z` | Undo             |
| `Ctrl + Y` | Redo             |

---

## ⚙️ Configuration

Use the internal command prompt (`Ctrl + P`) to change editor behavior.

Example commands:

```bash
set tabsize 4
set lineno 1
set helpinfo 0
```

---

## 📜 License

Lex is distributed under a **Modified MIT License (Non-Commercial)**. You are free to **use, modify, and fork** this project for personal or educational purposes.

Commercial redistribution or resale is **not allowed** without written permission from the author.

> © 2025 **danko1122q** — All rights reserved.
> See the [LICENSE](LICENSE) file for full details.

---

## 🙏 Acknowledgments

* **GNU Nano** — for inspiring the simplicity and interface design
* **Open Source Community** — for tools, libraries, and continuous support
* **Contributors** — who help improve and maintain the project
