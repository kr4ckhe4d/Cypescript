// TypeScript Array Processing Benchmark
// Equivalent to Cypescript benchmark for performance comparison

console.log("=== TypeScript Array Processing Benchmark ===");

// Configuration
const arraySize: number = 1000000; // 1 million elements
console.log("Array size: 1,000,000 elements");

// Create large array
console.log("Creating large array...");
const benchmarkArray: number[] = [];
for (let i: number = 1; i <= 100; i++) {
    benchmarkArray.push(i);
}

const actualSize: number = benchmarkArray.length;
console.log(`Actual benchmark array size: ${actualSize}`);

// Benchmark 1: Array Sum
console.log("Benchmark 1: Array Sum");
const iterations: number = 100000; // Repeat operation many times
console.log("Iterations: 100,000");

let sumTotal: number = 0;
console.log("Starting sum benchmark...");
const sumStart: number = performance.now();

for (let iter: number = 0; iter < iterations; iter++) {
    let currentSum: number = 0;
    
    // Sum all elements in array
    for (let j: number = 0; j < actualSize; j++) {
        currentSum += benchmarkArray[j];
    }
    
    sumTotal += currentSum;
}

const sumEnd: number = performance.now();
const sumTime: number = sumEnd - sumStart;
console.log("Sum benchmark complete");
console.log(`Total sum across all iterations: ${sumTotal}`);
console.log(`Sum benchmark time: ${sumTime.toFixed(2)}ms`);

// Benchmark 2: Array Maximum
console.log("Benchmark 2: Array Maximum");
const maxIterations: number = 100000;
console.log("Iterations: 100,000");

let maxTotal: number = 0;
console.log("Starting max benchmark...");
const maxStart: number = performance.now();

for (let maxIter: number = 0; maxIter < maxIterations; maxIter++) {
    let currentMax: number = benchmarkArray[0];
    
    // Find maximum element
    for (let k: number = 1; k < actualSize; k++) {
        if (benchmarkArray[k] > currentMax) {
            currentMax = benchmarkArray[k];
        }
    }
    
    maxTotal += currentMax;
}

const maxEnd: number = performance.now();
const maxTime: number = maxEnd - maxStart;
console.log("Max benchmark complete");
console.log(`Total max across all iterations: ${maxTotal}`);
console.log(`Max benchmark time: ${maxTime.toFixed(2)}ms`);

// Benchmark 3: Array Search
console.log("Benchmark 3: Array Search");
const searchIterations: number = 100000;
const searchTarget: number = 75;
console.log("Iterations: 100,000");
console.log(`Search target: ${searchTarget}`);

let foundCount: number = 0;
console.log("Starting search benchmark...");
const searchStart: number = performance.now();

for (let searchIter: number = 0; searchIter < searchIterations; searchIter++) {
    let found: boolean = false;
    
    // Linear search for target
    for (let l: number = 0; l < actualSize; l++) {
        if (benchmarkArray[l] === searchTarget) {
            found = true;
            break; // TypeScript can break early
        }
    }
    
    if (found) {
        foundCount++;
    }
}

const searchEnd: number = performance.now();
const searchTime: number = searchEnd - searchStart;
console.log("Search benchmark complete");
console.log(`Found target in iterations: ${foundCount}`);
console.log(`Search benchmark time: ${searchTime.toFixed(2)}ms`);

// Benchmark 4: Array Modification
console.log("Benchmark 4: Array Modification");
const modifyIterations: number = 50000; // Fewer iterations for modification
console.log("Iterations: 50,000");

// Create a copy for modification
const modifyArray: number[] = [...benchmarkArray];

console.log("Starting modification benchmark...");
const modifyStart: number = performance.now();

for (let modifyIter: number = 0; modifyIter < modifyIterations; modifyIter++) {
    // Double all elements
    for (let m: number = 0; m < actualSize; m++) {
        modifyArray[m] *= 2;
    }
    
    // Reset array for next iteration
    for (let n: number = 0; n < actualSize; n++) {
        modifyArray[n] /= 2;
    }
}

const modifyEnd: number = performance.now();
const modifyTime: number = modifyEnd - modifyStart;
console.log("Modification benchmark complete");
console.log(`Modification benchmark time: ${modifyTime.toFixed(2)}ms`);

// Summary
console.log("=== TypeScript Benchmark Summary ===");
console.log(`Sum benchmark: ${sumTime.toFixed(2)}ms`);
console.log(`Max benchmark: ${maxTime.toFixed(2)}ms`);
console.log(`Search benchmark: ${searchTime.toFixed(2)}ms`);
console.log(`Modification benchmark: ${modifyTime.toFixed(2)}ms`);
console.log(`Total time: ${(sumTime + maxTime + searchTime + modifyTime).toFixed(2)}ms`);
console.log("=== All Benchmarks Complete ===");
