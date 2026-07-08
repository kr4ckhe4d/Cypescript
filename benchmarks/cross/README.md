# Cross-Language Benchmarks

The **exact same algorithm** implemented in four languages:

| File | Language | Runtime |
|---|---|---|
| `*.csc` | Cypescript | native (LLVM -O2) |
| `*.ts`  | TypeScript | Node.js (type stripping) |
| `*.py`  | Python | CPython |
| `*.rs`  | Rust | native (rustc -O) |

The `.csc` and `.ts` files are nearly character-identical — demonstrating the
"run TypeScript with minute changes" goal (the only differences: `i32` vs
`number` and `println` vs `console.log`, and `console.log` works in Cypescript
too).

## Run

```bash
./build.sh                                    # build the compiler first
bash benchmarks/cross/run_cross_benchmarks.sh # best of 3 runs
bash benchmarks/cross/run_cross_benchmarks.sh 5  # best of 5 runs
```

The script verifies all four implementations produce the same output before
timing them, and skips languages that aren't installed.

## Results

Measured 2026-07-07 on Apple Silicon (macOS), best of 3 wall-clock runs.
Node v26.3.0, Python 3.14.4, rustc 1.89.0:

| Benchmark | Cypescript | Node (TS) | Python | Rust |
|---|---|---|---|---|
| Primes < 1M (trial division) | **0.051s** | 0.146s | 1.777s | 0.050s |
| Fibonacci fib(35) recursive  | **0.025s** | 0.125s | 0.621s | 0.026s |

**Takeaways**
- Cypescript is **within ~2% of Rust** on both workloads — both compile through
  LLVM with the same optimization pipeline.
- **2.9–5x faster than Node.js** running the identical TypeScript source.
- **25–35x faster than Python** (CPython 3.14).
- Numbers include process startup; Node carries ~50ms of runtime startup, which
  is why larger workloads (1M primes, fib(35)) are used.
