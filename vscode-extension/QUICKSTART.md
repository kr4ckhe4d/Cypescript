# Cypescript VSCode Extension - Quick Start

## 🚀 Installation

```bash
cd vscode-extension/
./install.sh
```

## ✨ Features Overview

### 🎨 **Syntax Highlighting**
- Keywords: `let`, `if`, `while`, `for`
- Types: `string`, `i32`, `f64`, `boolean`, arrays
- Functions: Built-in and C++ integration functions
- Comments, strings, numbers with proper colors

### 🧠 **IntelliSense**
- **Auto-completion** as you type
- **Function signatures** with parameter info
- **Hover documentation** for all functions
- **Type information** for variables

### 🔧 **Build Integration**
- `Ctrl+F5` / `Cmd+F5`: Compile and run basic examples
- `Ctrl+Shift+F5` / `Cmd+Shift+F5`: Compile with C++ integration
- Right-click context menu options
- Integrated terminal execution

### 📝 **Code Snippets**
Type these prefixes and press Tab:
- `let` → Variable declaration
- `array` → Array declaration  
- `if` → If statement
- `for` → For loop
- `println` → Print statement
- `template` → Basic program template
- `cpptemplate` → C++ integration template

### 🚨 **Smart Diagnostics**
- Missing semicolon warnings
- Unsupported feature detection
- Browser vs native compiler guidance
- Real-time error highlighting

## 🎯 Quick Demo

1. **Create a new file**: `test.csc`
2. **Type**: `template` and press Tab
3. **Fill in the template** with your code
4. **Press**: `Ctrl+F5` to compile and run
5. **Enjoy**: Full IntelliSense and syntax highlighting!

## 📚 Code Examples

### Basic Example
```cypescript
let message: string = "Hello, VSCode!";
let count: i32 = 42;

println(message);
println(count);
```

### With IntelliSense
- Type `let` + Tab → Gets variable template
- Type `println` + Tab → Gets function call template
- Hover over `println` → See documentation
- `Ctrl+Space` → See all available completions

### C++ Integration
```cypescript
let text: string = "Hello World";
let reversed: string = string_reverse(text); // Auto-complete available!
println(reversed);
```

## 🎨 Customization

Add to your VSCode `settings.json`:
```json
{
  "cypescript.enableIntelliSense": true,
  "cypescript.enableDiagnostics": true,
  "cypescript.compilerPath": "./build/cscript"
}
```

## 🔍 Troubleshooting

### Extension not working?
1. Check if `.csc` file is open
2. Reload VSCode window (`Ctrl+Shift+P` → "Reload Window")
3. Check VSCode Extensions panel for "Cypescript"

### Compilation not working?
1. Make sure you're in the Cypescript project root
2. Check that `./build/cscript` exists
3. Try manual compilation first: `./compile-run.sh example/basic/hello.csc`

### IntelliSense not showing?
1. Check language mode (bottom-right should show "Cypescript")
2. Try `Ctrl+Space` to trigger manually
3. Check settings: `cypescript.enableIntelliSense`

## 🎉 What's Next?

1. **Explore examples**: Open files from `example/basic/`
2. **Try C++ integration**: Use `example/cpp-integration/`
3. **Write your own code**: Create new `.csc` files
4. **Use snippets**: Type prefixes and press Tab
5. **Learn more**: Use `Cypescript: Open Interactive Documentation`

## 📞 Support

- **Issues**: Check main Cypescript repository
- **Documentation**: Run `./launch-docs.sh`
- **Examples**: Browse `example/` directory
- **Community**: Join discussions in the main repo

Happy coding with Cypescript! 🚀
