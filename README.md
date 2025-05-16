
## Build and Measure

```bash
clang -O3 src/main.c -o program
hyperfine --warmup 2 ./program 

cargo build --release
hyperfine --warmup 2 ./target/release/main
```

Initial results are:

```
C - 1.409 s ±  0.027 s
Rust - 1.489 s ±  0.025 s
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