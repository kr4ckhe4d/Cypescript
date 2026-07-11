# EP07 — Objects at Native Speed

**Length target:** 26–30 min · **Repo anchor:** `src/ObjectOptimizer.cpp`,
`createOptimizedObjectWithProperties`, `visit(ObjectAccessNode*)`,
`getOrCreateMethodFunction`, `checkInterfaceConformance`
**Goal:** object literals as LLVM structs, one-instruction property access,
methods with `this`, and compile-time-checked interfaces.

---

## [COLD OPEN] (0:00–0:40)

[SCREEN: two bars on a chart: "hash map objects: 720ms" vs "struct objects:
339ms" — 2.5M property accesses]

NARRATION:
> Same language. Same benchmark — two and a half million property reads. The
> only difference is how objects are stored. The slow bar looks up properties
> in a hash map at runtime, like a dynamic language. The fast bar compiles
> `user.age` down to **one machine instruction**. Today: how to get the fast
> bar, and why JavaScript engines spend millions of engineering hours trying
> to approximate what we get for free.

---

## Segment 1 — The design decision (0:40–6:00)

[B-ROLL / DIAGRAM: `{name: "Alice", age: 28, active: true}` shown two ways —
left: hash map with buckets and pointers; right: a flat struct
`| ptr(name) | i32 age | i32 active |` with byte offsets 0, 8, 12]

NARRATION:
> When your language sees an object literal, it has a choice.
>
> Option one — dynamic: store properties in a hash map. Flexible: add
> properties at runtime, delete them, whatever. Cost: every single `.age`
> hashes a string and chases buckets.
>
> Option two — static: at *compile time*, decide the object's memory layout.
> Name at offset 0, age at offset 8, active at offset 12. Now `.age` is
> "read 4 bytes at offset 8" — the same code C emits for a struct field.
>
> The catch: you must know every property at compile time. For a typed,
> TypeScript-ish language, you do — the literal is right there in the source.
> This is the deepest lesson of this series: **information you keep at
> compile time is speed you don't pay for at runtime.**

---

## Segment 2 — Compile-time layouts (6:00–12:00)

[SHOW: `ObjectOptimizer::ObjectLayout` from the repo]
```cpp
struct ObjectLayout {
    std::vector<std::pair<std::string, PropertyInfo>> properties; // in order
    std::map<std::string, size_t> propertyIndices;                // name → slot
    llvm::StructType* structType;                                 // the LLVM type
};
```

[SHOW: layout construction — walk the type mapping]
```cpp
for (auto& [name, typeName] : properties) {
    if (typeName == "string")      fields.push_back(charPtr);
    else if (typeName == "i32")    fields.push_back(i32Ty);
    else if (typeName == "f64")    fields.push_back(doubleTy);
    else if (isNested(typeName))   fields.push_back(charPtr); // ptr to child
    layout.propertyIndices[name] = i++;
}
layout.structType = llvm::StructType::create(ctx, fields, "CypescriptObject");
```

NARRATION:
> One pass over the literal's properties builds three things: the ordered
> field list, a name-to-slot map, and an actual LLVM struct type. Nested
> objects are just pointer fields to *their own* struct — which is how
> `company.employee.name` will work with zero extra machinery.

[SHOW: creation — alloca the struct, store each field]

---

## Segment 3 — GEP: the one instruction (12:00–17:00)

[SHOW: `generateDirectPropertyAccess`]
```cpp
size_t idx = layout.propertyIndices.at(property);       // compile-time lookup!
llvm::Value* fieldPtr = builder.CreateGEP(layout.structType, objectPtr,
    {constInt(0), constInt(idx)});
return builder.CreateLoad(fieldType, fieldPtr);
```

NARRATION:
> Meet GEP — GetElementPtr — the most famously confusing instruction in LLVM,
> and the star of this episode. GEP does *address math*: "starting at this
> struct pointer, give me the address of field number 2." It loads nothing,
> stores nothing. Just computes an address. Then one load reads the value.
>
> The line to worship is the first one: `propertyIndices.at(property)`
> happens in *our compiler*, at compile time. The hash lookup still exists —
> we just moved it out of your program and into mine. At runtime: address
> math plus one load. Assignments? Same GEP, CreateStore instead. That's
> `obj.age = 29`.

[SCREEN: run the repo's phase-1 benchmark on camera, show the 2.1x table
from `OPTIMIZATION_ROADMAP.md`]

⚠ PITFALL:
> That first `0` in the GEP indices trips everyone. The object pointer is
> "a pointer to *an array of* one struct" as far as GEP cares — the 0 says
> "the zeroth struct," *then* the field index. Forget it and your offsets
> are garbage. Every LLVM developer has this scar.

---

## Segment 4 — Methods and `this` (17:00–23:00)

[SCREEN: the target program]
```typescript
let account = {
    balance: 100,
    deposit(amount: i32): i32 {
        this.balance += amount;
        return this.balance;
    }
};
```

NARRATION:
> Methods look magical. They're not: **a method is a function with a secret
> first parameter.** `account.deposit(50)` is really
> `deposit(account, 50)`, and `this` is just parameter number zero.

[SHOW: `getOrCreateMethodFunction` — key lines only]
```cpp
paramTypes.push_back(charPtr);              // implicit `this`
// ... declared params after ...
auto* fn = llvm::Function::Create(..., objectKey + "_" + methodName, ...);
// inside the body's entry block:
namedValues["this"] = thisAlloca;           // `this` is just... a variable
variableToObjectKey["this"] = objectKey;    // so this.balance finds the layout
```

> Bind the hidden parameter into the symbol table under the name "this", and
> point "this" at the object's layout. Now every piece of machinery we
> already built — property GEPs, assignment, even `this.balance += 1` — just
> works, because to the rest of codegen, `this` is an ordinary variable that
> happens to hold an object pointer.
>
> And because we know at compile time which object a method belongs to, calls
> are *direct* — no vtable, no dynamic dispatch, inlinable by LLVM. Spoiler
> for episode 10: this is a big reason the benchmarks look the way they do.

[SCREEN: run the calculator/account demo.]

---

## Segment 5 — Interfaces: types that cost nothing (23:00–27:00)

[SCREEN: demo first — the error is the feature]
```typescript
interface User { name: string; age: i32; }
let u: User = { name: "Alice" };
// Type Error: Object assigned to 'u' does not satisfy interface 'User':
//   missing property 'age'
```

[SHOW: `checkInterfaceConformance` — the shape: for each interface member,
find it in the literal, check its kind; recurse for `extends`.]

NARRATION:
> Here's the punchline about interfaces in a compiled language: they're
> **free**. No runtime representation, no tag on the object, nothing in the
> binary. An interface is a promise checked while compiling — walk the
> members, match them against the literal, throw a good error if one's
> missing or mistyped. `extends`? Collect the parent's members first,
> recursively. Forty lines total in the repo. Zero-cost abstraction isn't a
> marketing phrase; you just implemented one.

---

## [OUTRO] (27:00–28:00)

> Structs, GEP, hidden-parameter methods, free interfaces — your language now
> has an object system that would make a JIT jealous. Next episode we handle
> failure: try/catch/throw, implemented with the two weirdest functions in
> the C library — setjmp and longjmp — plus a semantic analyzer that catches
> your users' mistakes before codegen ever runs.

**Checkpoint tag:** `ep07-objects` · **Homework:** add object destructuring
`let { name, age } = user;` — hint: it's a loop over bindings that reuses the
property-access visitor. The repo's `DestructuringDeclarationNode` is 40 lines.
