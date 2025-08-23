// Test case to reproduce string array access issue
let names: string[] = ["Alice", "Bob", "Charlie"];

// This should print "Alice" but currently returns garbage
println("First name:");
println(names[0]);

println("Second name:");
println(names[1]);

println("Third name:");
println(names[2]);
