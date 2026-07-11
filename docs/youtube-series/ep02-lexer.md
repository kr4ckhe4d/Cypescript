# EP02 — Lexing: Turning Text into Tokens

**Length target:** 22–26 min · **Repo anchor:** `src/Token.h`, `src/Lexer.cpp`
**Goal:** viewer ends with a lexer that tokenizes a real program, prints the
token stream, and handles comments, strings with escapes, and numbers.

---

## [COLD OPEN] (0:00–0:30)

[SCREEN: terminal]

[TYPE]
```bash
./build/cscript --print-tokens example/01_hello.csc
```

[SCREEN: token stream scrolls: `IDENTIFIER("println") LPAREN STRING_LITERAL("Hello...") RPAREN SEMICOLON`]

NARRATION:
> Before a compiler can understand your code, it has to *read* it. This is
> what reading looks like: every keyword, name, number, and bracket, labeled.
> Today we build the machine that does that — and I promise it's easier than
> you think.

---

## Segment 1 — What is a token? (0:30–4:00)

[B-ROLL / DIAGRAM: the line `let count: i32 = 42;` exploding into labeled
boxes: LET · IDENTIFIER(count) · COLON · TYPE_I32 · EQUAL · INT_LITERAL(42) · SEMICOLON]

NARRATION:
> A token is one "word" of your language, plus its category. The lexer's
> whole job: characters in, tokens out. It doesn't know grammar — it can't
> tell you `let let let` is nonsense. It just labels words. That separation
> is what keeps each stage simple.

[SHOW: `src/Token.h` — the enum, scroll through categories]
```cpp
enum TokenType {
    TOK_IDENTIFIER, TOK_STRING_LITERAL, TOK_INT_LITERAL, TOK_FLOAT_LITERAL,
    TOK_LET, TOK_CONST, TOK_FUNCTION, TOK_IF, TOK_ELSE, /* ... */
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, /* ... */
    TOK_PLUS, TOK_EQUAL_EQUAL, TOK_ARROW, /* ... */
    TOK_EOF, TOK_UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line = 0;    // for error messages — trust me, add this NOW
    int column = 0;
};
```

⚠ PITFALL:
> Add line and column to your Token struct **today**. I didn't — I added it
> months later and had to touch every error message in the compiler. Your
> future self, staring at "Parse Error" with no location, will thank you.

---

## Segment 2 — The lexer skeleton (4:00–9:00)

[SHOW: `src/Lexer.h`]
```cpp
class Lexer {
    std::string_view m_source;
    size_t m_currentPos = 0;
    int m_line = 1, m_col = 1;

    char peek() const;      // look, don't consume
    char advance();         // consume one char, track line/col
    bool isAtEnd() const;
public:
    explicit Lexer(std::string_view source);
    Token getNextToken();   // the only public method
};
```

NARRATION:
> The entire lexer is two verbs. `peek` — look at the current character
> without moving. `advance` — consume it and move forward. Every lexer ever
> written is these two verbs in a trench coat.

[TYPE: `advance()` — emphasize the line tracking]
```cpp
char Lexer::advance() {
    char c = m_source[m_currentPos++];
    if (c == '\n') { m_line++; m_col = 1; } else { m_col++; }
    return c;
}
```

[SHOW: the dispatch in `getNextToken()`]
```cpp
Token Lexer::getNextToken() {
    skipWhitespace();                      // also eats comments!
    if (isAtEnd()) return Token(TOK_EOF, "");

    char c = peek();
    if (std::isalpha(c) || c == '_') return makeIdentifier();
    if (std::isdigit(c))             return makeIntegerLiteral();
    if (c == '"')                    return makeStringLiteral();
    // ... operators and punctuation ...
}
```

> Look at the shape: peek at one character, and it tells you what *kind* of
> token is starting. A letter? Identifier or keyword. A digit? Number. A
> quote? String. Lexing is a big switch statement wearing a fancy name.

---

## Segment 3 — Identifiers and keywords (9:00–12:00)

