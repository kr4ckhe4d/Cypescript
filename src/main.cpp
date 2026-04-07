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
    std::string outputFile = "";
    bool verbose = false;
    bool printTokens = false;
    bool printAST = false;
    bool help = false;
    bool run = false;
    
    static CompilerOptions parseArgs(int argc, char** argv) {
        CompilerOptions opts;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--help") {
                opts.help = true;
            } else if (arg == "-v" || arg == "--verbose") {
                opts.verbose = true;
            } else if (arg == "-r" || arg == "--run") {
                opts.run = true;
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
        std::cout << "    -r, --run           Compile and run the program immediately\n";
        std::cout << "    -o, --output FILE   Specify output executable name\n";
        std::cout << "    --print-tokens      Print lexer tokens\n";
        std::cout << "    --print-ast         Print abstract syntax tree\n\n";
        std::cout << Colors::BOLD << "EXAMPLES:" << Colors::RESET << "\n";
        std::cout << "    cscript hello.csc\n";
        std::cout << "    cscript -r hello.csc\n";
        std::cout << "    cscript -o my_app hello.csc\n";
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
            
            // Find stdlib path - assume it's in the same directory as src/
            // For a distributed app, this would be in a fixed location
            std::string stdlibPath = "src/cypescript_stdlib.cpp";
            
            std::string compileCmd = "clang++ " + irFile + " " + stdlibPath + " -o " + executableName + " -std=c++17";
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
