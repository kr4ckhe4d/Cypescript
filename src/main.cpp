// src/main.cpp - Cypescript Compiler Entry Point
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <filesystem>
#include <chrono>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <set>

#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "AST.h"
#include "CodeGen.h"
#include "Optimizer.h"
#include "Semantic.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace fs = std::filesystem;

#ifndef CYPESCRIPT_VERSION
#define CYPESCRIPT_VERSION "dev"
#endif

// Absolute path of the running cscript binary (empty if it cannot be determined)
fs::path getExecutablePath(const char* argv0) {
#ifdef __APPLE__
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        std::error_code ec;
        fs::path p = fs::canonical(buf, ec);
        if (!ec) return p;
        return fs::path(buf);
    }
#elif defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        std::error_code ec;
        fs::path p = fs::canonical(fs::path(buf), ec);
        if (!ec) return p;
        return fs::path(buf);
    }
#elif defined(__linux__)
    std::error_code ec;
    fs::path p = fs::read_symlink("/proc/self/exe", ec);
    if (!ec) return p;
#endif
    if (argv0 && fs::exists(argv0)) {
        std::error_code ec;
        fs::path p = fs::canonical(argv0, ec);
        if (!ec) return p;
    }
    return fs::path();
}

// Locates the Cypescript runtime to link against. Search order:
//   1. $CYPESCRIPT_HOME/lib/libcypescript.a
//   2. libcypescript.a next to the cscript binary        (build-tree layout)
//   3. ../lib/libcypescript.a relative to the binary     (installed layout)
//   4. ../src/cypescript_stdlib.cpp relative to the binary (repo source fallback)
std::string findRuntimeLibrary(const char* argv0) {
    if (const char* home = std::getenv("CYPESCRIPT_HOME")) {
        fs::path candidate = fs::path(home) / "lib" / "libcypescript.a";
        if (fs::exists(candidate)) return candidate.string();
    }

    fs::path exePath = getExecutablePath(argv0);
    if (!exePath.empty()) {
        fs::path exeDir = exePath.parent_path();
        fs::path candidates[] = {
            exeDir / "libcypescript.a",
            exeDir / ".." / "lib" / "libcypescript.a",
            exeDir / ".." / "src" / "cypescript_stdlib.cpp",
        };
        for (const auto& candidate : candidates) {
            if (fs::exists(candidate)) return candidate.string();
        }
    }

    // Last resort: current working directory is the repo root
    if (fs::exists("src/cypescript_stdlib.cpp")) {
        return "src/cypescript_stdlib.cpp";
    }

    throw std::runtime_error(
        "Could not locate the Cypescript runtime library (libcypescript.a). "
        "Set CYPESCRIPT_HOME to the installation prefix or rebuild with ./build.sh");
}

// Directory holding the bundled Cypescript modules (game.csc and friends), so
// that `import { } from "game";` works from anywhere. Search order mirrors
// findRuntimeLibrary: $CYPESCRIPT_HOME, then paths relative to the binary.
fs::path findModuleDirectory(const char* argv0) {
    if (const char* home = std::getenv("CYPESCRIPT_HOME")) {
        fs::path candidate = fs::path(home) / "lib" / "cypescript";
        if (fs::is_directory(candidate)) return candidate;
    }

    fs::path exePath = getExecutablePath(argv0);
    if (!exePath.empty()) {
        fs::path exeDir = exePath.parent_path();
        fs::path candidates[] = {
            exeDir / ".." / "lib" / "cypescript",   // installed layout
            exeDir / "lib",                         // build tree (build/lib/game.csc)
            exeDir / ".." / "lib",                  // repo checkout
        };
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (fs::is_directory(candidate) && fs::exists(candidate / "game.csc")) {
                fs::path resolved = fs::canonical(candidate, ec);
                return ec ? candidate : resolved;
            }
        }
    }

    // Last resort: running from the repo root
    if (fs::is_directory("lib")) return fs::path("lib");
    return fs::path();
}

