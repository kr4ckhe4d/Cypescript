// `super(...)` calls the parent constructor; `super.method(...)` calls the
// implementation an override replaced. Both are direct calls — dispatching them
// virtually would re-enter the override that asked for them.

class Animal {
    name: string = "animal";
    legs: i32 = 4;
    constructor(name: string) { this.name = name; }
    speak(): void { println(`${this.name} makes a sound`); }
}

class Dog extends Animal {
    tricks: i32 = 0;
    constructor(name: string, tricks: i32) {
        super(name);                 // parent constructor
        this.tricks = tricks;
    }
    speak(): void {
        super.speak();               // the implementation this one replaced
        println(`...and knows ${this.tricks} tricks`);
    }
}

let d: Dog = new Dog("rex", 3);
d.speak();
println(d.name);      // set by the parent constructor
println(d.legs);      // inherited default
println(d.tricks);

// --- Chains: super works at every level ---
class A {
    trail: string = "";
    constructor() { this.trail = "A"; }
    who(): string { return "A"; }
}
class B extends A {
    constructor() { super(); this.trail = this.trail + ">B"; }
    who(): string { return super.who() + ">B"; }
}
class C extends B {
    constructor() { super(); this.trail = this.trail + ">C"; }
    who(): string { return super.who() + ">C"; }
}

let c: C = new C();
println(c.trail);     // A>B>C — the constructor chain ran root-first
println(c.who());     // A>B>C — each override calls the one it replaced

// The outermost call is still virtual: a C reached as an A runs C's who()
let asA: A = new C();
println(asA.who());   // A>B>C
