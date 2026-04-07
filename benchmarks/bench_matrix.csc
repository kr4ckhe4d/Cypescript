// Benchmark: Nested loops - matrix-style computation (300^3)
let N: i32 = 300;
let sum: i32 = 0;

for (let i: i32 = 0; i < N; i = i + 1) {
    for (let j: i32 = 0; j < N; j = j + 1) {
        let cell: i32 = 0;
        for (let k: i32 = 0; k < N; k = k + 1) {
            cell = cell + (i * k + j) % 1000;
        }
        sum = sum + cell % 997;
    }
}

print("Sum: ");
println(sum);
