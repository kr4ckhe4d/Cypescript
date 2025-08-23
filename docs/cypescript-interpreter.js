// Cypescript Interpreter for Browser Execution
// This is a simplified interpreter that simulates Cypescript execution

class CypescriptInterpreter {
    constructor() {
        this.variables = new Map();
        this.output = [];
        this.currentLine = ''; // Track current line for print vs println
        this.reset();
    }
    
    reset() {
        this.variables.clear();
        this.output = [];
        this.currentLine = '';
    }
    
    // Tokenizer - converts source code into tokens
    tokenize(source) {
        const tokens = [];
        const keywords = new Set([
            'let', 'const', 'var', 'if', 'else', 'while', 'for', 'do',
            'i32', 'f64', 'string', 'boolean'
        ]);
        
        const operators = new Set([
            '==', '!=', '<=', '>=', '&&', '||', '++', '--',
            '=', '<', '>', '+', '-', '*', '/', '%', '!', '&', '|'
        ]);
        
        let i = 0;
        while (i < source.length) {
            const char = source[i];
            
            // Skip whitespace
            if (/\s/.test(char)) {
                i++;
                continue;
            }
            
            // Skip comments
            if (char === '/' && source[i + 1] === '/') {
                while (i < source.length && source[i] !== '\n') i++;
                continue;
            }
            
            if (char === '/' && source[i + 1] === '*') {
                i += 2;
                while (i < source.length - 1) {
                    if (source[i] === '*' && source[i + 1] === '/') {
                        i += 2;
                        break;
                    }
                    i++;
                }
                continue;
            }
            
            // String literals
            if (char === '"') {
                let str = '';
                i++; // Skip opening quote
                while (i < source.length && source[i] !== '"') {
                    if (source[i] === '\\' && i + 1 < source.length) {
                        i++; // Skip escape character
                        const escaped = source[i];
                        switch (escaped) {
                            case 'n': str += '\n'; break;
                            case 't': str += '\t'; break;
                            case 'r': str += '\r'; break;
                            case '\\': str += '\\'; break;
                            case '"': str += '"'; break;
                            default: str += escaped; break;
                        }
                    } else {
                        str += source[i];
                    }
                    i++;
                }
                i++; // Skip closing quote
                tokens.push({ type: 'STRING', value: str });
                continue;
            }
            
            // Numbers
            if (/\d/.test(char)) {
                let num = '';
                while (i < source.length && /[\d.]/.test(source[i])) {
                    num += source[i];
                    i++;
                }
                tokens.push({ 
                    type: 'NUMBER', 
                    value: num.includes('.') ? parseFloat(num) : parseInt(num) 
                });
                continue;
            }
            
            // Identifiers and keywords
            if (/[a-zA-Z_]/.test(char)) {
                let identifier = '';
                while (i < source.length && /[a-zA-Z0-9_]/.test(source[i])) {
                    identifier += source[i];
                    i++;
                }
                
                if (keywords.has(identifier)) {
                    tokens.push({ type: 'KEYWORD', value: identifier });
                } else if (identifier === 'true') {
                    tokens.push({ type: 'BOOLEAN', value: true });
                } else if (identifier === 'false') {
                    tokens.push({ type: 'BOOLEAN', value: false });
                } else {
                    tokens.push({ type: 'IDENTIFIER', value: identifier });
                }
                continue;
            }
            
            // Two-character operators
            if (i < source.length - 1) {
                const twoChar = source.substr(i, 2);
                if (operators.has(twoChar)) {
                    tokens.push({ type: 'OPERATOR', value: twoChar });
                    i += 2;
                    continue;
                }
            }
            
            // Single-character operators and punctuation
            if (operators.has(char) || '(){}[];:,.'.includes(char)) {
                if (char === '=') {
                    tokens.push({ type: 'ASSIGN', value: char });
                } else {
                    tokens.push({ type: 'OPERATOR', value: char });
                }
                i++;
                continue;
            }
            
            // Unknown character
            i++;
        }
        
        return tokens;
    }
    
