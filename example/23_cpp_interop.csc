// 23 — Calling C++ from Cypescript
//
// Example 21 showed plain C. This one uses C++ where it actually pays: a
// growable map with sorted iteration and cleanup that happens on its own.
//
// The workflow is identical — one `link source` line, one `declare` per
// function. cscript compiles a .cpp with clang++ -std=c++17 and a .c with
// clang -std=c11, so each file gets the language it is written in.
//
//   cscript -r example/23_cpp_interop.csc

link source "native/wordindex.cpp";

// The C++ side is an opaque handle. Cypescript holds it as `ptr` and never
// looks inside — std::map and std::string stay entirely on the far side.
declare function wordindex_new(): ptr;
declare function wordindex_free(index: ptr): void;
declare function wordindex_add(index: ptr, text: string): void;
declare function wordindex_unique(index: ptr): i32;
declare function wordindex_count(index: ptr, word: string): i32;
declare function wordindex_word_at(index: ptr, position: i32): string;
declare function wordindex_most_common(index: ptr): string;

let index: ptr = wordindex_new();
if (index == null) {
    println("could not allocate the index");
} else {
    wordindex_add(index, "the quick brown fox jumps over the lazy dog");
    wordindex_add(index, "The dog barks and the fox runs");

    println(`unique words: ${wordindex_unique(index)}`);
    println(`"the" appears ${wordindex_count(index, "the")} times`);
    println(`"fox" appears ${wordindex_count(index, "fox")} times`);
    println(`"cat" appears ${wordindex_count(index, "cat")} times`);
    println(`most common: ${wordindex_most_common(index)}`);

    // std::map is ordered, so this walk comes out alphabetical for free
    println("first five, alphabetically:");
    let i: i32 = 0;
    while (i < 5) {
        println(`  ${wordindex_word_at(index, i)}`);
        i += 1;
    }

    // The handle owns everything; freeing it releases the map and its strings
    wordindex_free(index);
}

// A .c and a .cpp can be linked into the same program — see
// tests/test_c_interop.csc, which does exactly that.
println("done");
