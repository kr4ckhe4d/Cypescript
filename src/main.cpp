// src/main.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "AST.h"
#include "CodeGen.h"

int main(int argc, char **argv)
{
    llvm::outs() << "MyLangCompiler starting...\n";

    if (argc < 2)
    {
        llvm::errs() << "Error: No input file provided.\n";
        return 1;
    }

    std::string inputFileName = argv[1];
    llvm::outs() << "Input file: " << inputFileName << "\n";

    std::ifstream inputFile(inputFileName);
    if (!inputFile)
    {
        llvm::errs() << "Error: Could not open file: " << inputFileName << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string sourceCode = buffer.str();
    inputFile.close();

    llvm::outs() << "Source code read successfully.\n";
    // llvm::outs() << "Source Code:\n<<<<\n" << sourceCode << "\n>>>>\n"; // Optional: print source

    // --- Lexing Stage ---
    llvm::outs() << "--- Lexing ---\n";
    Lexer lexer(sourceCode);
    std::vector<Token> tokens;
    Token token;
    do
    {
        token = lexer.getNextToken();
        tokens.push_back(token);
        // Print tokens for debugging
        llvm::outs() << "Token { Type: " << tokenTypeToString(token.type)
                     << ", Value: \"" << token.value << "\" }\n";
    } while (token.type != TOK_EOF);
    llvm::outs() << "--- Lexing Complete ---\n";

    // --- Parsing Stage ---
    llvm::outs() << "--- Parsing ---\n";
    Parser parser(tokens);
    std::unique_ptr<ProgramNode> astRoot = parser.parse();

    if (astRoot)
    {
        llvm::outs() << "--- Parsing Complete (AST generated) ---\n";
        // --- Print the AST ---
        llvm::outs() << "\n--- Abstract Syntax Tree (AST) ---\n";
        astRoot->printNode(llvm::outs()); // Call the printNode method
        llvm::outs() << "--- End of AST ---\n\n";
        // --- End Print the AST ---
    }
    else
    {
        llvm::errs() << "--- Parsing Failed ---\n";
        return 1;
    }

    // --- Code Generation Stage ---
    llvm::outs() << "--- Code Generation ---\n";
    llvm::LLVMContext context;
    CodeGen codeGenerator(context);
    llvm::Module *module = codeGenerator.generate(astRoot.get());

    if (module)
    {
        llvm::outs() << "--- Code Generation Complete (LLVM IR generated) ---\n";
        std::string outputFilename = "output.ll";
        std::error_code EC;
        llvm::raw_fd_ostream dest(outputFilename, EC, llvm::sys::fs::OF_None);

        if (EC)
        {
            llvm::errs() << "Could not open file: " << EC.message() << "\n";
            return 1;
        }
        llvm::outs() << "Writing LLVM IR to " << outputFilename << "...\n";
        module->print(dest, nullptr);
        dest.flush();
        llvm::outs() << "LLVM IR written successfully.\n";
    }
    else
    {
        llvm::errs() << "--- Code Generation Failed ---\n";
        return 1;
    }

    llvm::outs() << "MyLangCompiler finished.\n";
    llvm::outs() << "\nTo compile the generated IR, run:\n";
    llvm::outs() << "  llc -filetype=obj -relocation-model=pic output.ll -o output.o\n"; // Added flags
    llvm::outs() << "  clang output.o -o my_program\n";
    llvm::outs() << "Then run the program:\n";
    llvm::outs() << "  ./my_program\n";

    return 0;
}