// ANSI color codes for better output
namespace Colors {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
}

// C++17 compatible string starts_with function
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

class CompilerOptions {
public:
    std::string inputFile;
    std::string outputFile = "";
    bool verbose = false;
    bool printTokens = false;
    bool printAST = false;
    bool help = false;
    bool version = false;
    bool run = false;
    bool noFold = false;
    // Extra flags appended to the final clang++ link line. Programs can add to
    // these from source with `link "raylib";` — see LinkDirectiveNode.
    std::vector<std::string> linkFlags;

    static CompilerOptions parseArgs(int argc, char** argv) {
        CompilerOptions opts;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--help") {
                opts.help = true;
            } else if (arg == "-V" || arg == "--version") {
                opts.version = true;
            } else if (arg == "-v" || arg == "--verbose") {
                opts.verbose = true;
            } else if (arg == "-r" || arg == "--run") {
                opts.run = true;
            } else if (arg == "--no-fold") {
                opts.noFold = true;
            } else if (arg == "--print-tokens") {
                opts.printTokens = true;
            } else if (arg == "--print-ast") {
                opts.printAST = true;
            } else if (arg == "-o" || arg == "--output") {
                if (i + 1 < argc) {
                    opts.outputFile = argv[++i];
                } else {
                    throw std::runtime_error("Option " + arg + " requires an argument");
                }
            } else if (starts_with(arg, "-l") && arg.size() > 2) {
                opts.linkFlags.push_back(arg);              // -lraylib
            } else if (starts_with(arg, "-L") && arg.size() > 2) {
                opts.linkFlags.push_back(arg);              // -L/opt/homebrew/lib
            } else if (arg == "--framework") {
                if (i + 1 < argc) {
                    opts.linkFlags.push_back("-framework");
                    opts.linkFlags.push_back(argv[++i]);
                } else {
                    throw std::runtime_error("Option " + arg + " requires an argument");
                }
            } else if (arg == "--link-flag") {
                if (i + 1 < argc) {
                    opts.linkFlags.push_back(argv[++i]);
                } else {
                    throw std::runtime_error("Option " + arg + " requires an argument");
                }
            } else if (starts_with(arg, "-")) {
                throw std::runtime_error("Unknown option: " + arg);
            } else if (opts.inputFile.empty()) {
                opts.inputFile = arg;
            } else {
                throw std::runtime_error("Multiple input files not supported");
            }
        }
        
        return opts;
    }
    
    void printHelp() const {
        std::cout << Colors::BOLD << "Cypescript Compiler" << Colors::RESET << "\n";
        std::cout << "A TypeScript-style language compiler built with C++ and LLVM\n\n";
        std::cout << Colors::BOLD << "USAGE:" << Colors::RESET << "\n";
        std::cout << "    cscript [OPTIONS] <input-file>\n\n";
        std::cout << Colors::BOLD << "OPTIONS:" << Colors::RESET << "\n";
        std::cout << "    -h, --help          Show this help message\n";
        std::cout << "    -V, --version       Print compiler version\n";
        std::cout << "    -v, --verbose       Enable verbose output\n";
        std::cout << "    -r, --run           Compile and run the program immediately\n";
        std::cout << "    -o, --output FILE   Specify output executable name\n";
        std::cout << "    --no-fold           Disable AST constant folding / dead-branch elimination\n";
        std::cout << "    --print-tokens      Print lexer tokens\n";
        std::cout << "    --print-ast         Print abstract syntax tree\n\n";
        std::cout << Colors::BOLD << "LINKING (for `declare function` / FFI):" << Colors::RESET << "\n";
        std::cout << "    -l<name>            Link against a library (e.g. -lraylib)\n";
        std::cout << "    -L<dir>             Add a library search path\n";
        std::cout << "    --framework NAME    Link a macOS framework\n";
        std::cout << "    --link-flag FLAG    Pass an arbitrary flag to the linker\n";
        std::cout << "    (a program can also request these itself with `link \"raylib\";`)\n\n";
        std::cout << Colors::BOLD << "EXAMPLES:" << Colors::RESET << "\n";
        std::cout << "    cscript hello.csc\n";
        std::cout << "    cscript -r hello.csc\n";
        std::cout << "    cscript -o my_app hello.csc\n";
        std::cout << "    cscript -r game.csc -lraylib -L/opt/homebrew/lib\n";
    }
};

