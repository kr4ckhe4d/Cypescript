# Cypescript Web Documentation

Interactive web-based documentation for the Cypescript programming language with runnable examples.

## Features

- 📖 **Complete Language Reference** - Comprehensive documentation of all Cypescript features
- 🚀 **Runnable Examples** - Execute Cypescript code directly in the browser
- 🎮 **Interactive Playground** - Write and test your own Cypescript code
- 📱 **Responsive Design** - Works on desktop, tablet, and mobile devices
- 🔍 **Search Functionality** - Quickly find what you're looking for
- 🎨 **Modern UI** - Clean, professional interface with syntax highlighting

## Quick Start

### Option 1: Python Server (Recommended)

```bash
# Navigate to docs directory
cd docs

# Start the server (Python 3 required)
python3 serve.py

# Or specify a custom port
python3 serve.py 3000
```

The documentation will automatically open in your browser at `http://localhost:8000`.

### Option 2: Any HTTP Server

You can use any static file server:

```bash
# Using Node.js http-server
npx http-server docs -p 8000 -o

# Using Python 2
cd docs && python -m SimpleHTTPServer 8000

# Using PHP
cd docs && php -S localhost:8000
```

## Documentation Sections

### 1. **Overview**
- Language introduction and key features
- Quick example to get started
- Feature highlights with visual cards

### 2. **Getting Started**
- Installation instructions
- Your first program
- Compilation workflow

### 3. **Data Types**
- All supported types (`i32`, `f64`, `string`, `boolean`)
- Interactive examples for each type
- Type compatibility rules

### 4. **Variables**
- Variable declaration syntax
- Assignment operations
- Scope rules and examples

### 5. **Operators**
- Arithmetic operators (`+`, `-`, `*`, `/`, `%`)
- Comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- Operator precedence table
- Interactive examples

### 6. **Control Flow**
- If-else statements
- Nested conditionals
- Complex condition examples

### 7. **Loops**
- While loops
- For loops (traditional C-style)
- Do-while loops
- Nested loop examples

### 8. **Functions**
- Built-in functions (`print`)
- Function call syntax
- Examples with different data types

### 9. **Examples**
- **Factorial Calculator** - Demonstrates loops and arithmetic
- **Prime Number Checker** - Shows complex conditional logic
- **Fibonacci Sequence** - Nested loops and variable management

### 10. **Interactive Playground**
- Write custom Cypescript code
- Real-time execution
- Error handling and display

## Browser Interpreter

The documentation includes a complete Cypescript interpreter written in JavaScript that runs in the browser. This allows all examples to be executed without requiring the actual Cypescript compiler.

### Interpreter Features

- ✅ **Complete tokenizer** - Handles all Cypescript syntax
- ✅ **Recursive descent parser** - Builds abstract syntax trees
- ✅ **Full evaluator** - Executes all language constructs
- ✅ **Error handling** - Provides meaningful error messages
- ✅ **Variable scoping** - Proper variable management
- ✅ **All operators** - Arithmetic and comparison operations
- ✅ **Control flow** - If-else statements with nesting
- ✅ **All loop types** - While, for, and do-while loops
- ✅ **Built-in functions** - Print function with type support

### Supported Language Features

The browser interpreter supports all major Cypescript features:

```typescript
// Variables and types
let number: i32 = 42;
let text: string = "Hello";
let flag: boolean = true;

// Arithmetic operations
let result: i32 = (number + 10) * 2;

// Control flow
if (result > 50) {
    print("Large number!");
} else {
    print("Small number");
}

// Loops
for (let i: i32 = 0; i < 5; i = i + 1) {
    print("Count: ");
    print(i);
}

while (number > 0) {
    print(number);
    number = number - 1;
}

do {
    print("At least once");
} while (false);
```

## Technical Implementation

### Frontend Technologies

- **HTML5** - Semantic markup and structure
- **CSS3** - Modern styling with Grid and Flexbox
- **JavaScript ES6+** - Interactive functionality
- **Prism.js** - Syntax highlighting
- **Font Awesome** - Icons and visual elements

### Architecture

```
docs/
├── index.html              # Main documentation page
├── styles.css              # Complete styling
├── script.js               # Navigation and UI logic
├── cypescript-interpreter.js # Browser-based interpreter
├── serve.py                # Development server
└── README.md               # This file
```

### Key Components

1. **Navigation System** - Smooth section switching with URL hash support
2. **Code Execution Engine** - Full Cypescript interpreter in JavaScript
3. **Responsive Design** - Mobile-first approach with breakpoints
4. **Interactive Elements** - Runnable examples and playground
5. **Search Functionality** - Real-time documentation search
6. **Copy-to-Clipboard** - Easy code copying from examples

## Development

### Adding New Examples

To add a new runnable example:

1. Add the HTML structure:
```html
<div class="code-example">
    <div class="code-header">
        <span class="filename">example.csc</span>
        <button class="run-btn" onclick="runExample('my-example')">
            <i class="fas fa-play"></i> Run
        </button>
    </div>
    <pre><code class="language-typescript" id="my-example">
        // Your Cypescript code here
    </code></pre>
    <div class="output" id="my-example-output"></div>
</div>
```

2. The `runExample('my-example')` function will automatically:
   - Extract code from the element with id `my-example`
   - Execute it using the browser interpreter
   - Display results in `my-example-output`

### Extending the Interpreter

The browser interpreter is modular and can be extended:

```javascript
// Add new built-in functions
case 'CALL':
    if (node.name === 'myFunction') {
        // Handle your custom function
    }
    break;

// Add new operators
case 'BINARY':
    switch (node.operator) {
        case '**': return Math.pow(left, right); // Exponentiation
        // ... existing operators
    }
    break;
```

## Browser Compatibility

- ✅ **Chrome 60+**
- ✅ **Firefox 55+**
- ✅ **Safari 12+**
- ✅ **Edge 79+**
- ✅ **Mobile browsers** (iOS Safari, Chrome Mobile)

## Performance

- **Fast loading** - Optimized assets and minimal dependencies
- **Smooth interactions** - Hardware-accelerated animations
- **Efficient execution** - Optimized interpreter with proper error handling
- **Memory management** - Automatic cleanup of interpreter state

## Contributing

To contribute to the documentation:

1. **Content updates** - Edit `index.html` for new sections
2. **Styling changes** - Modify `styles.css` for visual improvements
3. **Functionality** - Update `script.js` for new features
4. **Interpreter** - Extend `cypescript-interpreter.js` for language features

## License

This documentation is part of the Cypescript project and follows the same MIT license.
