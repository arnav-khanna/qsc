# QSC3 Compression Engine

QSC3 is an experimental lossless compression project written in C. It packages one or more input files into a `.qsc` archive using optional reversible pre-transforms, a custom LZ-style tokenizer, adaptive probability models, adaptive match-length coding, and an in-tree binary arithmetic coder.

The current codebase is best understood as a research prototype. It does not currently depend on Brotli, zlib, zstd, LZMA, bzip2, or another external compressor for its main compression pipeline. The Python benchmark script uses those libraries only as comparison baselines.

## Motivation

The project explores whether a compact custom codec can combine:

- LZ77-style match finding with repeated-distance states.
- Separate token streams for instructions, match lengths, literals, repeated-offset types, and new offsets.
- Adaptive bit-level probability models tailored to each stream.
- Adaptive slot-based match-length coding.
- A final arithmetic coding stage instead of delegating entropy coding to an existing backend.

The long-term goal is to turn these experiments into a reproducible, standalone compression codec with a stable bitstream, rigorous benchmarks, and a clear research story.

## How The Algorithm Works

QSC3 splits files into 8 MiB chunks. Raw chunks may use up to 512 KiB of carry-over history from the previous chunk as match context. The carry-over bytes are available to the match finder, but are not emitted again in the decompressed output. Transformed chunks are encoded independently so each transform can remain self-contained.

Within each chunk, the compressor tests the original bytes plus selected reversible transforms. It keeps the representation that produces the smallest final encoded chunk.

The active transforms are:

- raw bytes;
- a zero-run transform for sparse binary/image-like data;
- a gated bit-plane transform for selected binary chunks;
- a static text-token transform using a built-in dictionary;
- a dynamic text-token transform using frequent per-chunk words and small delimiter fragments.

After transform selection, the compressor emits a sequence of LZ instructions:

- `0`: literal byte.
- `1`: match copy.

For matches, QSC3 stores a match length and an offset type. Offset types are modeled after repeated-distance codecs:

- `rep0`: repeat the most recent offset.
- `rep1`: repeat the second most recent offset.
- `rep2`: repeat the third most recent offset.
- `new_offset`: encode a new copy distance and promote it into the repeated-offset state.

The compressor then entropy-codes several logical streams with adaptive models:

- instruction stream, using an order-3 binary context;
- match lengths, using adaptive slot coding;
- literals, using a fixed-weight mix of byte, character-class, word, and match-length contexts;
- repeated-offset type stream, using a small adaptive cascade;
- new offsets, using slot coding and previous-slot context.

The final compressed chunk is the output of the custom arithmetic encoder in `range_coder.c`.

## Compression Pipeline

```text
Input file or directory
-> archive file table
-> 8 MiB file chunks
-> optional zero-run, bit-plane, or text-token transform
-> optional 512 KiB previous-chunk history for raw chunks
-> LZ tokenization
-> split token streams
-> adaptive stream models
-> binary arithmetic coding
-> chunk frames
-> QSC3 archive
```

Compression actually happens in two places:

- Redundancy reduction happens in the optional reversible transforms and in the LZ tokenizer, which replaces repeated byte ranges with length/offset instructions.
- Bit-level size reduction happens in the adaptive arithmetic encoder, which writes modeled token streams into the final chunk payload.

No Brotli post-processing stage is present in the main codec.

## File Format Specification

All archive-level integer fields are big-endian.

```text
Archive:
  magic              4 bytes      ASCII "QSC3"
  version            1 byte       currently 6
  file_count         uint32

  file table[file_count]:
    path_len         uint16
    path_bytes       path_len bytes, no NUL terminator
    original_size    uint64
    data_offset      uint64, absolute archive offset

  file data:
    repeated chunks:
      original_len   uint32
      compressed_len uint32
      compressed     compressed_len bytes
    sentinel:
      original_len   uint32 = 0
      compressed_len uint32 = 0
```

The `compressed` chunk payload starts with a transform byte:

```text
0  raw arithmetic payload, decoded with previous-chunk history
1  static text transform payload, decoded without previous-chunk history
2  dynamic text transform payload, decoded without previous-chunk history
3  zero-run transform payload, decoded without previous-chunk history
4  bit-plane transform payload, decoded without previous-chunk history
```

For dynamic text chunks, the transform byte is followed by a small dictionary header:

```text
token_count          uint8
repeated token_count times:
  token_len          uint8
  token_bytes        token_len bytes
```

For bit-plane chunks, the transform byte is followed by a four-byte big-endian original length. The remaining bytes are the arithmetic-coded bit-plane payload.

The arithmetic-coded stream has this logical order:

