# Contributing

QSC is an experimental compression engine. Contributions are welcome when they are reproducible, tested, and clear about their effect on compression ratio, speed, memory, and compatibility.

## Development Setup

```sh
make
./qsc3_c benchmark benchmarks/datasets/canterbury/alice29.txt
python3 benchmarks/run.py
```

Optional Python packages for the benchmark harness:

```sh
python3 -m pip install brotli zstandard
```

## Pull Request Guidelines

- Keep behavior changes separate from documentation-only changes.
- Include roundtrip validation for codec changes.
- Include benchmark results for ratio or speed claims.
- State whether a change modifies the archive format or bitstream.
- Do not add generated archives, binaries, or large external corpora to Git.
- Keep third-party compressors out of the QSC codec path. They may be used only as benchmark baselines.

## Benchmark Claims

For benchmark claims, include:

- dataset names and sizes;
- dataset source or hash when possible;
- command used;
- compiler and flags;
- CPU and operating system;
- compressed size;
- compression and decompression time;
- verification result.
