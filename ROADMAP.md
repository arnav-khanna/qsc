# QSC3 Roadmap

This roadmap turns the current prototype into a cleaner open-source project first, then into a stronger research codec.

## Phase 1: Publishable Prototype

Focus: code cleanup, test coverage, and documentation.

- Clean up any remaining naming mismatches between QSC3, archive versions, and internal comments.
- Remove or implement inactive declarations such as unused order-3 literal model fields.
- Add unit tests for:
  - arithmetic encoder/decoder roundtrips;
  - varint encoding and decoding;
  - LZ tokenization and reconstruction;
  - repeated-offset state transitions;
  - text, zero-run, bit-plane, shuffle4, and shuffle13 transform roundtrips;
  - adaptive length coding roundtrips;
  - archive pack/unpack behavior.
- Add regression tests over small text, binary, JSON, CSV, HTML, CSS, JavaScript, and already-compressed files.
- Add malformed archive tests.
- Add fuzz testing for the decompressor.
- Add GitHub Actions for macOS and Linux builds.
- Add `LICENSE`.
- Add reproducibility notes for benchmarks.
- Keep generated archives, binaries, and large datasets out of the repository.
- Add compatibility tests for older archives versus the v9 transform-flagged and fast-payload chunk format.

## Phase 2: Standalone Bitstream

Focus: replace any possible backend ambiguity, implement a documented custom entropy stage, and formalize the bitstream format.

- Keep Brotli and other third-party compressors strictly outside the codec path.
- Replace optional benchmark-only zlib integration with a cleaner benchmark harness if desired.
- Stabilize the custom entropy coder API.
- Decide whether the final entropy coder should remain binary arithmetic coding or move to range coding/ANS.
- Write a normative bitstream specification.
- Replace the one-byte transform token limit with a versioned dictionary section.
- Add chunk-level checksums.
- Add file-level integrity metadata.
- Add decoder limits for memory, path length, file count, chunk size, and malformed stream states.
- Add a standalone decoder conformance test suite.

## Phase 3: Modeling Research

Focus: adaptive prediction, context modeling, and arithmetic/range coding improvements.

- Evaluate adaptive prediction for instruction streams.
- Evaluate direct length modeling, slot coding, and richer match-length contexts.
- Add ablation tests for each literal context.
- Experiment with richer literal contexts:
  - order-2 and order-3 byte contexts;
  - word and syntax contexts;
  - binary file contexts;
  - match-adjacent contexts.
- Improve offset modeling with distance classes and recent-distance probabilities.
- Test arithmetic coding versus range coding.
- Evaluate adaptive probability rescaling strategies.
- Explore context mixing with learned or adaptive weights instead of fixed weights.

## Phase 4: Large-Scale Benchmarking

Focus: reproducible evaluation across standard corpora and realistic datasets.

- Benchmark on Silesia.
- Benchmark on Calgary.
- Benchmark on Canterbury.
- Benchmark on Enwik8.
- Benchmark on Enwik9 or a clearly documented large-text alternative.
- Benchmark on image datasets:
  - raw bitmap data;
  - PNG/JPEG files as already-compressed controls;
  - structured image metadata datasets where relevant.
- Add binary executable corpora.
- Add source-code corpora.
- Add structured data corpora such as JSON, CSV, XML, and logs.
- Publish benchmark scripts that record:
  - dataset hashes;
  - codec versions;
  - compiler and flags;
  - CPU and memory;
  - operating system;
  - compression levels;
  - wall time and CPU time;
  - peak memory;
  - compressed size;
  - decompression validation.
- Run multiple trials and report medians, variance, and confidence intervals.
