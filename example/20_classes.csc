// 20 — Classes
// Fields (with optional defaults), a constructor, and methods with `this`.
// Under the hood a class instance is the same fast struct as an object
// literal; `new` allocates it and runs the constructor.

class BankAccount {
    owner: string;
    balance: i32;

    constructor(owner: string, openingBalance: i32) {
        this.owner = owner;
        this.balance = openingBalance;
    }

    deposit(amount: i32): i32 {
        this.balance += amount;
        return this.balance;
    }

    withdraw(amount: i32): i32 {
        if (amount > this.balance) {
            throw "insufficient funds for " + this.owner;
        }
        this.balance -= amount;
        return this.balance;
    }

    describe(): void {
        println(`${this.owner}: $${this.balance}`);
    }
}

// Fields can have defaults instead of a constructor
class Config {
    host: string = "localhost";
    port: i32 = 8080;
    debug: boolean = false;
}

let account = new BankAccount("Alice", 100);
account.describe();               // Alice: $100
println(account.deposit(50));     // 150
println(account.withdraw(30));    // 120

try {
    account.withdraw(10000);
} catch (e) {
    println("error: " + e);       // error: insufficient funds for Alice
}

// Class names work as type annotations
let savings: BankAccount = new BankAccount("Bob", 500);
savings.describe();               // Bob: $500

// Instances are independent
let cfg = new Config();
println(cfg.host);                // localhost
cfg.port = 9090;
println(cfg.port);                // 9090
