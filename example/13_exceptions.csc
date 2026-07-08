// 13 — Exception Handling
// try/catch/finally and throw. Throws unwind through function calls.
// Uncaught exceptions print a message and exit with code 1.

function parseAge(input: i32): i32 {
    if (input < 0) {
        throw "age cannot be negative: " + input;
    }
    if (input > 150) {
        throw "age is implausible: " + input;
    }
    return input;
}

// Basic catch
try {
    println(parseAge(28));   // 28
    println(parseAge(-5));   // throws — next line never runs
    println("unreachable");
} catch (e) {
    println("caught: " + e);
}

// finally always runs, error or not
try {
    println(parseAge(200));
} catch (e) {
    println("caught: " + e);
} finally {
    println("cleanup runs either way");
}

// Nested try: rethrowing from a catch propagates outward
try {
    try {
        throw "inner problem";
    } catch (e) {
        println("inner handler: " + e);
        throw "escalated: " + e;
    }
} catch (e) {
    println("outer handler: " + e);
}

println("program continues normally");
