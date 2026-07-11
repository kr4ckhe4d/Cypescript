# EP03 — Parsing: Building the Tree

**Length target:** 26–30 min · **Repo anchor:** `src/Parser.cpp`, `src/AST.h`
**Goal:** recursive descent parser producing an AST for declarations,
expressions with correct precedence, and function calls. Viewer can
`--print-ast` a real program.

---

## [COLD OPEN] (0:00–0:40)

[SCREEN: terminal, `--print-ast` on `let result = 5 + 3 * 2;`]

```
VariableDeclarationNode: let result : auto =
  BinaryExpressionNode: +
    Left:  IntegerLiteralNode: 5
    Right: BinaryExpressionNode: *
      Left:  IntegerLiteralNode: 3
      Right: IntegerLiteralNode: 2
```

NARRATION:
> Look closely at this tree. Nobody told the computer that multiplication
> binds tighter than addition — and yet the `*` ended up *deeper* in the tree
> than the `+`. That's not luck. That's the most elegant trick in compiler
> construction, and by the end of this video you'll be able to write it from
> memory.

---

## Segment 1 — What an AST is (0:40–5:00)

[B-ROLL / DIAGRAM: tokens on a flat line morphing into a tree]

NARRATION:
> The lexer gave us a flat list of words. But programs aren't flat — an if
> statement *contains* statements, an addition *contains* two operands. The
> Abstract Syntax Tree makes that containment explicit. "Abstract" because we
> throw away what doesn't matter: parentheses and semicolons *shape* the
> tree, then vanish.

[SHOW: `src/AST.h` — the node hierarchy, condensed]
```cpp
class ASTNode        { public: int line = 0, column = 0; virtual ~ASTNode(); };
class ExpressionNode : public ASTNode {};   // has a value
class StatementNode  : public ASTNode {};   // does a thing

class IntegerLiteralNode : public ExpressionNode { long long value; };
class BinaryExpressionNode : public ExpressionNode {
    Operator op;
    std::unique_ptr<ExpressionNode> left, right;
};
class VariableDeclarationNode : public StatementNode {
    std::string variableName, typeName;
    std::unique_ptr<ExpressionNode> initializer;
    bool isConst;
};
```

> Two families: expressions have values, statements do things. Every node
> owns its children through `unique_ptr` — the tree cleans itself up, no
> memory management drama. And notice every node carries `line` and `column`
> from episode 2's tokens.

---

## Segment 2 — Recursive descent: the big idea (5:00–10:00)

[SHOW: parser skeleton]
```cpp
class Parser {
    const std::vector<Token>& m_tokens;
    size_t m_currentPos = 0;

    const Token& peek(int offset = 0) const;
    const Token& advance();
    const Token& consume(TokenType expected, const std::string& err);
};
```

NARRATION:
> Recursive descent means: **one function per grammar rule**. There's a
> function that knows how to parse a statement. A function for an if. A
> function for an expression. And when a rule contains another rule — an if
> statement contains a condition expression — the function just... calls the
> other function. The grammar of your language becomes the call graph of
> your parser. It's the single most readable way to write a parser, which is
> why Clang, V8, and Roslyn — real industrial compilers — all use it.

[SHOW: `consume` with the positioned error]
```cpp
const Token& Parser::consume(TokenType expected, const std::string& msg) {
    if (peek().type == expected) return advance();
    throw std::runtime_error("Parse Error: " + msg + ". Found " +
        tokenTypeToString(peek().type) + " at line " +
        std::to_string(peek().line) + ", column " +
        std::to_string(peek().column));
}
```

> `consume` is your assertion: "the grammar says a `)` must be here — eat it
> or die with a good error." Most of your parser's error handling is just
> using consume instead of advance.

---

## Segment 3 — Parsing statements (10:00–15:00)

[TYPE: the statement dispatcher — abbreviated live, show full from repo]
```cpp
std::unique_ptr<StatementNode> Parser::parseStatement() {
    if (peek().type == TOK_LET || peek().type == TOK_CONST)
        return parseVariableDeclarationStatement();
    if (peek().type == TOK_IF)     return parseIfStatement();
    if (peek().type == TOK_WHILE)  return parseWhileStatement();
    if (peek().type == TOK_RETURN) return parseReturnStatement();
    // ... identifiers start expressions/assignments ...
}
```

