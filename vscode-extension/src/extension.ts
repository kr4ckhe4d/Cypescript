import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { exec } from 'child_process';

export function activate(context: vscode.ExtensionContext) {
    console.log('Cypescript extension is now active!');

    // Register commands
    const compileCommand = vscode.commands.registerCommand('cypescript.compile', () => {
        compileCurrentFile(false, false);
    });

    const compileAndRunCommand = vscode.commands.registerCommand('cypescript.compileAndRun', () => {
        compileCurrentFile(true, false);
    });

    const compileWithCppCommand = vscode.commands.registerCommand('cypescript.compileWithCpp', () => {
        compileCurrentFile(true, true);
    });

    const openDocsCommand = vscode.commands.registerCommand('cypescript.openDocs', () => {
        openInteractiveDocs();
    });

    // Register language features
    const completionProvider = vscode.languages.registerCompletionItemProvider(
        'cypescript',
        new CypescriptCompletionProvider(),
        '.'
    );

    const hoverProvider = vscode.languages.registerHoverProvider(
        'cypescript',
        new CypescriptHoverProvider()
    );

    const diagnosticCollection = vscode.languages.createDiagnosticCollection('cypescript');
    context.subscriptions.push(diagnosticCollection);

    // Register document change listener for diagnostics
    const documentChangeListener = vscode.workspace.onDidChangeTextDocument((event) => {
        if (event.document.languageId === 'cypescript') {
            updateDiagnostics(event.document, diagnosticCollection);
        }
    });

    const documentOpenListener = vscode.workspace.onDidOpenTextDocument((document) => {
        if (document.languageId === 'cypescript') {
            updateDiagnostics(document, diagnosticCollection);
        }
    });

    context.subscriptions.push(
        compileCommand,
        compileAndRunCommand,
        compileWithCppCommand,
        openDocsCommand,
        completionProvider,
        hoverProvider,
        documentChangeListener,
        documentOpenListener
    );

    // Show welcome message for .csc files
    vscode.window.onDidChangeActiveTextEditor((editor) => {
        if (editor && editor.document.languageId === 'cypescript') {
            showWelcomeMessage();
        }
    });
}

function compileCurrentFile(run: boolean, useCpp: boolean) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'cypescript') {
        vscode.window.showErrorMessage('Please open a Cypescript (.csc) file first.');
        return;
    }

    const document = editor.document;
    const filePath = document.fileName;
    const workspaceFolder = vscode.workspace.getWorkspaceFolder(document.uri);
    
    if (!workspaceFolder) {
        vscode.window.showErrorMessage('Please open a workspace folder.');
        return;
    }

    // Save the file first
    document.save().then(() => {
        const terminal = vscode.window.createTerminal('Cypescript');
        terminal.show();

        const relativePath = path.relative(workspaceFolder.uri.fsPath, filePath);
        const baseName = path.basename(filePath, '.csc');

        if (useCpp) {
            terminal.sendText(`./compile-with-cpp.sh "${relativePath}" "${baseName}"`);
            if (run) {
                terminal.sendText(`./${baseName}`);
            }
        } else if (run) {
            terminal.sendText(`./compile-run.sh "${relativePath}"`);
        } else {
            terminal.sendText(`./build/cscript "${relativePath}"`);
        }
    });
}

function openInteractiveDocs() {
    const terminal = vscode.window.createTerminal('Cypescript Docs');
    terminal.show();
    terminal.sendText('./launch-docs.sh');
    
    vscode.window.showInformationMessage(
        'Opening Cypescript interactive documentation in your browser...',
        'OK'
    );
}

function showWelcomeMessage() {
    const config = vscode.workspace.getConfiguration('cypescript');
    if (config.get('showWelcomeMessage', true)) {
        vscode.window.showInformationMessage(
            'Welcome to Cypescript! Use Ctrl+F5 to compile and run, or Ctrl+Shift+F5 for C++ integration.',
            'Don\'t show again',
            'Open Docs'
        ).then(selection => {
            if (selection === 'Don\'t show again') {
                config.update('showWelcomeMessage', false, vscode.ConfigurationTarget.Global);
            } else if (selection === 'Open Docs') {
                openInteractiveDocs();
            }
        });
    }
}

