# Cypescript Language Support for VSCode

This extension provides comprehensive language support for **Cypescript** - a TypeScript-style language that compiles to native code through LLVM.

## Features

### 🎨 **Syntax Highlighting**
- Full syntax highlighting for Cypescript (.csc) files
- Support for keywords, types, strings, numbers, comments
- Special highlighting for C++ integration functions
- Proper bracket matching and auto-closing

### 🧠 **IntelliSense**
- **Auto-completion** for keywords, types, and functions
- **Hover documentation** for built-in and C++ functions
- **Function signatures** with parameter information
- **Type information** for variables and expressions

### 🔧 **Build Integration**
- **Compile and Run** (`Ctrl+F5` / `Cmd+F5`)
- **C++ Integration Compilation** (`Ctrl+Shift+F5` / `Cmd+Shift+F5`)
- **Interactive Documentation** launcher
- Integrated terminal execution

### 📝 **Code Snippets**
- Variable declarations (`let`, `array`)
- Control structures (`if`, `while`, `for`)
- Function calls (`print`, `println`)
- C++ integration functions
- Program templates

### 🚨 **Diagnostics**
- Real-time error detection
- Warnings for missing semicolons
- Information about unsupported features in native compiler
- Suggestions for browser interpreter usage

## Quick Start

1. **Install the extension** (when published to marketplace)
2. **Open a Cypescript file** (`.csc` extension)
3. **Start coding** with full IntelliSense support
4. **Compile and run** with `Ctrl+F5`

## Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| `Cypescript: Compile and Run` | `Ctrl+F5` | Compile and run using basic pipeline |
| `Cypescript: Compile with C++ Integration` | `Ctrl+Shift+F5` | Compile with C++ library support |
| `Cypescript: Compile Cypescript File` | - | Compile only (no execution) |
| `Cypescript: Open Interactive Documentation` | - | Launch browser documentation |

## Language Features

### ✅ **Fully Supported (Native Compiler)**
- Variables: `let name: string = "value";`
- Types: `string`, `i32`, arrays (`i32[]`, `string[]`)
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparisons: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Control flow: `if`/`else`, `while`, `for`, `do-while`
- Arrays: creation, access, assignment, `.length`
- Built-in functions: `print()`, `println()`

### 🚀 **C++ Integration Functions**
- **String functions**: `string_reverse()`, `string_upper()`, `string_lower()`
- **Array functions**: `array_sum_i32()`, `array_max_i32()`, `array_min_i32()`
- **File I/O**: `file_read()`, `file_write()`, `file_exists()`
- **Utilities**: `random_int()`, `random_seed()`, `math_abs_i32()`

### 🌐 **Browser Interpreter Only**
- Object literals: `{ key: value }`
- Boolean literals: `true`, `false`
- Float literals: `3.14`
- Complex nested structures

## Code Examples

### Basic Program
```cypescript
let message: string = "Hello, Cypescript!";
let count: i32 = 42;

println(message);
print("Count: ");
println(count);
```

### C++ Integration
```cypescript
let text: string = "Hello World";
let reversed: string = string_reverse(text);
println(reversed); // "dlroW olleH"

let numbers: i32[] = [10, 5, 8, 3, 12];
let sum: i32 = array_sum_i32(numbers, numbers.length);
println(sum); // 38
```

### Control Flow
```cypescript
let score: i32 = 85;

if (score >= 90) {
    println("Grade: A");
} else {
    if (score >= 80) {
        println("Grade: B");
    } else {
        println("Grade: C");
    }
}
```

## Configuration

The extension can be configured through VSCode settings:

```json
{
  "cypescript.compilerPath": "./build/cscript",
  "cypescript.enableIntelliSense": true,
  "cypescript.enableDiagnostics": true,
  "cypescript.showWelcomeMessage": true
}
```

## Requirements

- **Cypescript compiler** built and available
- **LLVM** installed for native compilation
- **Node.js** for extension development

## Installation

### From Source
1. Clone the Cypescript repository
2. Navigate to `vscode-extension/`
3. Run `npm install`
4. Run `npm run compile`
5. Press `F5` to launch extension development host

### From Marketplace (Future)
1. Open VSCode
2. Go to Extensions (`Ctrl+Shift+X`)
3. Search for "Cypescript"
4. Click Install

## Development

### Building the Extension
```bash
cd vscode-extension/
npm install
npm run compile
```

### Testing
```bash
# Launch extension development host
code --extensionDevelopmentPath=.
```

### Packaging
```bash
npm install -g vsce
vsce package
```

## Contributing

Contributions are welcome! Please see the main Cypescript repository for contribution guidelines.

## License

MIT License - see the main Cypescript repository for details.

## Links

- **Main Repository**: [Cypescript](https://github.com/your-username/cypescript)
- **Documentation**: Run `./launch-docs.sh` in Cypescript project
- **Issues**: [GitHub Issues](https://github.com/your-username/cypescript/issues)
