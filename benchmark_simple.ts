const iterations = 100000000;
let sum = 0;

console.log("Starting TypeScript/Node.js simple benchmark (100,000,000 iterations)...");

const start = Date.now();
for (let i = 0; i < iterations; i++) {
    sum = sum + 1;
}
const end = Date.now();

console.log(`Benchmark completed! Sum: ${sum}`);
console.log(`Execution time: ${end - start}ms`);
