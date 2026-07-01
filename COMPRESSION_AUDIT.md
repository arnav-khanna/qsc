# Compression Audit

Audit date: 2026-07-01

Repository path: `/Users/rajivkhanna/Downloads/quick_sort 9`

## Executive Answer

Is this algorithm currently relying on Brotli or any other existing compressor?

No for the main QSC3 compression and decompression pipeline. The C codec path uses in-repository optional reversible transforms, in-repository BWT/MTF text clustering, an in-repository LZ tokenizer, in-repository adaptive models, and an in-repository arithmetic coder. Brotli appears only in `benchmarks/run.py` as an external comparison baseline. zlib appears only behind the optional `HAS_ZLIB` benchmark comparison path and the `make zlib` target.

Exact source references:

- Main chunk compression uses ratio-first transform selection and calls custom arithmetic payload encoders for emitted chunks: `qsc3.c:1173-1286`, `qsc3.c:1387-1656`.
- The LZ tokenizer is implemented in-tree: `lz_engine.c:208-380`.
- The arithmetic entropy coder is implemented in-tree: `range_coder.c:33-124`.
- BWT/MTF2 and row-XOR/RLE transforms are implemented in-tree: `qsc3.c:435-1169`, `qsc3.c:1441-1499`, `qsc3.c:1620-1649`.
- A byte-aligned fast payload coder remains in-tree as an experimental compatibility path, but v11 does not emit it by default: `qsc3.c:1288-1361`, `qsc3.c:1776-1869`.
- Chunk decompression dispatches to the in-tree arithmetic, direct literal arithmetic, or fast payload decoder and uses the in-tree LZ reconstructor: `qsc3.c:1668-1774`, `qsc3.c:1776-1876`.
- Transform decode dispatch is implemented in-tree: `qsc3.c:1878-2121`.
- Brotli is imported and invoked only by the benchmark script: `benchmarks/run.py:15-23`, `benchmarks/run.py:42-57`, `benchmarks/run.py:140-167`.
- Python zlib, bz2, lzma, Brotli, and Zstandard are benchmark baselines only: `benchmarks/run.py:1-57`.
- Optional C zlib support is benchmark-only and guarded by `HAS_ZLIB` in `qsc3.c:2513-2530`.
- The default build does not link zlib or Brotli: `Makefile`.
- The optional `make zlib` target links `-lz` only for benchmark comparison output.
- The existing built binary links only `/usr/lib/libSystem.B.dylib` according to `otool -L ./qsc3_c`.

## Architecture Review

The repository is a compact C command-line archive compressor with a Python benchmark harness.

Core modules:

- `main.c`: command-line entry point for `compress`, `decompress`, and `benchmark`.
- `qsc3.c` / `qsc3.h`: archive format, file walking, chunking, multi-threaded compression, sequential decompression, and C benchmark mode.
- `lz_engine.c` / `lz_engine.h`: custom LZ match finder and reconstructor with repeated-offset states.
- `context_model.c` / `context_model.h`: adaptive models for instructions, literals, repeated-offset types, and offset slots.
- `range_coder.c` / `range_coder.h`: custom binary arithmetic encoder and decoder.
- `dynarray.h`: macro-based dynamic arrays used by token streams and byte buffers.
- `benchmarks/run.py`: comparison harness against Python compression libraries.

The architecture is coherent for a research prototype: parsing, modeling, entropy coding, and archive packaging are separated into recognizable modules. The main risk is that the current project presents research ideas and production-facing wording together. It needs stronger tests, validation, documentation, and benchmark controls before GitHub publication as a serious codec.

## True Compression Pipeline

```text
Input path
-> qsc_pack()
-> file discovery and archive file table
-> full file read
-> 8 MiB chunk split
-> ratio-first raw, zero-run, row-XOR/RLE, shuffle4, shuffle13, static-text, dynamic-text, or BWT/MTF2 candidate selection
-> optional 512 KiB previous-chunk history for raw chunks
-> qsc_compress_chunk()
-> either:
   LZ token streams: instructions, literals, lengths, rep_types, new_offsets
   OR direct byte stream for BWT/MTF2 text
-> arithmetic-coded stream metadata and adaptive stream models
-> chunk frame: original_len, compressed_len, compressed bytes
-> QSC3 archive output
```

Compression is actually happening at:

