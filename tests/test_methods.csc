// Tests: object methods, `this` binding, property assignment

let calculator = {
    value: 0,
    add: function(x: i32): i32 {
        this.value = this.value + x;
        return this.value;
    },
    subtract: function(x: i32): i32 {
        this.value = this.value - x;
        return this.value;
    },
    reset(): void {
        this.value = 0;
    }
};

println(calculator.add(5));      // 5
println(calculator.add(7));      // 12
println(calculator.subtract(2)); // 10
calculator.reset();
println(calculator.value);       // 0

// Direct property assignment
let user = { name: "Alice", age: 28, active: true };
user.age = 29;
user.name = "Alice Johnson";
println(user.age);  // 29
println(user.name); // Alice Johnson

// Methods reading multiple properties via this
let rect = {
    width: 4,
    height: 5,
    area(): i32 {
        return this.width * this.height;
    },
    scale(factor: i32): void {
        this.width = this.width * factor;
        this.height = this.height * factor;
    }
};

println(rect.area()); // 20
rect.scale(2);
println(rect.area()); // 80
