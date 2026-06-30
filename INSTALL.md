# Installation

QSC builds as a single C command-line binary.

## Requirements

- C11 compiler.
- POSIX-like shell environment.
- `pthread` and `libm`.

## Build

```sh
make
```

This creates `./qsc3_c`.

## Install

Install as `qsc` under `/usr/local/bin`:

```sh
make install
```

Install under a user-local prefix:

```sh
make install PREFIX="$HOME/.local"
```

Uninstall:

```sh
make uninstall
```

If you used a custom prefix, pass the same prefix to uninstall:

```sh
make uninstall PREFIX="$HOME/.local"
```

## Verify

```sh
qsc benchmark benchmarks/datasets/canterbury/alice29.txt
```

Or, without installing:

```sh
./qsc3_c benchmark benchmarks/datasets/canterbury/alice29.txt
```