class CypescriptCompletionProvider implements vscode.CompletionItemProvider {
    provideCompletionItems(
        document: vscode.TextDocument,
        position: vscode.Position
    ): vscode.CompletionItem[] {
        const completions: vscode.CompletionItem[] = [];

        // Keywords
        const keywords = ['let', 'const', 'if', 'else', 'while', 'for', 'do', 'break', 'continue', 'return'];
        keywords.forEach(keyword => {
            const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            item.detail = 'Cypescript keyword';
            completions.push(item);
        });

        // Types
        const types = ['string', 'i32', 'f64', 'boolean', 'i32[]', 'string[]', 'f64[]', 'boolean[]'];
        types.forEach(type => {
            const item = new vscode.CompletionItem(type, vscode.CompletionItemKind.TypeParameter);
            item.detail = 'Cypescript type';
            completions.push(item);
        });

        // Built-in functions
        const builtinFunctions = [
            { name: 'print', detail: 'Print without newline', params: '(value)' },
            { name: 'println', detail: 'Print with newline', params: '(value)' }
        ];
        builtinFunctions.forEach(func => {
            const item = new vscode.CompletionItem(func.name, vscode.CompletionItemKind.Function);
            item.detail = func.detail;
            item.insertText = new vscode.SnippetString(`${func.name}($1);`);
            completions.push(item);
        });

        // C++ Integration functions
        const cppFunctions = [
            { name: 'string_reverse', detail: 'Reverse a string', params: '(str: string): string' },
            { name: 'string_upper', detail: 'Convert to uppercase', params: '(str: string): string' },
            { name: 'string_lower', detail: 'Convert to lowercase', params: '(str: string): string' },
            { name: 'string_length', detail: 'Get string length', params: '(str: string): i32' },
            { name: 'string_substring', detail: 'Extract substring', params: '(str: string, start: i32, length: i32): string' },
            { name: 'string_find', detail: 'Find substring position', params: '(str: string, substr: string): i32' },
            { name: 'string_concat', detail: 'Concatenate strings', params: '(str1: string, str2: string): string' },
            { name: 'array_sum_i32', detail: 'Sum array elements', params: '(arr: i32[], size: i32): i32' },
            { name: 'array_max_i32', detail: 'Find maximum element', params: '(arr: i32[], size: i32): i32' },
            { name: 'array_min_i32', detail: 'Find minimum element', params: '(arr: i32[], size: i32): i32' },
            { name: 'file_read', detail: 'Read file contents', params: '(filename: string): string' },
            { name: 'file_write', detail: 'Write to file', params: '(filename: string, content: string): i32' },
            { name: 'file_exists', detail: 'Check if file exists', params: '(filename: string): i32' },
            { name: 'random_seed', detail: 'Seed random generator', params: '(seed: i32): void' },
            { name: 'random_int', detail: 'Generate random integer', params: '(min: i32, max: i32): i32' },
            { name: 'random_double', detail: 'Generate random double', params: '(): f64' },
            { name: 'math_abs_i32', detail: 'Absolute value', params: '(x: i32): i32' }
        ];
        cppFunctions.forEach(func => {
            const item = new vscode.CompletionItem(func.name, vscode.CompletionItemKind.Function);
            item.detail = `${func.detail} - C++ Integration`;
            item.documentation = new vscode.MarkdownString(`**${func.name}**${func.params}\\n\\n${func.detail}\\n\\n*Requires C++ integration compilation*`);
            item.insertText = new vscode.SnippetString(`${func.name}($1)`);
            completions.push(item);
        });

        return completions;
    }
}

