
## Build and Measure

```bash
clang -O3 -flto=full src/main.c -o program
hyperfine --warmup 2 ./program 

cargo build --release
hyperfine --warmup 2 ./target/release/main
```

Initial results are:

```
C - 1.409 s ±  0.027 s
Rust - 1.489 s ±  0.025 s
```