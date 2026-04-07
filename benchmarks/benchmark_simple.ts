// Benchmark: Simple loop (100M iterations) with data dependency
const iterations = 100000000;
let sum = 0;

for (let i = 0; i < iterations; i++) {
    sum += i % 7;
}

console.log(`Sum: ${sum}`);