[SHOW: `parseVariableDeclarationStatement` — walk it line by line]
```cpp
bool isConst = (peek().type == TOK_CONST);
advance();                                        // let / const
const Token& name = consume(TOK_IDENTIFIER, "Expected variable name");
std::string typeName = "auto";
if (peek().type == TOK_COLON) { advance(); typeName = parseType(); }
consume(TOK_EQUAL, "Expected '='");
auto initializer = parseExpression();             // recursion!
consume(TOK_SEMICOLON, "Expected ';'");
return std::make_unique<VariableDeclarationNode>(name.value, typeName,
                                                 std::move(initializer), isConst);
```

NARRATION:
> Read it out loud and it *is* the grammar: let-or-const, then a name, then
> optionally colon-and-a-type, then equals, then an expression, then a
> semicolon. When your parser reads like the spec, you wrote it right.

---

## Segment 4 — THE trick: precedence climbing (15:00–23:00)

[B-ROLL / DIAGRAM: precedence ladder, loosest at top]

```
parseExpression
 └─ parseLogicalOr      (||)
     └─ parseLogicalAnd (&&)
         └─ parseComparison  (== != < <= > >=)
             └─ parseAddition     (+ -)
                 └─ parseMultiplication (* / %)
                     └─ parseUnary  (! -)
                         └─ parsePrimary (literals, names, parens)
```

NARRATION:
> Here's the trick from the cold open. One function per precedence *level*,
> arranged in a ladder from loosest-binding to tightest. Each level parses
> its left side by calling the next level down, then loops on its own
> operator.

[TYPE: two levels, live — this is the money shot of the episode]
```cpp
std::unique_ptr<ExpressionNode> Parser::parseAdditionExpression() {
    auto expr = parseMultiplicationExpression();          // go DEEPER first
    while (peek().type == TOK_PLUS || peek().type == TOK_MINUS) {
        TokenType op = advance().type;
        auto right = parseMultiplicationExpression();     // and again
        expr = std::make_unique<BinaryExpressionNode>(
            op == TOK_PLUS ? ADD : SUBTRACT, std::move(expr), std::move(right));
    }
    return expr;
}
```

> Trace `5 + 3 * 2` on paper with me. [DIAGRAM: step-through animation]
> Addition asks multiplication for its left side — multiplication grabs the
> `5`, sees no `*`, hands it back. Addition sees `+`, asks multiplication for
> the right side — and *this* time multiplication grabs `3`, sees the `*`,
> and consumes `3 * 2` whole. The deeper function runs first, so tighter
> operators nest deeper. Precedence emerges from the call order. No tables,
> no magic.

> And parentheses? `parsePrimary` sees `(`, calls all the way back up to
> `parseExpression`, and consumes the `)`. Recursion gives you parentheses
> for free.

⚠ PITFALL:
> The `while` loop, not `if`. With `if`, `1 + 2 + 3` parses as `1 + 2` and
> then dies on the second plus. Left-associativity lives in that loop.

---

## Segment 5 — Primary expressions, calls, and the driver (23:00–28:00)

[SHOW: `parsePrimaryExpression` + function-call detection]
```cpp
if (peek().type == TOK_IDENTIFIER) {
    const Token& name = advance();
    if (peek().type == TOK_LPAREN) {         // it's a call: name(...)
        auto call = std::make_unique<FunctionCallNode>(name.value);
        advance();
        while (peek().type != TOK_RPAREN) {
            call->arguments.push_back(parseExpression());
            if (peek().type == TOK_COMMA) advance(); else break;
        }
        consume(TOK_RPAREN, "Expected ')'");
        return call;
    }
    return std::make_unique<VariableExpressionNode>(name.value);
}
```

NARRATION:
> One token of lookahead: a name followed by `(` is a call; otherwise it's a
> variable. Ninety percent of parsing ambiguity dissolves with one peek.

[SCREEN: run `--print-ast` on a file with nested ifs and math — scroll the
tree, point out shapes matching the source]

---

## [OUTRO] (28:00–29:00)

> We can now see the *structure* of any program. But a tree is still just
> data. Next episode we cross the line that separates toys from real
> compilers: we walk this tree and emit LLVM IR — and your language produces
> its first native executable. It's the best moment in this entire series.

**Checkpoint tag:** `ep03-parser` · **Homework:** add the `%` operator to the
multiplication level, and parse `1 + 2 % 3` — check the tree shape.