```text
num_instructions
num_literals
num_matches
num_new_offsets
instruction stream
match-length stream
literal stream
rep-type stream
new-offset stream
```

The first four fields are arithmetic-coded varints. The remaining streams are decoded using the same adaptive models used by the encoder.

## Installation

Requirements:

- A C11 compiler.
- POSIX-like environment.
- `pthread` and `libm`.
- Optional Python 3 dependencies for benchmarks: `brotli` and `zstandard`.

Build:

```sh
make
```

Install the command as `qsc`:

```sh
make install
```

Install into a custom prefix:

```sh
make install PREFIX="$HOME/.local"
```

Optional zlib comparison in the built-in benchmark:

```sh
make zlib
```

Clean build outputs:

```sh
make clean
```

## Usage Examples

Compress one file:

```sh
./qsc3_c compress benchmarks/datasets/canterbury/alice29.txt alice29.qsc
```

Decompress an archive into a directory:

```sh
./qsc3_c decompress test.qsc out
```

Benchmark one file with the C benchmark mode:

```sh
./qsc3_c benchmark benchmarks/datasets/canterbury/alice29.txt
```

Run the Python benchmark suite:

```sh
python3 -m pip install brotli zstandard
python3 benchmarks/run.py
```

## Benchmark Methodology

The Python benchmark script reads the standard corpora under `benchmarks/datasets/`, compresses and decompresses each file with Python bindings for zlib, bzip2, LZMA, Brotli, and Zstandard when available, then invokes the QSC3 command-line binary for comparison.

Current checked-in results are under `benchmarks/results/`. On the current Canterbury + Calgary run, QSC3 is smaller than zlib-9 and zstd-10 in aggregate, but Brotli, LZMA, and bzip2 still beat QSC3 on aggregate ratio. QSC3 is not yet a Brotli-beating general-purpose codec.

Important caveats:

- Python baselines run in-process and mostly measure memory-to-memory compression.
- QSC3 is measured through a subprocess and includes archive framing, file I/O, directory output, and process startup overhead.
- QSC3 compressed sizes include `.qsc` archive metadata; Python baseline sizes are raw compressed buffers.
- Codec levels are not normalized across algorithms.
- Results in `benchmarks/results/results.csv` should be treated as exploratory, not publication-grade.

A publication-quality benchmark should pin codec versions, compression levels, hardware, compiler flags, dataset hashes, warm-up policy, run count, statistical summaries, and exact command lines.

## Current Limitations

- No automated test suite.
- CI currently covers only a basic build and smoke test.
- No stable bitstream compatibility policy.
- No checksum or corruption detection in the archive format.
- Limited input validation for malformed archives.
- Decompression writes archive paths directly under the output directory and should be hardened before use on untrusted archives.
- Files are read fully into memory before chunk compression.
- The benchmark harness is useful for exploration but is not yet scientifically fair.
- Some declared model fields, such as order-3 literal model state, are not active in the implementation.
- The new transforms improve some corpus files but are not a substitute for production block modeling and static dictionaries such as Brotli's.

## Future Improvements

- Add unit tests for range coding, pre-transforms, LZ tokenization, adaptive length coding, archive roundtrips, and malformed input handling.
- Add a fuzzing harness for decompression.
- Define and version a stable bitstream specification.
- Add checksums per chunk and per file.
- Replace ad hoc benchmark scripts with a reproducible harness.
- Replace the fixed one-byte text-token format with a more scalable dictionary coding scheme.
- Add CI across macOS and Linux.
- Harden path handling and archive validation.
- Add streaming APIs so large files do not need to be loaded completely into memory.

## Research Roadmap

Near-term research should determine which parts of QSC3 add measurable value beyond a conventional LZ plus arithmetic/range coding design.

Priority questions:

- Do optional pre-transforms improve compression enough to justify their speed and format complexity?
- Which literal model contexts contribute meaningful gains?
- How much does repeated-offset modeling help on text, binaries, structured data, and already-compressed inputs?
- What is the best entropy coder for this design: arithmetic, range, ANS, or another approach?
- Can the format become deterministic, stable, and independently decodable from a written specification?

Closest prior-art families include LZ77-style parsing, LZMA-style repeated distances and range coding, PPM/PAQ-style context modeling, and modern LZ codecs such as Brotli and Zstandard.

## Contributing Guidelines

Before opening a pull request:

- Keep behavior changes separate from documentation-only changes.
- Add roundtrip tests for any codec change.
- Include benchmark commands and dataset hashes for performance claims.
- Do not claim algorithmic novelty without a clear comparison against prior work.
- Document any bitstream change in the file format section.
- Prefer small, reviewable changes with focused commit messages.

For research contributions, include the baseline, dataset, command line, compiler, CPU, operating system, and number of runs.
