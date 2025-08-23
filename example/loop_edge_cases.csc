// Loop Edge Cases Testing
print("=== Loop Edge Cases ===");

// 1. For loop with no initialization
print("=== For Loop - No Init ===");
let preInitialized: i32 = 10;
for (; preInitialized > 8; preInitialized = preInitialized - 1) {
    print("Pre-initialized value: ");
    print(preInitialized);
}

// 2. For loop with assignment initialization (not declaration)
print("=== For Loop - Assignment Init ===");
let existing: i32 = 0;
for (existing = 5; existing < 7; existing = existing + 1) {
    print("Assignment init: ");
    print(existing);
}

// 3. Do-while that executes exactly once
print("=== Do-While - Single Execution ===");
let singleRun: i32 = 10;
do {
    print("This runs once: ");
    print(singleRun);
    singleRun = singleRun + 1;
} while (singleRun < 10); // Condition is false immediately

// 4. Nested different loop types
print("=== Mixed Nested Loops ===");
let outer: i32 = 1;
while (outer <= 2) {
    print("Outer while: ");
    print(outer);
    
    for (let inner: i32 = 1; inner <= 2; inner = inner + 1) {
        print("  Inner for: ");
        print(inner);
        
        let doInner: i32 = 1;
        do {
            print("    Do-while: ");
            print(doInner);
            doInner = doInner + 1;
        } while (doInner <= 1);
    }
    
    outer = outer + 1;
}

print("=== Edge Cases Complete ===");