class CypescriptHoverProvider implements vscode.HoverProvider {
    provideHover(
        document: vscode.TextDocument,
        position: vscode.Position
    ): vscode.Hover | undefined {
        const range = document.getWordRangeAtPosition(position);
        if (!range) return undefined;

        const word = document.getText(range);

        // Function documentation
        const functionDocs: { [key: string]: string } = {
            'print': '**print(value)** - Print value without newline\\n\\nBuilt-in function for output.',
            'println': '**println(value)** - Print value with newline\\n\\nBuilt-in function for output.',
            'string_reverse': '**string_reverse(str: string): string** - Reverse a string\\n\\nC++ integration function. Requires compilation with `./compile-with-cpp.sh`',
            'string_upper': '**string_upper(str: string): string** - Convert string to uppercase\\n\\nC++ integration function.',
            'string_lower': '**string_lower(str: string): string** - Convert string to lowercase\\n\\nC++ integration function.',
            'string_length': '**string_length(str: string): i32** - Get string length\\n\\nC++ integration function.',
            'array_sum_i32': '**array_sum_i32(arr: i32[], size: i32): i32** - Sum array elements\\n\\nC++ integration function.',
            'array_max_i32': '**array_max_i32(arr: i32[], size: i32): i32** - Find maximum element\\n\\nC++ integration function.',
            'array_min_i32': '**array_min_i32(arr: i32[], size: i32): i32** - Find minimum element\\n\\nC++ integration function.',
            'file_read': '**file_read(filename: string): string** - Read file contents\\n\\nC++ integration function.',
            'file_write': '**file_write(filename: string, content: string): i32** - Write to file\\n\\nC++ integration function. Returns 1 on success, 0 on failure.',
            'random_int': '**random_int(min: i32, max: i32): i32** - Generate random integer\\n\\nC++ integration function. Returns random number between min and max (inclusive).'
        };

        if (functionDocs[word]) {
            return new vscode.Hover(new vscode.MarkdownString(functionDocs[word]));
        }

        // Type documentation
        const typeDocs: { [key: string]: string } = {
            'string': '**string** - Text data type\\n\\nUsed for storing text values.',
            'i32': '**i32** - 32-bit signed integer\\n\\nUsed for storing whole numbers.',
            'f64': '**f64** - 64-bit floating-point number\\n\\nUsed for storing decimal numbers. (Browser interpreter only)',
            'boolean': '**boolean** - Boolean data type\\n\\nUsed for storing true/false values. (Browser interpreter only)'
        };

        if (typeDocs[word]) {
            return new vscode.Hover(new vscode.MarkdownString(typeDocs[word]));
        }

        return undefined;
    }
}

function updateDiagnostics(document: vscode.TextDocument, collection: vscode.DiagnosticCollection) {
    const config = vscode.workspace.getConfiguration('cypescript');
    if (!config.get('enableDiagnostics', true)) {
        return;
    }

    const diagnostics: vscode.Diagnostic[] = [];
    const text = document.getText();
    const lines = text.split('\\n');

    lines.forEach((line, lineIndex) => {
        // Check for common issues
        
        // Missing semicolon (simple heuristic)
        if (line.trim().match(/^(let|print|println).*[^;{]$/)) {
            const diagnostic = new vscode.Diagnostic(
                new vscode.Range(lineIndex, line.length - 1, lineIndex, line.length),
                'Missing semicolon',
                vscode.DiagnosticSeverity.Warning
            );
            diagnostic.code = 'missing-semicolon';
            diagnostics.push(diagnostic);
        }

        // Unsupported features in native compiler
        if (line.includes('true') || line.includes('false')) {
            const match = line.match(/(true|false)/);
            if (match) {
                const startPos = line.indexOf(match[0]);
                const diagnostic = new vscode.Diagnostic(
                    new vscode.Range(lineIndex, startPos, lineIndex, startPos + match[0].length),
                    'Boolean literals are not supported in native compiler. Use browser interpreter or integers (1/0).',
                    vscode.DiagnosticSeverity.Information
                );
                diagnostic.code = 'boolean-literal';
                diagnostics.push(diagnostic);
            }
        }

        // Object literals
        if (line.includes('{') && line.includes(':') && !line.includes('//')) {
            const diagnostic = new vscode.Diagnostic(
                new vscode.Range(lineIndex, 0, lineIndex, line.length),
                'Object literals are not supported in native compiler. Use browser interpreter.',
                vscode.DiagnosticSeverity.Information
            );
            diagnostic.code = 'object-literal';
            diagnostics.push(diagnostic);
        }

        // Float literals
        const floatMatch = line.match(/\\b\\d+\\.\\d+\\b/);
        if (floatMatch) {
            const startPos = line.indexOf(floatMatch[0]);
            const diagnostic = new vscode.Diagnostic(
                new vscode.Range(lineIndex, startPos, lineIndex, startPos + floatMatch[0].length),
                'Float literals are not supported in native compiler. Use browser interpreter.',
                vscode.DiagnosticSeverity.Information
            );
            diagnostic.code = 'float-literal';
            diagnostics.push(diagnostic);
        }
    });

    collection.set(document.uri, diagnostics);
}

export function deactivate() {
    console.log('Cypescript extension deactivated');
}
