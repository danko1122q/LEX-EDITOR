# Resource Files for Lex

This directory contains resource files that are bundled with Lex for portability and customization.

## Directory Structure

```
resources/
├── syntax/              # Syntax highlighting definitions
│   ├── c.json
│   ├── cpp.json
│   ├── java.json
│   ├── json.json
│   ├── make.json
│   ├── python.json
│   ├── rust.json
│   └── zig.json
├── themes/              # Color themes
│   ├── dark.nino
│   ├── light.nino
│   ├── monokai.nino
│   ├── ms-dos.nino
│   └── solarized.nino
└── bundle.c             # Resource bundling (for compilation)
```

## Syntax Files

Files in the `syntax/` folder are bundled in the binary for portability. These provide syntax highlighting for various programming languages.

### Built-in Syntax Support

Lex includes syntax highlighting for:
- **C** - `c.json`
- **C++** - `cpp.json`
- **Java** - `java.json`
- **JSON** - `json.json`
- **Makefile** - `make.json`
- **Python** - `python.json`
- **Rust** - `rust.json`
- **Zig** - `zig.json`

### Custom Syntax Files

If you would like to create your own syntax files, place them in:

* **Linux/macOS:** `~/.config/lex/syntax/`
* **Windows:** `~/.lex/syntax/`

Custom syntax files will override built-in definitions if they have the same filename.

See the [Syntax Highlighting documentation](./SYNTAX_HIGHLIGHTING.md) for detailed information on creating custom syntax files.

## Themes

The `themes/` folder contains example color themes that you can use to customize the appearance of Lex.

### Available Themes

* **dark.nino** - Dark color scheme
* **light.nino** - Light color scheme  
* **monokai.nino** - Popular Monokai theme
* **ms-dos.nino** - Retro MS-DOS style
* **solarized.nino** - Solarized color palette

### Installing Themes

Copy theme files to your configuration directory:

* **Linux/macOS:** `~/.config/lex/`
* **Windows:** `~/.lex/`

### Applying Themes

To apply a theme:

1. Press `Ctrl+P` to open the command prompt
2. Type: `exec <theme-name>.nino`
3. Press `Enter`

**Examples:**

```bash
# Apply dark theme
exec dark.nino

# Apply solarized theme
exec solarized.nino
```

### Creating Custom Themes

Theme files use configuration syntax to define colors and styles. You can modify existing themes or create your own.

**Example theme structure:**

```bash
# Color definitions (see solarized.nino for format)
color bg 002b36
color top.fg cccccc
color top.bg 004852
color top.tabs.fg 93a1a1
color top.tabs.bg 003440
# ... more color settings
```

**Color format:** Use 6-digit hexadecimal RGB values (e.g., `002b36`, `cccccc`)

### Theme Color Properties

Common theme properties you can customize:

* `color bg` - Background color
* `color top.fg` / `color top.bg` - Top bar colors
* `color top.tabs.fg` / `color top.tabs.bg` - Tab colors
* `color top.select.fg` / `color top.select.bg` - Selected tab colors
* `color explorer.*` - File explorer colors
* `color prompt.*` - Prompt/command line colors
* `color status.*` - Status bar colors
* `color lineno.*` - Line number colors
* `color hl.*` - Syntax highlighting colors

## Configuration Directory

User configuration files and custom resources should be placed in:

* **Linux/macOS:** `~/.config/lex/`
* **Windows:** `~/.lex/`

**Example structure:**

```
~/.config/lex/          (Linux/macOS)
~/.lex/                 (Windows)
├── syntax/             # Custom syntax files
│   └── mylang.json
├── dark.nino           # Custom theme
├── light.nino          # Custom theme
└── config              # Main configuration file
```

## Tips

* **Browse existing files** - Check the `resources/` folder for examples
* **Backup before modifying** - Keep copies of original theme files
* **Test changes incrementally** - Apply one change at a time to identify issues
* **Use hex color pickers** - Tools like https://htmlcolorcodes.com/ help choose colors
* **Share your work** - Consider contributing custom themes or syntax files to the project

## Note

The `.nino` extension for theme files is maintained for compatibility with the original nino project that Lex is based on.