[SHOW: `makeIdentifier()` from the repo]
```cpp
Token Lexer::makeIdentifier() {
    size_t start = m_currentPos;
    while (std::isalnum(peek()) || peek() == '_') advance();
    std::string_view value = m_source.substr(start, m_currentPos - start);

    static const std::unordered_map<std::string_view, TokenType> keywords = {
        {"let", TOK_LET}, {"const", TOK_CONST}, {"function", TOK_FUNCTION},
        {"if", TOK_IF}, {"else", TOK_ELSE}, {"while", TOK_WHILE},
        {"return", TOK_RETURN}, {"true", TOK_TRUE}, /* ... */
    };
    auto it = keywords.find(value);
    if (it != keywords.end()) return Token(it->second, std::string(value));
    return Token(TOK_IDENTIFIER, std::string(value));
}
```

NARRATION:
> Here's a design decision that looks tiny and isn't: keywords are just
> identifiers that appear in this map. `let` isn't special to the character
> scanner — we scan a word, *then* check if it's reserved. This means adding
> a keyword to your language is literally adding one line to a map. When I
> added `class`, `switch`, and `throw` to Cypescript, the lexer change took
> thirty seconds each.

---

## Segment 4 — Numbers, strings, comments (12:00–19:00)

[SHOW: string literal handling with escapes]
```cpp
case 'n': result += '\n'; break;
case 't': result += '\t'; break;
case '\\': result += '\\'; break;
case '"': result += '"'; break;
```

NARRATION:
> Strings have one subtlety: escape sequences. When you see a backslash, the
> next character means something different. We *decode them now*, in the
> lexer, so no later stage ever thinks about backslashes again. Push work as
> early in the pipeline as it can go.

[SHOW: number lexing — decimal, then float detection]
```cpp
while (std::isdigit(peek())) advance();
if (peek() == '.' && std::isdigit(peekNext())) {
    advance();                              // consume '.'
    while (std::isdigit(peek())) advance();
    return Token(TOK_FLOAT_LITERAL, ...);
}
return Token(TOK_INT_LITERAL, ...);
```

⚠ PITFALL (tell the story — it's a real bug from this repo):
> Here's a bug that shipped in Cypescript for weeks. I special-cased numbers
> starting with `0` for hex — `0x1F` — and octal. And in that branch I forgot
> to check for `.`. Result: `0.5` lexed as the integer `0`, then a stray dot
> token, and the parser exploded with the least helpful error imaginable.
> Test your lexer with `0.5` specifically. Not `3.14`. `0.5`.

[SHOW: comments living in `skipWhitespace()` — `//` and `/* */`]

> Comments are whitespace with self-esteem. Handle them where you skip
> spaces, and the rest of the compiler never knows they existed.

---

## Segment 5 — Two-character operators + wiring it up (19:00–24:00)

[SHOW: the two-char operator lookahead]
```cpp
case '=':
    if (next == '=') { advance(); advance(); return Token(TOK_EQUAL_EQUAL, "=="); }
    if (next == '>') { advance(); advance(); return Token(TOK_ARROW, "=>"); }
    break;
```

NARRATION:
> `=` versus `==` versus `=>`: peek one character further before deciding.
> Maximal munch — always take the longest token that matches. Fun fact:
> when I later added TypeScript's `===`, the fix was two lines right here —
> consume a third `=` and treat it as `==`.

[TYPE: the driver loop in main.cpp]
```cpp
Lexer lexer(sourceCode);
Token token;
do {
    token = lexer.getNextToken();
    llvm::outs() << tokenTypeToString(token.type)
                 << " ('" << token.value << "')\n";
} while (token.type != TOK_EOF);
```

[SCREEN: run it on a real file. The cold-open shot, now earned.]

---

## [OUTRO] (24:00–25:00)

> That's a working lexer — the same core that's in the finished compiler
> today, handling twenty keywords, template literals, and floats. Next
> episode is where it gets architecturally interesting: we take this flat
> stream of tokens and discover its *structure*. Recursive descent parsing —
> the most useful algorithm you've never been taught.

**Checkpoint tag:** `ep02-lexer` · **Homework:** add a `%` token and a `//`
comment to a test file; verify with `--print-tokens`.
