// Object handles, `ptr` values and `null` must compare by address. They used to
// go through strcmp — which reads them as text and segfaults on anything
// non-null — because every pointer looked like a string to the compiler.

class Node { value: i32 = 0; }

declare function malloc(size: i64): ptr;
declare function free(block: ptr): void;

// A live object compared against null
let node: Node = new Node();
if (node == null) { println("wrong: live object read as null"); }
else { println("live object is not null"); }
if (node != null) { println("!= null works"); }

// A null-valued handle
function nothing(): Node { return null; }
let missing: Node = nothing();
if (missing == null) { println("null object compares equal to null"); }

// Identity, not contents: two distinct objects with identical fields
let a: Node = new Node();
let b: Node = new Node();
if (a == b) { println("wrong: distinct objects compared equal"); }
else { println("distinct objects are not equal"); }

// Aliases point at the same object
let alias: Node = a;
if (alias == a) { println("alias is the same object"); }

// Opaque C handles
let block: ptr = malloc(32);
if (block != null) { println("malloc returned non-null"); }
free(block);

let empty: ptr = null;
if (empty == null) { println("null ptr compares equal to null"); }

// Strings still compare by value, not by address
let s1: string = "hello";
let s2: string = "hel" + "lo";
if (s1 == s2) { println("strings still compare by value"); }
