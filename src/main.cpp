// src/main.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory> // Include for unique_ptr
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h" // Include for file output stream
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "Lexer.h"
#include "Token.h"
#include "Parser.h" // Include Parser
#include "AST.h"    // Include AST
#include "CodeGen.h"

int main(int argc, char **argv) {
    llvm::outs() << "MyLangCompiler starting...\n";

    if (argc < 2) {
        llvm::errs() << "Error: No input file provided.\n";
        return 1;
    }

    std::string inputFileName = argv[1];
    llvm::outs() << "Input file: " << inputFileName << "\n";

    // --- Read Input File ---
    std::ifstream inputFile(inputFileName);
    if (!inputFile) {
        llvm::errs() << "Error: Could not open file: " << inputFileName << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string sourceCode = buffer.str();
    inputFile.close();

    llvm::outs() << "Source code read successfully.\n";

    // --- Lexing Stage ---
    llvm::outs() << "--- Lexing ---\n";
    Lexer lexer(sourceCode);
    std::vector<Token> tokens; // Store tokens
    Token token;
    do {
        token = lexer.getNextToken();
        tokens.push_back(token);
        // Optional: Can comment out the token printing for cleaner output now
        // llvm::outs() << "Token { Type: " << tokenTypeToString(token.type)
        //              << ", Value: \"" << token.value << "\" }\n";
    } while (token.type != TOK_EOF);
    llvm::outs() << "--- Lexing Complete ---\n";

    // --- Parsing Stage ---
    llvm::outs() << "--- Parsing ---\n";
    Parser parser(tokens);
    std::unique_ptr<ProgramNode> astRoot = parser.parse(); // Run the parser

    if (astRoot) {
        llvm::outs() << "--- Parsing Complete (AST generated) ---\n";
        // Later: We can add code here to print/visualize the AST
        // Later: Pass astRoot to the Semantic Analyzer or Code Generator
    } else {
        llvm::errs() << "--- Parsing Failed ---\n";
        return 1; // Exit if parsing failed
    }

    // --- Code Generation Stage ---
    llvm::outs() << "--- Code Generation ---\n";
    llvm::LLVMContext context; // Create LLVM Context
    CodeGen codeGenerator(context); // Create Code Generator

    // Generate LLVM IR Module from AST
    llvm::Module* module = codeGenerator.generate(astRoot.get());

    if (module) {
        llvm::outs() << "--- Code Generation Complete (LLVM IR generated) ---\n";
        std::string outputFilename = "output.ll";
        std::error_code EC;
        llvm::raw_fd_ostream dest(outputFilename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message() << "\n";
            return 1;
        }
        llvm::outs() << "Writing LLVM IR to " << outputFilename << "...\n";
        module->print(dest, nullptr); // Print IR to the file stream
        dest.flush(); // Ensure it's written
        llvm::outs() << "LLVM IR written successfully.\n";

    } else {
        llvm::errs() << "--- Code Generation Failed ---\n";
        return 1; // Exit if code generation failed
    }

    llvm::outs() << "MyLangCompiler finished.\n";
    llvm::outs() << "\nTo compile the generated IR, run:\n";
    llvm::outs() << "  llc output.ll -o output.o\n";
    llvm::outs() << "  clang output.o -o my_program\n"; // or gcc output.o -o my_program
    llvm::outs() << "Then run the program:\n";
    llvm::outs() << "  ./my_program\n";

    return 0;
}