- `qsc3.c:141-1169`: optional reversible transforms can rewrite text tokens, zero runs, row-correlated bytes, 4-byte lane-structured binary chunks, 13-byte lane-structured spreadsheet-like chunks, or BWT/MTF2 text clusters before final coding. The bit-plane decoder is retained for transform-flag compatibility, but v11 does not emit bit-plane chunks by default.
- `lz_engine.c:208-380`: tokenization replaces repeated byte sequences with match instructions.
- `qsc3.c:1173-1260`: selected LZ-backed chunk representations are encoded into an arithmetic-coded bitstream.
- `qsc3.c:1262-1286`: BWT/MTF2 text chunks can use a direct adaptive arithmetic byte payload.
- `range_coder.c:52-87`: arithmetic interval updates encode modeled bits.
- `range_coder.c:103-115`: final pending arithmetic bits are flushed into bytes.

Archive packaging happens in `qsc3.c:1264-1413`. Packaging is not compression by itself, aside from storing compressed chunk payloads.

## Algorithm Components

| Component | Classification | Evidence | Notes |
| --- | --- | --- | --- |
| Tokenization | Custom | `qsc3.c:141-1169`, `qsc3.c:1387-1656`, `lz_engine.c:208-380` | Optional zero-run, row-XOR/RLE, shuffle4, shuffle13, static-text, dynamic-text, and BWT/MTF2 transforms followed either by a custom LZ77-style parser or by direct byte arithmetic coding for BWT/MTF2 text. Standard family, custom implementation. |
| Prediction | Custom, incomplete | `context_model.c:58-157`, `context_model.c:199-285` | Adaptive instruction, rep-type, offset, and literal models exist. Some declared order-3 literal fields in `context_model.h:74-90` are not active in `context_model.c`. |
| Delta encoding | Custom, limited | `qsc3.c:435-503`, `qsc3.c:1441-1499` | Row-XOR is a reversible predictor/delta-like transform for row-correlated binary data. There is still no general numeric delta transform. |
| Dictionary encoding | Custom | `lz_engine.c:118-187`, `lz_engine.c:208-380` | The sliding-window LZ matcher functions as dictionary matching. No external dictionary compressor is used. |
| Columnar transforms | Custom, limited | `qsc3.c:315-434`, `qsc3.c:1173-1286` | Tokens are separated into logical streams before entropy coding. The active shuffle4 and shuffle13 transforms are limited byte reorderings for selected binary chunks, not a general table/column transform framework. |
| Entropy coding | Custom | `range_coder.c:33-210`, `qsc3.c:1173-1286` | In-tree binary arithmetic coder for LZ token streams and direct BWT/MTF2 byte streams. No Brotli/zlib/zstd entropy backend in the main codec. |
| Final compression stage | Custom | `qsc3.c:760-769`, `range_coder.c:103-119` | Final emitted chunk bytes are produced by the custom arithmetic encoder. |

## External Compression Backend Search

Findings:

- No Brotli call exists in the C compression or decompression path.
- No zstd, lzma, bz2, gzip, or deflate backend exists in the C codec path.
- `qsc3.c` includes zlib only when `HAS_ZLIB` is defined and uses it only inside `qsc_benchmark()`, not `qsc_pack()` or `qsc_compress_chunk()`.
- The current codec includes built-in static and dynamic text dictionaries implemented directly in `qsc3.c`; these are not Brotli dictionaries or Brotli calls.
- Zero-run, row-XOR/RLE, BWT/MTF2, shuffle4, and shuffle13 transforms are implemented directly in `qsc3.c`; they do not call external compression libraries. A bit-plane decoder remains in-tree for compatibility.
- `benchmarks/run.py` imports and calls Python compression libraries as baseline algorithms.
- `benchmarks/results/results.csv` contains benchmark rows for external codecs, but it is generated output, not codec implementation.

Source references:

- `qsc3.c:21-23`: conditional `#include <zlib.h>`.
- `qsc3.c:2513-2530`: optional zlib benchmark comparison.
- `benchmarks/run.py:1-57`, `benchmarks/run.py:140-167`: Python benchmark imports, baseline compressor/decompressor wrappers, and algorithm table.

## Benchmark Integrity

Are comparisons fair?

Not publication-grade. They are useful for local exploration, but the current comparisons mix subprocess archive compression for QSC3 with in-process memory-buffer compression for Python baselines.

Fairness issues:

