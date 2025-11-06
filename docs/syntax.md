# Syntax Highlighting

Lex supports simple keyword-based syntax highlighting for various programming languages.

## Enabling Syntax Highlighting

To enable syntax highlighting, use the command prompt:

```bash
syntax 1
```

Press `Ctrl+P` to open the command prompt, then type the command above.

## Built-in Language Support

Lex comes with syntax highlighting for the following languages out of the box:

* **C** (`.c`, `.h`)
* **C++** (`.cpp`, `.hpp`, `.cc`, `.hh`, `.c++`, `.h++`)
* **Go** (`.go`)
* **HTML** (`.html`, `.htm`)
* **Java** (`.java`)
* **JavaScript** (`.js`, `.mjs`, `.cjs`)
* **JSON** (`.json`)
* **Makefile** (`Makefile`, `makefile`, `.mk`)
* **PHP** (`.php`, `.phtml`)
* **Python** (`.py`, `.pyw`)
* **Ruby** (`.rb`, `.rake`, `.gemspec`)
* **Rust** (`.rs`)
* **Shell** (`.sh`, `.bash`, `.zsh`)
* **SQL** (`.sql`)
* **TypeScript** (`.ts`, `.tsx`)
* **Zig** (`.zig`)

## Custom Syntax Files

You can create your own syntax highlighting definitions by adding custom syntax files.

### Syntax File Locations

Place your custom syntax files in the following directories:

* **Linux/macOS:** `~/.config/lex/syntax`
* **Windows:** `~/.lex/syntax`

Create the directory if it doesn't exist:

```bash
# Linux/macOS
mkdir -p ~/.config/lex/syntax

# Windows (PowerShell)
New-Item -ItemType Directory -Force -Path "$env:USERPROFILE\.lex\syntax"
```

## Syntax File Format

Syntax highlighting rules are stored in **JSON files**. Each file defines the highlighting rules for a specific language.

### Example Syntax File

```json
{
    "name": "Example Language",
    "extensions": [
        ".ext1",
        ".ext2"
    ],
    "comment": "//",
    "multiline-comment": [
        "/*",
        "*/"
    ],
    "keywords1": [
        "for",
        "while",
        "if",
        "else"
    ],
    "keywords2": [
        "int",
        "char",
        "float"
    ],
    "keywords3": [
        "string"
    ]
}
```

### Field Descriptions

| Field | Type | Description |
|-------|------|-------------|
| `name` | String | The display name of the language |
| `extensions` | Array | File extensions that will automatically use this syntax (e.g., `[".py", ".pyw"]`) |
| `comment` | String | The string used for single-line comments (e.g., `"//"`, `"#"`) |
| `multiline-comment` | Array | Two-element array defining the start and end of multi-line comments (e.g., `["/*", "*/"]`) |
| `keywords1` | Array | First group of keywords, typically for control flow statements (colored differently) |
| `keywords2` | Array | Second group of keywords, typically for data types and declarations |
| `keywords3` | Array | Third group of keywords, typically for built-in types, functions or special identifiers |

## Keyword Groups

Each keyword group (`keywords1`, `keywords2`, `keywords3`) is displayed with a different color to help distinguish between different types of syntax elements:

* **keywords1** - Usually for control flow keywords: `if`, `else`, `for`, `while`, `return`, `break`, `continue`, etc.
* **keywords2** - Usually for declarations and modifiers: `int`, `class`, `const`, `function`, `import`, `public`, etc.
* **keywords3** - Usually for built-in types and constants: `string`, `bool`, `true`, `false`, `null`, `undefined`, etc.

## Example: Kotlin Syntax File

Here's an example of how to create a syntax file for Kotlin:

```json
{
    "name": "Kotlin",
    "extensions": [
        ".kt",
        ".kts"
    ],
    "comment": "//",
    "multiline-comment": [
        "/*",
        "*/"
    ],
    "keywords1": [
        "break",
        "continue",
        "do",
        "else",
        "for",
        "if",
        "return",
        "when",
        "while",
        "throw",
        "try",
        "catch",
        "finally"
    ],
    "keywords2": [
        "abstract",
        "class",
        "const",
        "constructor",
        "enum",
        "final",
        "fun",
        "import",
        "interface",
        "internal",
        "object",
        "open",
        "override",
        "package",
        "private",
        "protected",
        "public",
        "sealed",
        "val",
        "var"
    ],
    "keywords3": [
        "Boolean",
        "Byte",
        "Char",
        "Double",
        "Float",
        "Int",
        "Long",
        "Short",
        "String",
        "true",
        "false",
        "null",
        "this",
        "super"
    ]
}
```

Save this file as `kotlin.json` in your syntax directory.

## Tips for Creating Syntax Files

1. **Keep it simple** - Only include the most common keywords to avoid cluttering
2. **Test your syntax** - Open a file with the specified extension to see if highlighting works
3. **Organize keywords logically** - Group similar keywords together for consistent coloring:
   - **keywords1**: Control flow and statements
   - **keywords2**: Declarations, modifiers, and structural keywords
   - **keywords3**: Types, built-ins, and constants
4. **Check existing files** - Look at built-in syntax files in the source code for examples and patterns
5. **Use meaningful names** - Name your JSON file after the language (e.g., `rust.json`, `go.json`)
6. **Follow conventions** - Use lowercase for the filename to match built-in syntax files

## Troubleshooting

If syntax highlighting isn't working:

1. Verify the syntax file is in the correct directory
2. Check that the JSON format is valid (use a JSON validator)
3. Ensure the file extension matches the `extensions` array in your syntax file
4. Confirm syntax highlighting is enabled with `syntax 1` command
5. Restart Lex after adding new syntax files
6. Check that custom syntax files don't conflict with built-in language definitions
