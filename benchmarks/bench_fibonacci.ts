function fibonacci(n) {
    let a = 0, b = 1;
    for (let i = 0; i < n; i++) {
        let temp = a + b; a = b; b = temp;
    }
    return a;
}
const iterations = 10000000;
let sum = 0;
for (let i = 0; i < iterations; i++) { sum += fibonacci(20 + i % 5); }
console.log(`Sum: ${sum}`);
