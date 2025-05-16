
## Build and Measure

```bash
clang -O3 src/main.c -o program
hyperfine --warmup 2 ./program 

cargo build --release
hyperfine --warmup 2 ./target/release/main
```

```bash
cargo build --release && hyperfine --warmup 2 ./target/release/main
```

Initial results are:

```
C: 
692.8 ms ±   7.7 m
Rust:
740.0 ms ±  20.1 ms
```

After specifying no-inline in both version and passing the mvstack to the heap, and switching to 300_000_000 iters, the results are:

```
C:
Benchmark 1: ./program
  Time (mean ± σ):     641.5 ms ±   2.2 ms    [User: 639.1 ms, System: 1.7 ms]
  Range (min … max):   638.0 ms … 645.8 ms    10 runs

Rust:
Benchmark 1: ./target/release/main
  Time (mean ± σ):     642.0 ms ±   1.7 ms    [User: 639.2 ms, System: 1.8 ms]
  Range (min … max):   640.5 ms … 645.2 ms    10 runs
```

Full dav1d build command from ninja:
```bash
clang ... -DNDEBUG -Wall -Winvalid-pch -Wextra -std=c99 -O3 -fvisibility=hidden -Wundef -Werror=vla -Wno-missing-field-initializers -Wno-unused-parameter -Wstrict-prototypes -Werror=missing-prototypes -Wshorten-64-to-32 -fomit-frame-pointer -ffast-math -fno-align-functions -DBITDEPTH=16 -MD -MQ
```

For this sample, use:

```
clang -DNDEBUG -Winvalid-pch -O3 -fvisibility=hidden -Wundef -Werror=vla -Wno-missing-field-initializers -Wno-unused-parameter -Wstrict-prototypes  -Wshorten-64-to-32 -fomit-frame-pointer -ffast-math -fno-align-functions -DBITDEPTH=16 -MD -MQ -i src/main.c -o program
```


But the results are the same (actually slightly slower), so we can ignore that for now.