// Quotes an argument for the shell that std::system() will use. The link line is
// built from source-controlled directives, but they still reach std::system(),
// so nothing goes on that line unquoted.
std::string shellQuote(const std::string& arg) {
#ifdef _WIN32
    // cmd.exe understands double quotes only, and has no escape for a literal
    // quote inside them — so reject rather than silently mangle.
    if (arg.find('"') != std::string::npos) {
        throw std::runtime_error("Link flag contains a quote character, which cannot "
                                 "be passed safely on Windows: " + arg);
    }
    return "\"" + arg + "\"";
#else
    std::string quoted = "'";
    for (char c : arg) {
        if (c == '\'') quoted += "'\\''";
        else quoted += c;
    }
    return quoted + "'";
#endif
}

// Turns the program's own `link ...;` directives into clang++ flags.
std::vector<std::string> collectLinkFlags(const ProgramNode* astRoot) {
    std::vector<std::string> flags;
    if (!astRoot) return flags;
    for (const auto& stmt : astRoot->statements) {
        auto* link = dynamic_cast<const LinkDirectiveNode*>(stmt.get());
        if (!link) continue;
        switch (link->kind) {
            case LinkDirectiveNode::Kind::Library:
                flags.push_back("-l" + link->value);
                break;
            case LinkDirectiveNode::Kind::Framework:
                flags.push_back("-framework");
                flags.push_back(link->value);
                break;
            case LinkDirectiveNode::Kind::SearchPath:
                flags.push_back("-L" + link->value);
                break;
        }
    }
    return flags;
}

class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}
    
    double elapsed() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0; // Return milliseconds
    }
};

