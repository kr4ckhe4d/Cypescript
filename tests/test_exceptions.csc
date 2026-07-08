// Tests: try/catch/finally and throw

// Basic catch
try {
    println("in try");
    throw "something broke";
} catch (e) {
    println("caught: " + e);
}

// finally always runs
try {
    throw "with finally";
} catch (e) {
    println("handled");
} finally {
    println("finally ran");
}

// No throw: catch skipped, finally still runs
try {
    println("no error here");
} catch (e) {
    println("should not print");
} finally {
    println("finally after success");
}

// Throwing non-string values (converted to strings)
try {
    throw 42;
} catch (e) {
    println("caught number: " + e);
}

// Nested try blocks: inner throw handled by inner catch
try {
    try {
        throw "inner";
    } catch (e) {
        println("inner caught: " + e);
        throw "rethrown from catch";
    }
} catch (e) {
    println("outer caught: " + e);
}

// Throw from inside a function unwinds into the caller's try
function risky(n: i32): i32 {
    if (n < 0) {
        throw "negative input: " + n;
    }
    return n * 2;
}

try {
    println(risky(21));  // 42
    println(risky(-1));  // throws
    println("unreachable");
} catch (e) {
    println("caught from function: " + e);
}

println("done");
