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

#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "AST.h"
#include "CodeGen.h"

namespace fs = std::filesystem;

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
    std::string outputFile = "output.ll";
    bool verbose = false;
    bool printTokens = false;
    bool printAST = false;
    bool help = false;
    
    static CompilerOptions parseArgs(int argc, char** argv) {
        CompilerOptions opts;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--help") {
                opts.help = true;
            } else if (arg == "-v" || arg == "--verbose") {
                opts.verbose = true;
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
        std::cout << "    -v, --verbose       Enable verbose output\n";
        std::cout << "    -o, --output FILE   Specify output file (default: output.ll)\n";
        std::cout << "    --print-tokens      Print lexer tokens\n";
        std::cout << "    --print-ast         Print abstract syntax tree\n\n";
        std::cout << Colors::BOLD << "EXAMPLES:" << Colors::RESET << "\n";
        std::cout << "    cscript hello.csc\n";
        std::cout << "    cscript -v --print-ast hello.csc\n";
        std::cout << "    cscript -o my_output.ll hello.csc\n";
    }
};

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
        std::error_code EC;
        llvm::raw_fd_ostream dest(opts.outputFile, EC, llvm::sys::fs::OF_None);
        
        if (EC) {
            printError("Could not open output file '" + opts.outputFile + "': " + EC.message());
            return 1;
        }
        
        module->print(dest, nullptr);
        dest.flush();
        printSuccess("LLVM IR written to " + opts.outputFile + " (" + std::to_string(writeTimer.elapsed()) + "ms)", opts.verbose);
        
        // Print compilation summary
        if (opts.verbose) {
            llvm::outs() << "\n" << Colors::BOLD << "=== Compilation Summary ===" << Colors::RESET << "\n";
            llvm::outs() << "Total time: " << Colors::GREEN << totalTimer.elapsed() << "ms" << Colors::RESET << "\n";
            llvm::outs() << "Input: " << opts.inputFile << " (" << sourceCode.size() << " bytes)\n";
            llvm::outs() << "Output: " << opts.outputFile << "\n";
            llvm::outs() << "Status: " << Colors::GREEN << "SUCCESS" << Colors::RESET << "\n\n";
        } else {
            llvm::outs() << Colors::GREEN << "✓ Compilation successful" << Colors::RESET << "\n";
        }
        
        // Print next steps
        llvm::outs() << Colors::BOLD << "Next steps:" << Colors::RESET << "\n";
        llvm::outs() << "1. Compile to object: " << Colors::CYAN << "llc -filetype=obj -relocation-model=pic " 
                   << opts.outputFile << " -o output.o" << Colors::RESET << "\n";
        llvm::outs() << "2. Link executable:   " << Colors::CYAN << "clang output.o -o my_program" << Colors::RESET << "\n";
        llvm::outs() << "3. Run program:       " << Colors::CYAN << "./my_program" << Colors::RESET << "\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        printError(e.what());
        return 1;
    }
}
