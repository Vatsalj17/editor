# Text Editor

A lightweight terminal-based text editor written in C with syntax highlighting and modern features.

## Features

- **Syntax highlighting** for C/C++ files
- **File operations** - open, save, create files
- **Search functionality** with incremental highlighting
- **Navigation** - arrow keys, page up/down, home/end
- **Status bar** showing file info and modifications
- **Tab support** with configurable tab stops

## Build & Run

```bash
make
./editr [filename]
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+S` | Save file |
| `Ctrl+Q` | Quit (warns if unsaved) |
| `Ctrl+F` | Search |
| `Arrow Keys` | Navigate |
| `Page Up/Down` | Scroll |
| `Home/End` | Jump to line start/end |

## Project Structure

```
editor/
├── include/    # Header files
├── src/        # Source files  
├── Makefile    # Build config
└── README.md
```

The codebase follows a modular design with separate files for terminal control, syntax highlighting, file I/O, search, and screen rendering.

## Requirements

- GCC compiler
- POSIX-compliant system (Linux, macOS, BSD)
- Terminal with ANSI escape sequences