    // Simple recursive descent parser
    parse(tokens) {
        let current = 0;
        
        const peek = () => tokens[current];
        const advance = () => tokens[current++];
        const match = (type, value = null) => {
            const token = peek();
            return token && token.type === type && (value === null || token.value === value);
        };
        
        const parseExpression = () => {
            return parseComparison();
        };
        
        const parseComparison = () => {
            let left = parseAddition();
            
            while (match('OPERATOR') && ['==', '!=', '<', '<=', '>', '>='].includes(peek().value)) {
                const operator = advance().value;
                const right = parseAddition();
                left = { type: 'BINARY', operator, left, right };
            }
            
            return left;
        };
        
        const parseAddition = () => {
            let left = parseMultiplication();
            
            while (match('OPERATOR') && ['+', '-'].includes(peek().value)) {
                const operator = advance().value;
                const right = parseMultiplication();
                left = { type: 'BINARY', operator, left, right };
            }
            
            return left;
        };
        
        const parseMultiplication = () => {
            let left = parsePrimary();
            
            while (match('OPERATOR') && ['*', '/', '%'].includes(peek().value)) {
                const operator = advance().value;
                const right = parsePrimary();
                left = { type: 'BINARY', operator, left, right };
            }
            
            return left;
        };
        
        const parsePrimary = () => {
            if (match('NUMBER')) {
                return { type: 'LITERAL', value: advance().value };
            }
            
            if (match('STRING')) {
                return { type: 'LITERAL', value: advance().value };
            }
            
            if (match('BOOLEAN')) {
                return { type: 'LITERAL', value: advance().value };
            }
            
            if (match('IDENTIFIER')) {
                return { type: 'VARIABLE', name: advance().value };
            }
            
            if (match('OPERATOR', '(')) {
                advance(); // consume '('
                const expr = parseExpression();
                if (!match('OPERATOR', ')')) {
                    throw new Error('Expected closing parenthesis');
                }
                advance(); // consume ')'
                return expr;
            }
            
            const token = peek();
            throw new Error(`Unexpected token in expression: ${token ? token.value : 'EOF'}`);
        };
        
        const parseStatement = () => {
            // Skip any unexpected tokens at the beginning
            while (current < tokens.length && !match('KEYWORD') && !match('IDENTIFIER') && !match('OPERATOR', '{')) {
                current++;
            }
            
            if (current >= tokens.length) {
                throw new Error('Unexpected end of input');
            }
            
            // Variable declaration
            if (match('KEYWORD', 'let')) {
                advance(); // consume 'let'
                
                if (!match('IDENTIFIER')) {
                    throw new Error('Expected variable name after let');
                }
                const name = advance().value;
                
                // Skip type annotation
                if (match('OPERATOR', ':')) {
                    advance(); // consume ':'
                    if (!match('IDENTIFIER') && !match('KEYWORD')) {
                        throw new Error('Expected type after :');
                    }
                    advance(); // consume type
                }
                
                if (!match('ASSIGN')) {
                    throw new Error('Expected = in variable declaration');
                }
                advance(); // consume '='
                
                const value = parseExpression();
                
                if (!match('OPERATOR', ';')) {
                    throw new Error('Expected ; after variable declaration');
                }
                advance(); // consume ';'
                
                return { type: 'DECLARATION', name, value };
            }
            
            // If statement
            if (match('KEYWORD', 'if')) {
                advance(); // consume 'if'
                
                if (!match('OPERATOR', '(')) {
                    throw new Error('Expected ( after if');
                }
                advance(); // consume '('
                
                const condition = parseExpression();
                
                if (!match('OPERATOR', ')')) {
                    throw new Error('Expected ) after if condition');
                }
                advance(); // consume ')'
                
                if (!match('OPERATOR', '{')) {
                    throw new Error('Expected { after if condition');
                }
                advance(); // consume '{'
                
                const thenStatements = [];
                while (!match('OPERATOR', '}') && current < tokens.length) {
                    thenStatements.push(parseStatement());
                }
                
                if (!match('OPERATOR', '}')) {
                    throw new Error('Expected } to close if block');
                }
                advance(); // consume '}'
                
                let elseStatements = [];
                if (match('KEYWORD', 'else')) {
                    advance(); // consume 'else'
                    
                    if (!match('OPERATOR', '{')) {
                        throw new Error('Expected { after else');
                    }
                    advance(); // consume '{'
                    
                    while (!match('OPERATOR', '}') && current < tokens.length) {
                        elseStatements.push(parseStatement());
                    }
                    
                    if (!match('OPERATOR', '}')) {
                        throw new Error('Expected } to close else block');
                    }
                    advance(); // consume '}'
                }
                
                return { type: 'IF', condition, thenStatements, elseStatements };
            }
            
            // While loop
            if (match('KEYWORD', 'while')) {
                advance(); // consume 'while'
                
                if (!match('OPERATOR', '(')) {
                    throw new Error('Expected ( after while');
                }
                advance(); // consume '('
                
                const condition = parseExpression();
                
                if (!match('OPERATOR', ')')) {
                    throw new Error('Expected ) after while condition');
                }
                advance(); // consume ')'
                
                if (!match('OPERATOR', '{')) {
                    throw new Error('Expected { after while condition');
                }
                advance(); // consume '{'
                
                const body = [];
                while (!match('OPERATOR', '}') && current < tokens.length) {
                    body.push(parseStatement());
                }
                
                if (!match('OPERATOR', '}')) {
                    throw new Error('Expected } to close while block');
                }
                advance(); // consume '}'
                
                return { type: 'WHILE', condition, body };
            }
            
            // For loop
            if (match('KEYWORD', 'for')) {
                advance(); // consume 'for'
                
                if (!match('OPERATOR', '(')) {
                    throw new Error('Expected ( after for');
                }
                advance(); // consume '('
                
                // Parse initialization
                let init = null;
                if (!match('OPERATOR', ';')) {
                    init = parseStatement();
                } else {
                    advance(); // consume ';'
                }
                
                // Parse condition
                let condition = null;
                if (!match('OPERATOR', ';')) {
                    condition = parseExpression();
                }
                if (!match('OPERATOR', ';')) {
                    throw new Error('Expected ; after for condition');
                }
                advance(); // consume ';'
                
                // Parse increment
                let increment = null;
                if (!match('OPERATOR', ')')) {
                    if (match('IDENTIFIER')) {
                        const name = advance().value;
                        if (match('ASSIGN')) {
                            advance(); // consume '='
                            const value = parseExpression();
                            increment = { type: 'ASSIGNMENT', name, value };
                        }
                    }
                }
                
                if (!match('OPERATOR', ')')) {
                    throw new Error('Expected ) after for header');
                }
                advance(); // consume ')'
                
                if (!match('OPERATOR', '{')) {
                    throw new Error('Expected { after for header');
                }
                advance(); // consume '{'
                
                const body = [];
                while (!match('OPERATOR', '}') && current < tokens.length) {
                    body.push(parseStatement());
                }
                
                if (!match('OPERATOR', '}')) {
                    throw new Error('Expected } to close for block');
                }
                advance(); // consume '}'
                
                return { type: 'FOR', init, condition, increment, body };
            }
            
            // Do-while loop
            if (match('KEYWORD', 'do')) {
                advance(); // consume 'do'
                
                if (!match('OPERATOR', '{')) {
                    throw new Error('Expected { after do');
                }
                advance(); // consume '{'
                
                const body = [];
                while (!match('OPERATOR', '}') && current < tokens.length) {
                    body.push(parseStatement());
                }
                
                if (!match('OPERATOR', '}')) {
                    throw new Error('Expected } to close do block');
                }
                advance(); // consume '}'
                
                if (!match('KEYWORD', 'while')) {
                    throw new Error('Expected while after do block');
                }
                advance(); // consume 'while'
                
                if (!match('OPERATOR', '(')) {
                    throw new Error('Expected ( after while');
                }
                advance(); // consume '('
                
                const condition = parseExpression();
                
                if (!match('OPERATOR', ')')) {
                    throw new Error('Expected ) after while condition');
                }
                advance(); // consume ')'
                
                if (!match('OPERATOR', ';')) {
                    throw new Error('Expected ; after do-while');
                }
                advance(); // consume ';'
                
                return { type: 'DO_WHILE', condition, body };
            }
            
            // Identifier - could be function call or assignment
            if (match('IDENTIFIER')) {
                const name = advance().value;
                
                // Function call
                if (match('OPERATOR', '(')) {
                    advance(); // consume '('
                    const args = [];
                    
                    if (!match('OPERATOR', ')')) {
                        args.push(parseExpression());
                        while (match('OPERATOR', ',')) {
                            advance(); // consume ','
                            args.push(parseExpression());
                        }
                    }
                    
                    if (!match('OPERATOR', ')')) {
                        throw new Error('Expected ) after function arguments');
                    }
                    advance(); // consume ')'
                    
                    if (!match('OPERATOR', ';')) {
                        throw new Error('Expected ; after function call');
                    }
                    advance(); // consume ';'
                    
                    return { type: 'CALL', name, arguments: args };
                }
                
                // Assignment
                if (match('ASSIGN')) {
                    advance(); // consume '='
                    const value = parseExpression();
                    
                    if (!match('OPERATOR', ';')) {
                        throw new Error('Expected ; after assignment');
                    }
                    advance(); // consume ';'
                    
                    return { type: 'ASSIGNMENT', name, value };
                }
                
                throw new Error(`Unexpected identifier: ${name}`);
            }
            
            const token = peek();
            throw new Error(`Unexpected token: ${token ? token.value : 'EOF'}`);
        };
        
        const statements = [];
        while (current < tokens.length) {
            statements.push(parseStatement());
        }
        
        return { type: 'PROGRAM', statements };
    }
    