std::string readFile(const std::string& filename) {
    if (!fs::exists(filename)) {
        throw std::runtime_error("File does not exist: " + filename);
    }
    
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// --- Module resolution -------------------------------------------------------
// Cypescript modules are inlined at compile time: every
//   import { name, ... } from "./file";
// line is replaced by the (recursively resolved) source of the referenced
// file, once per file. `export` markers are handled by the parser.

std::string resolveImportsImpl(const std::string& source, const fs::path& baseDir,
                               std::set<std::string>& visited);

// Where bare imports like `from "game"` are looked up. Set once from main().
fs::path g_moduleSearchDir;

std::string loadModule(const fs::path& modulePath, std::set<std::string>& visited) {
    fs::path resolved = modulePath;
    if (resolved.extension().empty()) {
        resolved += ".csc";
    }
    if (!fs::exists(resolved)) {
        throw std::runtime_error("Imported module not found: " + resolved.string());
    }
    std::string canonical = fs::canonical(resolved).string();
    if (visited.count(canonical)) {
        return ""; // already inlined (also breaks import cycles)
    }
    visited.insert(canonical);

    std::ifstream file(resolved);
    if (!file) {
        throw std::runtime_error("Could not open imported module: " + resolved.string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return resolveImportsImpl(buffer.str(), resolved.parent_path(), visited);
}

std::string resolveImportsImpl(const std::string& source, const fs::path& baseDir,
                               std::set<std::string>& visited) {
    std::stringstream output;
    std::stringstream input(source);
    std::string line;

    while (std::getline(input, line)) {
        std::string trimmed = line;
        size_t firstChar = trimmed.find_first_not_of(" \t");
        trimmed = (firstChar == std::string::npos) ? "" : trimmed.substr(firstChar);

        if (starts_with(trimmed, "import ") || starts_with(trimmed, "import{")) {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.rfind('"');
            if (firstQuote == std::string::npos || lastQuote <= firstQuote) {
                throw std::runtime_error("Malformed import statement: " + line);
            }
            std::string modulePath = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            // A bare name like "game" resolves against the modules bundled with
            // the compiler; anything relative or absolute resolves as written.
            bool isRelative = starts_with(modulePath, "./") || starts_with(modulePath, "../") ||
                              starts_with(modulePath, "/");
            fs::path resolved = baseDir / modulePath;
            if (!isRelative && !g_moduleSearchDir.empty()) {
                fs::path bundled = g_moduleSearchDir / modulePath;
                if (!fs::exists(resolved) && !fs::exists(resolved.string() + ".csc")) {
                    resolved = bundled;
                }
            }
            output << loadModule(resolved, visited) << "\n";
        } else {
            output << line << "\n";
        }
    }
    return output.str();
}

std::string resolveImports(const std::string& source, const std::string& inputFile) {
    std::set<std::string> visited;
    fs::path inputPath(inputFile);
    if (fs::exists(inputPath)) {
        visited.insert(fs::canonical(inputPath).string());
    }
    return resolveImportsImpl(source, inputPath.parent_path(), visited);
}

void printStageHeader(const std::string& stage, bool verbose) {
    if (verbose) {
        llvm::outs() << Colors::CYAN << "=== " << stage << " ===" << Colors::RESET << "\n";
    }
}

void printSuccess(const std::string& message, bool verbose) {
    if (verbose) {
        llvm::outs() << Colors::GREEN << "✓ " << message << Colors::RESET << "\n";
    }
}

void printError(const std::string& message) {
    llvm::errs() << Colors::RED << "✗ Error: " << message << Colors::RESET << "\n";
}

void printWarning(const std::string& message) {
    llvm::errs() << Colors::YELLOW << "⚠ Warning: " << message << Colors::RESET << "\n";
}

int main(int argc, char** argv) {
    Timer totalTimer;
    
    try {
        // Parse command line arguments
        CompilerOptions opts = CompilerOptions::parseArgs(argc, argv);
        
        if (opts.help) {
            opts.printHelp();
            return 0;
        }

        if (opts.version) {
            std::cout << "cypescript " << CYPESCRIPT_VERSION << "\n";
            return 0;
        }
        
        if (opts.inputFile.empty()) {
            printError("No input file provided");
            std::cout << "Use --help for usage information\n";
            return 1;
        }
        
        if (opts.verbose) {
            llvm::outs() << Colors::BOLD << "Cypescript Compiler v1.0.0" << Colors::RESET << "\n";
            llvm::outs() << "Input file: " << opts.inputFile << "\n";
            llvm::outs() << "Output file: " << opts.outputFile << "\n\n";
        }
        
        // Read source code
        printStageHeader("Reading Source", opts.verbose);
        Timer readTimer;
        std::string sourceCode = readFile(opts.inputFile);
        printSuccess("Source code read (" + std::to_string(readTimer.elapsed()) + "ms)", opts.verbose);

        // Module Resolution (import/export inlining)
        printStageHeader("Module Resolution", opts.verbose);
        Timer importTimer;
        g_moduleSearchDir = findModuleDirectory(argv[0]);
        sourceCode = resolveImports(sourceCode, opts.inputFile);
        printSuccess("Imports resolved (" + std::to_string(importTimer.elapsed()) + "ms)", opts.verbose);
        
        // Lexical Analysis
        printStageHeader("Lexical Analysis", opts.verbose);
        Timer lexTimer;
        Lexer lexer(sourceCode);
        std::vector<Token> tokens;
        Token token;
        
        do {
            token = lexer.getNextToken();
            tokens.push_back(token);
            
            if (opts.printTokens) {
                llvm::outs() << "Token { Type: " << Colors::YELLOW << tokenTypeToString(token.type) 
                           << Colors::RESET << ", Value: \"" << Colors::GREEN << token.value 
                           << Colors::RESET << "\" }\n";
            }
        } while (token.type != TOK_EOF);
        
        printSuccess("Lexical analysis complete (" + std::to_string(lexTimer.elapsed()) + "ms, " 
                    + std::to_string(tokens.size()) + " tokens)", opts.verbose);
        
        // Syntax Analysis (Parsing)
        printStageHeader("Syntax Analysis", opts.verbose);
        Timer parseTimer;
        Parser parser(tokens);
        std::unique_ptr<ProgramNode> astRoot = parser.parse();
        
        if (!astRoot) {
            printError("Parsing failed");
            return 1;
        }
        
        printSuccess("Syntax analysis complete (" + std::to_string(parseTimer.elapsed()) + "ms)", opts.verbose);

        // Semantic Analysis (undefined variables, const reassignment, break/continue
        // placement, function arity) — errors carry line/column positions
        printStageHeader("Semantic Analysis", opts.verbose);
        Timer semanticTimer;
        try {
            SemanticAnalyzer analyzer;
            analyzer.analyze(astRoot.get());
        } catch (const std::runtime_error& e) {
            printError(e.what());
            return 1;
        }
        printSuccess("Semantic analysis complete (" + std::to_string(semanticTimer.elapsed()) + "ms)", opts.verbose);

        // AST Optimization (constant folding + dead-branch elimination)
        if (!opts.noFold) {
            printStageHeader("AST Optimization", opts.verbose);
            Timer optTimer;
            ASTOptimizer optimizer;
            ASTOptimizer::Stats optStats = optimizer.optimize(astRoot.get());
            printSuccess("Constant folding complete (" + std::to_string(optTimer.elapsed()) + "ms, "
                        + std::to_string(optStats.foldedExpressions) + " expressions folded, "
                        + std::to_string(optStats.eliminatedBranches) + " dead branches removed)", opts.verbose);
        }

        if (opts.printAST || opts.verbose) {
            llvm::outs() << "\n" << Colors::MAGENTA << "=== Abstract Syntax Tree ===" << Colors::RESET << "\n";
            astRoot->printNode(llvm::outs());
            llvm::outs() << Colors::MAGENTA << "=== End of AST ===" << Colors::RESET << "\n\n";
        }
        
        // Code Generation
        printStageHeader("Code Generation", opts.verbose);
        Timer codegenTimer;
        llvm::LLVMContext context;
        CodeGen codeGenerator(context);
        llvm::Module* module = codeGenerator.generate(astRoot.get());
        
        if (!module) {
            printError("Code generation failed");
            return 1;
        }
        
        printSuccess("Code generation complete (" + std::to_string(codegenTimer.elapsed()) + "ms)", opts.verbose);
        
        // Write LLVM IR
        printStageHeader("Writing Output", opts.verbose);
        Timer writeTimer;
        
        std::string irFile = opts.outputFile;
        bool isExecutable = true;
        
        if (irFile.empty()) {
            irFile = "temp_output.ll";
        } else if (irFile.size() > 3 && irFile.substr(irFile.size() - 3) == ".ll") {
            isExecutable = false;
        } else {
            // Specified an executable name, use a temp .ll file
            irFile += ".ll";
        }

        std::error_code EC;
        llvm::raw_fd_ostream dest(irFile, EC, llvm::sys::fs::OF_None);
        
        if (EC) {
            printError("Could not open IR file '" + irFile + "': " + EC.message());
            return 1;
        }
        
        module->print(dest, nullptr);
        dest.flush();
        printSuccess("LLVM IR written to " + irFile + " (" + std::to_string(writeTimer.elapsed()) + "ms)", opts.verbose);
        
        std::string executableName = opts.outputFile;
        if (executableName.empty()) {
            // Default executable name: input filename without extension
            fs::path p(opts.inputFile);
            executableName = p.stem().string();
        }

        if (isExecutable) {
            printStageHeader("Compiling to Executable", opts.verbose);
            Timer compileTimer;
            
            // Locate the runtime: precompiled libcypescript.a next to the binary
            // (or CYPESCRIPT_HOME), falling back to the stdlib source in the repo
            std::string stdlibPath = findRuntimeLibrary(argv[0]);

            std::string compileCmd = "clang++ -O2 " + irFile + " \"" + stdlibPath + "\" -o " + executableName + " -std=c++17";

            // Libraries the program asked for itself via `link "raylib";`, followed
            // by anything passed on the command line (which therefore wins).
            std::vector<std::string> sourceLinkFlags = collectLinkFlags(astRoot.get());
            if (!sourceLinkFlags.empty() || !opts.linkFlags.empty()) {
                // Our own lib/ dir, so `link "cypescript_game";` resolves without
                // the program needing to know where cscript was installed.
                fs::path runtimeDir = fs::path(stdlibPath).parent_path();
                if (!runtimeDir.empty()) {
                    compileCmd += " " + shellQuote("-L" + runtimeDir.string());
                }
            }
            for (const std::string& flag : sourceLinkFlags) {
                compileCmd += " " + shellQuote(flag);
            }
            for (const std::string& flag : opts.linkFlags) {
                compileCmd += " " + shellQuote(flag);
            }

            if (opts.verbose) {
                llvm::outs() << "Running: " << Colors::CYAN << compileCmd << Colors::RESET << "\n";
            }
            
            int result = std::system(compileCmd.c_str());
            if (result != 0) {
                printError("Failed to compile executable");
                return 1;
            }
            
            // Clean up temp IR file if it wasn't requested
            if (opts.outputFile != irFile) {
                fs::remove(irFile);
            }
            
            printSuccess("Executable created: " + executableName + " (" + std::to_string(compileTimer.elapsed()) + "ms)", opts.verbose);
        }

        // Print compilation summary
        if (opts.verbose) {
            llvm::outs() << "\n" << Colors::BOLD << "=== Compilation Summary ===" << Colors::RESET << "\n";
            llvm::outs() << "Total time: " << Colors::GREEN << totalTimer.elapsed() << "ms" << Colors::RESET << "\n";
            llvm::outs() << "Input: " << opts.inputFile << " (" << sourceCode.size() << " bytes)\n";
            if (isExecutable) {
                llvm::outs() << "Executable: " << executableName << "\n";
            } else {
                llvm::outs() << "Output IR: " << irFile << "\n";
            }
            llvm::outs() << "Status: " << Colors::GREEN << "SUCCESS" << Colors::RESET << "\n\n";
        } else {
            if (isExecutable) {
                llvm::outs() << Colors::GREEN << "✓ Compiled to: " << Colors::BOLD << executableName << Colors::RESET << "\n";
            } else {
                llvm::outs() << Colors::GREEN << "✓ IR written to: " << irFile << Colors::RESET << "\n";
            }
        }
        
        if (opts.run && isExecutable) {
            printStageHeader("Running Program", opts.verbose);
            std::string runCmd = "./" + executableName;
            if (opts.verbose) {
                llvm::outs() << "Executing: " << Colors::CYAN << runCmd << Colors::RESET << "\n";
                llvm::outs() << "----------------------------------------\n";
            }
            
            int result = std::system(runCmd.c_str());
            
            if (opts.verbose) {
                llvm::outs() << "----------------------------------------\n";
                if (result == 0) {
                    printSuccess("Program exited successfully", true);
                } else {
                    printError("Program exited with code " + std::to_string(result));
                }
            }
            
            // Optional: remove executable after running if it was just a temporary run
            // fs::remove(executableName);
        } else if (!opts.verbose && !isExecutable) {
            // Print next steps for IR mode only if not verbose
            llvm::outs() << Colors::BOLD << "Next steps:" << Colors::RESET << "\n";
            llvm::outs() << "1. Compile to executable: " << Colors::CYAN << "clang++ " << irFile 
                       << " src/cypescript_stdlib.cpp -o " << executableName << Colors::RESET << "\n";
            llvm::outs() << "2. Run program:           " << Colors::CYAN << "./" << executableName << Colors::RESET << "\n";
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        printError(e.what());
        return 1;
    }
}
