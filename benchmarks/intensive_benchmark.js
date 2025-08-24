// JavaScript Intensive Computational Benchmark
// Equivalent to Cypescript intensive benchmark

console.log("=== JavaScript Intensive Computational Benchmark ===");

// Benchmark 1: Nested Loop Computation
console.log("Benchmark 1: Nested Loop Matrix Multiplication Simulation");
const matrixSize = 200;
const matrixIterations = 100;

console.log("Matrix size: 200x200");
console.log("Iterations: 100");

let totalOperations = 0;
console.log("Starting intensive nested loop benchmark...");
const nestedStart = performance.now();

for (let iter = 0; iter < matrixIterations; iter++) {
    for (let i = 0; i < matrixSize; i++) {
        for (let j = 0; j < matrixSize; j++) {
            for (let k = 0; k < matrixSize; k++) {
                // Simulate matrix multiplication operation
                const result = (i * j + k) % 1000;
                totalOperations += result;
            }
        }
    }
}

const nestedEnd = performance.now();
const nestedTime = nestedEnd - nestedStart;
console.log("Nested loop benchmark complete");
console.log(`Total operations: ${totalOperations}`);
console.log(`Nested loop time: ${nestedTime.toFixed(2)}ms`);

// Benchmark 2: Prime Number Generation
console.log("Benchmark 2: Prime Number Generation");
const primeLimit = 10000;
let primeCount = 0;

console.log("Finding primes up to: 10,000");

console.log("Starting prime generation benchmark...");
const primeStart = performance.now();

for (let num = 2; num <= primeLimit; num++) {
    let isPrime = true;
    
    // Check if number is prime
    for (let divisor = 2; divisor * divisor <= num; divisor++) {
        if (num % divisor === 0) {
            isPrime = false;
            break;
        }
    }
    
    if (isPrime) {
        primeCount++;
    }
}

const primeEnd = performance.now();
const primeTime = primeEnd - primeStart;
console.log("Prime generation benchmark complete");
console.log(`Primes found: ${primeCount}`);
console.log(`Prime generation time: ${primeTime.toFixed(2)}ms`);

// Benchmark 3: Fibonacci Sequence
console.log("Benchmark 3: Fibonacci Sequence Generation");
const fibCount = 35;
const fibIterations = 1000;

console.log("Fibonacci numbers to calculate: 35");
console.log("Iterations: 1,000");

let fibTotal = 0;
console.log("Starting Fibonacci benchmark...");
const fibStart = performance.now();

for (let fibIter = 0; fibIter < fibIterations; fibIter++) {
    for (let n = 0; n <= fibCount; n++) {
        // Calculate nth Fibonacci number iteratively
        let a = 0;
        let b = 1;
        let fibResult = 0;
        
        if (n === 0) {
            fibResult = 0;
        } else if (n === 1) {
            fibResult = 1;
        } else {
            for (let fibI = 2; fibI <= n; fibI++) {
                fibResult = a + b;
                a = b;
                b = fibResult;
            }
        }
        
        fibTotal += fibResult;
    }
}

const fibEnd = performance.now();
const fibTime = fibEnd - fibStart;
console.log("Fibonacci benchmark complete");
console.log(`Total Fibonacci sum: ${fibTotal}`);
console.log(`Fibonacci time: ${fibTime.toFixed(2)}ms`);

// Benchmark 4: Array Sorting Simulation (Bubble Sort)
console.log("Benchmark 4: Array Sorting Simulation");
const sortSize = 1000;
const sortIterations = 50;

console.log("Array size: 1,000");
console.log("Iterations: 50");

const testArray = [64, 34, 25, 12, 22, 11, 90, 88, 76, 50, 42, 30, 18, 95, 70];
const testSize = testArray.length;
let sortOperations = 0;

console.log("Starting sorting benchmark...");
const sortStart = performance.now();

for (let sortIter = 0; sortIter < sortIterations; sortIter++) {
    // Bubble sort implementation
    for (let i = 0; i < testSize - 1; i++) {
        for (let j = 0; j < testSize - i - 1; j++) {
            if (testArray[j] > testArray[j + 1]) {
                // Swap elements
                const temp = testArray[j];
                testArray[j] = testArray[j + 1];
                testArray[j + 1] = temp;
                sortOperations++;
            }
        }
    }
    
    // Reset array for next iteration (reverse it)
    testArray.reverse();
}

const sortEnd = performance.now();
const sortTime = sortEnd - sortStart;
console.log("Sorting benchmark complete");
console.log(`Sort operations: ${sortOperations}`);
console.log(`Sorting time: ${sortTime.toFixed(2)}ms`);

// Summary
const totalTime = nestedTime + primeTime + fibTime + sortTime;
console.log("=== JavaScript Intensive Benchmark Summary ===");
console.log(`Nested loops: ${nestedTime.toFixed(2)}ms`);
console.log(`Prime generation: ${primeTime.toFixed(2)}ms`);
console.log(`Fibonacci: ${fibTime.toFixed(2)}ms`);
console.log(`Sorting: ${sortTime.toFixed(2)}ms`);
console.log(`Total time: ${totalTime.toFixed(2)}ms`);
console.log("=== Intensive Benchmark Complete ===");