- QSC3 measurements include process startup, archive metadata, file I/O, directory output, and filesystem cleanup in `benchmarks/run.py`.
- Python baselines operate on an already-loaded byte buffer and report raw compressed buffer size.
- QSC3 compressed size includes archive overhead.
- Compression levels are inconsistent: zlib uses level 9, zstd uses level 10, Brotli uses library defaults, and bz2/lzma use Python defaults.
- Dataset iteration uses checked-in Canterbury and Calgary corpus files under `benchmarks/datasets/`, with hashes in `benchmarks/datasets/SHA256SUMS`.
- Benchmark output is stored in `benchmarks/results/results.csv`; `benchmarks/results/summary.md` summarizes the current run.

Is Brotli being used internally?

No. Brotli is used by `benchmarks/run.py` as a baseline only.

Are results reporting PDT only or PDT+Brotli?

The code does not use the name `PDT`. The QSC3 rows report the output size of the QSC3 archive produced by `./qsc3_c compress`; they do not apply Brotli afterward. Brotli appears as a separate baseline row.

Are there misleading benchmark claims?

Potentially yes if `benchmarks/results/results.csv` is presented without caveats. The numbers compare different measurement surfaces: raw in-memory compressed buffers for Python libraries versus archive files and subprocess timings for QSC3. Claims should state that current results are exploratory and not a fair codec shootout.

## Scientific Novelty Evaluation

Genuinely novel or potentially novel parts:

- The exact combination of match-length bucket context with the mixed literal model is a custom design choice.
- The optional raw/zero-run/shuffle4/shuffle13/static/dynamic transform selection is custom to this implementation, though run-length coding, byte shuffling, and dictionary/text-token transforms are standard compression ideas.
- The particular scoring constants, stream order, and model mix are custom engineering decisions.

Standard compression techniques:

- LZ77-style sliding-window match copying.
- Hash-table match finding.
- Lazy parsing.
- Repeated-offset states similar to LZMA-family designs.
- Slot-based distance coding.
- Adaptive binary arithmetic/range coding.
- Context modeling and fixed-weight context mixing.
- Splitting token streams before entropy coding.
- Run-length and byte-shuffle transforms.

Closest existing research and systems:

- LZ77/LZSS-family dictionary compression.
- LZMA-style LZ parsing, repeated distances, literal contexts, and range coding.
- PPM and PAQ-family context modeling and arithmetic coding.
- Brotli and Zstandard as modern LZ-family production codecs with advanced entropy coding and block modeling.

Is this publishable as a research project?

Yes as an experimental research prototype, if claims are framed carefully and supported by reproducible ablation studies. It is not ready to be presented as a proven novel codec or production compressor.

What would need to change to become a standalone codec?

- Stable, written bitstream specification.
- Independent decoder conformance tests.
- Strong malformed-input validation.
- Checksums and corruption detection.
- CI and fuzzing.
- Reproducible large-corpus benchmarks.
- Ablation studies proving which model components improve compression.
- Clear licensing.
- Versioned compatibility guarantees.

## GitHub Publication Checklist

Missing documentation:

- Stable bitstream specification.
- Deeper security model for archive extraction.
- API documentation for chunk-level functions.

Missing tests:

- Unit tests for each module.
- Roundtrip regression tests.
- Cross-platform build tests.
- Decompression fuzzing.
- Malformed archive tests.
- Large-file and multi-file archive tests.

Missing validation:

- Archive path traversal protection.
- Bounds checks for malformed stream counts.
- Chunk-size validation.
- File-count and path-length limits.
- Checksum verification.
- Memory allocation failure handling.

Missing CI/CD:

- Broader GitHub Actions build matrix.
- Sanitizer builds.
- Static analysis.
- Python benchmark lint or smoke test.
- Release artifact workflow.

Licensing status:

- Project license is present in `LICENSE`.
- Third-party benchmark dependencies are listed in `requirements-benchmark.txt`.

Missing reproducibility requirements:

- Dependency versions.
- Compiler version and flags.
- CPU, operating system, and memory details.
- Multiple benchmark runs and statistical summaries.
- Raw command logs.
- Binary/source revision identifier.

## Verification Performed

- Searched source files for external compression libraries and entropy backends.
- Inspected the main C compression and decompression path.
- Inspected benchmark scripts.
- Checked the built binary with `otool -L`; the default binary links only `/usr/lib/libSystem.B.dylib`.
- Ran `make clean && make` with the default `-O3` build flags.
- Ran `python3 benchmarks/run.py` over 29 Canterbury + Calgary files.
- Verified QSC3 roundtrips for every benchmark row generated by `benchmarks/run.py`.

Benchmark result:

```text
QSC3 v11 aggregate: 6,062,277 original bytes -> 1,321,396 compressed bytes
ratio: 0.217970
verified rows: 29 / 29
```
