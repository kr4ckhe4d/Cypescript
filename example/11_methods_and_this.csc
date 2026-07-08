// 11 — Object Methods and `this`
// Methods can be written as function-expressions or shorthand.
// Inside a method, `this` refers to the object and supports reads and writes.

let account = {
    owner: "Alice",
    balance: 100,

    deposit: function(amount: i32): i32 {
        this.balance += amount;
        return this.balance;
    },

    // Shorthand method syntax
    withdraw(amount: i32): i32 {
        if (amount > this.balance) {
            throw "insufficient funds";
        }
        this.balance -= amount;
        return this.balance;
    },

    describe(): void {
        println(`${this.owner} has $${this.balance}`);
    }
};

account.describe();             // Alice has $100
println(account.deposit(50));   // 150
println(account.withdraw(30));  // 120
account.describe();             // Alice has $120

// Methods composing state
let rect = {
    width: 4,
    height: 5,
    area(): i32 {
        return this.width * this.height;
    },
    scale(factor: i32): void {
        this.width *= factor;
        this.height *= factor;
    }
};

println(rect.area());  // 20
rect.scale(2);
println(rect.area());  // 80
