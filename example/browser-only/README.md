# Browser-Only Examples

These examples use **advanced language features** that are only supported in the **browser interpreter**, not the native compiler.

## How to Run

**Use the interactive web documentation:**
```bash
./launch-docs.sh
```

Then copy and paste the code into the browser playground to run it.

## Why Browser-Only?

These examples use features that are **intentionally not implemented** in the native compiler:

### ❌ Object Literals
```typescript
let player = {
    name: "Aragorn",
    level: 45,
    stats: { hp: 850, mp: 200 }
};
```

### ❌ Object Property Access
```typescript
println(player.name);
println(player.stats.hp);
```

### ❌ Boolean Literals
```typescript
let isActive: boolean = true;
let isComplete: boolean = false;
```

### ❌ Floating-Point Literals
```typescript
let pi: f64 = 3.14159;
let temperature: f64 = 98.6;
```

## Design Philosophy

The **native compiler** focuses on:
- ✅ **Core programming constructs** (variables, loops, arrays)
- ✅ **High performance** (LLVM compilation)
- ✅ **C++ integration** (systems programming)

The **browser interpreter** provides:
- ✅ **Rich data structures** (objects, nested data)
- ✅ **Interactive development** (immediate feedback)
- ✅ **Learning environment** (experimentation)

## Example Files

- **`game_system.csc`** - RPG management system with complex object hierarchies
- **`complex_data_structures.csc`** - E-commerce system with nested objects and arrays

## Alternative Approaches

If you want to use these concepts in the native compiler, consider:

### Instead of Objects → Use Arrays and Indices
```typescript
// Instead of: let player = { name: "Aragorn", level: 45 }
let playerNames: string[] = ["Aragorn"];
let playerLevels: i32[] = [45];
let playerIndex: i32 = 0;

println(playerNames[playerIndex]); // "Aragorn"
println(playerLevels[playerIndex]); // 45
```

### Instead of Booleans → Use Integers
```typescript
// Instead of: let isActive: boolean = true;
let isActive: i32 = 1; // 1 = true, 0 = false

if (isActive == 1) {
    println("Player is active");
}
```

### Instead of Floats → Use Scaled Integers
```typescript
// Instead of: let pi: f64 = 3.14159;
let pi_scaled: i32 = 314159; // Scaled by 100000
// Use integer arithmetic and scale back when needed
```

## Future Development

These features **may be added** to the native compiler in future versions:
- 🔄 **Boolean literals** (`true`, `false`) - Relatively easy to add
- 🔄 **Floating-point support** (`f64` literals and arithmetic)
- ❓ **Object literals** - More complex, may remain browser-only for simplicity

The browser interpreter will **always support** the full feature set for learning and experimentation.
