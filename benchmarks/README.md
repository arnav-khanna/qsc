# Benchmarks

This directory contains the standard benchmark corpora and the latest generated results for QSC.

## Layout

- `datasets/canterbury/`: Canterbury Corpus files.
- `datasets/calgary/`: Calgary Corpus files.
- `datasets/SHA256SUMS`: SHA-256 hashes for the checked-in benchmark files.
- `run.py`: benchmark runner.
- `results/results.csv`: machine-readable benchmark output.
- `results/summary.md`: human-readable benchmark summary.

## Data Sources

The included benchmark data comes from the Canterbury corpus site:

- Canterbury Corpus: `cantrbry.zip`
- Calgary Corpus: `calgary.tar.gz`

Large corpora such as Silesia, Enwik8, and Enwik9 are intentionally not checked into Git. Add them separately when running larger research benchmarks.

## Methodology

Build QSC first:

```sh
make
```

Install optional baseline dependencies:

```sh
python3 -m pip install -r requirements-benchmark.txt
```

Run the benchmark:

```sh
python3 benchmarks/run.py
```

The runner validates decompression for every algorithm that participates. QSC results include command-line process startup, archive framing, file I/O, and filesystem extraction. Python baselines run in process on byte buffers. For that reason, these results are useful for development tracking and transparency, but they are not a perfectly fair production-codec shootout.

## Metrics

`results/results.csv` records:

- corpus;
- file;
- algorithm;
- original size;
- compressed size;
- compression ratio;
- compression time;
- decompression time;
- compression speed;
- decompression speed;
- verification status.

See `results/summary.md` for the latest generated summary.
