// JavaScript Array Processing Benchmark
// Equivalent to Cypescript array_processing_benchmark.csc

console.log("=== JavaScript Array Processing Benchmark ===");

// Configuration
const arraySize = 1000000; // 1 million elements
console.log(`Array size: ${arraySize.toLocaleString()} elements`);

// Create large array
console.log("Creating large array...");
const testArray = Array.from({length: 100}, (_, i) => i + 1); // 1-100 for comparison
const actualSize = testArray.length;
console.log(`Actual benchmark array size: ${actualSize}`);

// Benchmark 1: Array Sum
console.log("Benchmark 1: Array Sum");
const iterations = 100000; // 100,000 iterations
console.log(`Iterations: ${iterations.toLocaleString()}`);

const sumStart = performance.now();
let sumTotal = 0;
for (let iter = 0; iter < iterations; iter++) {
    let currentSum = 0;
    for (let i = 0; i < testArray.length; i++) {
        currentSum += testArray[i];
    }
    sumTotal += currentSum;
}
const sumEnd = performance.now();
const sumTime = sumEnd - sumStart;

console.log("Sum benchmark complete");
console.log(`Total sum across all iterations: ${sumTotal}`);
console.log(`Sum benchmark time: ${sumTime.toFixed(2)}ms`);

// Benchmark 2: Array Maximum
console.log("Benchmark 2: Array Maximum");
const maxIterations = 100000;
console.log(`Iterations: ${maxIterations.toLocaleString()}`);

const maxStart = performance.now();
let maxTotal = 0;
for (let iter = 0; iter < maxIterations; iter++) {
    let currentMax = testArray[0];
    for (let i = 1; i < testArray.length; i++) {
        if (testArray[i] > currentMax) {
            currentMax = testArray[i];
        }
    }
    maxTotal += currentMax;
}
const maxEnd = performance.now();
const maxTime = maxEnd - maxStart;

console.log("Max benchmark complete");
console.log(`Total max across all iterations: ${maxTotal}`);
console.log(`Max benchmark time: ${maxTime.toFixed(2)}ms`);

// Benchmark 3: Array Search
console.log("Benchmark 3: Array Search");
const searchIterations = 100000;
const searchTarget = 75;
console.log(`Iterations: ${searchIterations.toLocaleString()}`);
console.log(`Search target: ${searchTarget}`);

const searchStart = performance.now();
let foundCount = 0;
for (let iter = 0; iter < searchIterations; iter++) {
    let found = false;
    for (let i = 0; i < testArray.length; i++) {
        if (testArray[i] === searchTarget) {
            found = true;
            break;
        }
    }
    if (found) {
        foundCount++;
    }
}
const searchEnd = performance.now();
const searchTime = searchEnd - searchStart;

console.log("Search benchmark complete");
console.log(`Found target in iterations: ${foundCount}`);
console.log(`Search benchmark time: ${searchTime.toFixed(2)}ms`);

// Benchmark 4: Array Modification
console.log("Benchmark 4: Array Modification");
const modifyIterations = 50000; // Fewer iterations for modification
console.log(`Iterations: ${modifyIterations.toLocaleString()}`);

// Create a copy for modification
const modifyArray = [...testArray];

const modifyStart = performance.now();
for (let iter = 0; iter < modifyIterations; iter++) {
    // Double all elements
    for (let i = 0; i < modifyArray.length; i++) {
        modifyArray[i] *= 2;
    }
    
    // Reset array for next iteration
    for (let i = 0; i < modifyArray.length; i++) {
        modifyArray[i] /= 2;
    }
}
const modifyEnd = performance.now();
const modifyTime = modifyEnd - modifyStart;

console.log("Modification benchmark complete");
console.log(`Modification benchmark time: ${modifyTime.toFixed(2)}ms`);

// Summary
const totalTime = sumTime + maxTime + searchTime + modifyTime;
console.log("=== JavaScript Benchmark Summary ===");
console.log(`Sum benchmark: ${sumTime.toFixed(2)}ms`);
console.log(`Max benchmark: ${maxTime.toFixed(2)}ms`);
console.log(`Search benchmark: ${searchTime.toFixed(2)}ms`);
console.log(`Modification benchmark: ${modifyTime.toFixed(2)}ms`);
console.log(`Total time: ${totalTime.toFixed(2)}ms`);
console.log("=== All Benchmarks Complete ===");
