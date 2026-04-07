const N = 300;
let sum = 0;
for (let i = 0; i < N; i++) {
    for (let j = 0; j < N; j++) {
        let cell = 0;
        for (let k = 0; k < N; k++) { cell += (i * k + j) % 1000; }
        sum += cell % 997;
    }
}
console.log(`Sum: ${sum}`);