    // Evaluator
    evaluate(node) {
        switch (node.type) {
            case 'PROGRAM':
                for (const stmt of node.statements) {
                    this.evaluate(stmt);
                }
                break;
                
            case 'DECLARATION':
                const declValue = this.evaluate(node.value);
                this.variables.set(node.name, declValue);
                break;
                
            case 'ASSIGNMENT':
                const assignValue = this.evaluate(node.value);
                if (!this.variables.has(node.name)) {
                    throw new Error(`Undefined variable: ${node.name}`);
                }
                this.variables.set(node.name, assignValue);
                break;
                
            case 'CALL':
                if (node.name === 'print') {
                    const args = node.arguments.map(arg => this.evaluate(arg));
                    for (const arg of args) {
                        if (typeof arg === 'boolean') {
                            this.currentLine += (arg ? 'true' : 'false');
                        } else {
                            this.currentLine += String(arg);
                        }
                    }
                } else if (node.name === 'println') {
                    const args = node.arguments.map(arg => this.evaluate(arg));
                    for (const arg of args) {
                        if (typeof arg === 'boolean') {
                            this.currentLine += (arg ? 'true' : 'false');
                        } else {
                            this.currentLine += String(arg);
                        }
                    }
                    // Add the current line to output and start a new line
                    this.output.push(this.currentLine);
                    this.currentLine = '';
                } else {
                    throw new Error(`Unknown function: ${node.name}`);
                }
                break;
                
            case 'IF':
                const condition = this.evaluate(node.condition);
                if (this.isTruthy(condition)) {
                    for (const stmt of node.thenStatements) {
                        this.evaluate(stmt);
                    }
                } else {
                    for (const stmt of node.elseStatements) {
                        this.evaluate(stmt);
                    }
                }
                break;
                
            case 'WHILE':
                while (this.isTruthy(this.evaluate(node.condition))) {
                    for (const stmt of node.body) {
                        this.evaluate(stmt);
                    }
                }
                break;
                
            case 'FOR':
                if (node.init) {
                    this.evaluate(node.init);
                }
                
                while (!node.condition || this.isTruthy(this.evaluate(node.condition))) {
                    for (const stmt of node.body) {
                        this.evaluate(stmt);
                    }
                    
                    if (node.increment) {
                        this.evaluate(node.increment);
                    }
                }
                break;
                
            case 'DO_WHILE':
                do {
                    for (const stmt of node.body) {
                        this.evaluate(stmt);
                    }
                } while (this.isTruthy(this.evaluate(node.condition)));
                break;
                
            case 'BINARY':
                const left = this.evaluate(node.left);
                const right = this.evaluate(node.right);
                
                switch (node.operator) {
                    case '+': return left + right;
                    case '-': return left - right;
                    case '*': return left * right;
                    case '/': return Math.floor(left / right); // Integer division
                    case '%': return left % right;
                    case '==': return left === right;
                    case '!=': return left !== right;
                    case '<': return left < right;
                    case '<=': return left <= right;
                    case '>': return left > right;
                    case '>=': return left >= right;
                    default:
                        throw new Error(`Unknown operator: ${node.operator}`);
                }
                
            case 'LITERAL':
                return node.value;
                
            case 'VARIABLE':
                if (!this.variables.has(node.name)) {
                    throw new Error(`Undefined variable: ${node.name}`);
                }
                return this.variables.get(node.name);
                
            default:
                throw new Error(`Unknown node type: ${node.type}`);
        }
    }
    
    isTruthy(value) {
        if (typeof value === 'boolean') return value;
        if (typeof value === 'number') return value !== 0;
        if (typeof value === 'string') return value !== '';
        return false;
    }
    
    execute(source) {
        this.reset();
        
        try {
            const tokens = this.tokenize(source);
            const ast = this.parse(tokens);
            this.evaluate(ast);
            
            // If there's remaining content in currentLine, add it to output
            if (this.currentLine) {
                this.output.push(this.currentLine);
            }
            
            return this.output.join('\n');
        } catch (error) {
            throw new Error(`Runtime Error: ${error.message}`);
        }
    }
}

// Global interpreter instance
const interpreter = new CypescriptInterpreter();

// Main execution function used by the UI
function executeCypescriptCode(source) {
    return interpreter.execute(source);
}
