#!/usr/bin/env python3
import bz2
import csv
import lzma
import os
import shutil
import subprocess
import sys
import tempfile
import time
import zlib
from collections import defaultdict
from pathlib import Path

try:
    import brotli
except ImportError:
    brotli = None

try:
    import zstandard as zstd
except ImportError:
    zstd = None


ROOT = Path(__file__).resolve().parents[1]
BENCH_DIR = ROOT / "benchmarks"
DATASET_DIR = BENCH_DIR / "datasets"
RESULTS_DIR = BENCH_DIR / "results"
RESULTS_CSV = RESULTS_DIR / "results.csv"
QSC_BINARY = ROOT / "qsc3_c"


def zlib_compress(data): return zlib.compress(data, 9)
def zlib_decompress(data): return zlib.decompress(data)
def bz2_compress(data): return bz2.compress(data)
def bz2_decompress(data): return bz2.decompress(data)
def lzma_compress(data): return lzma.compress(data)
def lzma_decompress(data): return lzma.decompress(data)


def build_algorithms():
    algos = {
        "zlib-9": (zlib_compress, zlib_decompress),
        "bzip2": (bz2_compress, bz2_decompress),
        "lzma": (lzma_compress, lzma_decompress),
    }

    if brotli is not None:
        algos["brotli-default"] = (brotli.compress, brotli.decompress)

    if zstd is not None:
        zstd_c = zstd.ZstdCompressor(level=10)
        zstd_d = zstd.ZstdDecompressor()
        algos["zstd-10"] = (zstd_c.compress, zstd_d.decompress)

    return algos


def discover_files():
    files = []
    for path in sorted(DATASET_DIR.rglob("*")):
        rel = path.relative_to(DATASET_DIR)
        if path.is_file() and path.name != "SHA256SUMS" and not any(part.startswith(".") for part in rel.parts):
            files.append(path)
    return files


def corpus_name(path):
    rel = path.relative_to(DATASET_DIR)
    return rel.parts[0] if len(rel.parts) > 1 else "local"


def run_qsc3(input_path):
    comp_path = Path(tempfile.mktemp())
    decomp_path = Path(tempfile.mktemp())

    try:
        t0 = time.time()
        subprocess.run(
            [str(QSC_BINARY), "compress", str(input_path), str(comp_path)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
        t1 = time.time()

        subprocess.run(
            [str(QSC_BINARY), "decompress", str(comp_path), str(decomp_path)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
        t2 = time.time()

        output_file = decomp_path / input_path.name
        original = input_path.read_bytes()
        decompressed = output_file.read_bytes()
        if original != decompressed:
            raise RuntimeError("QSC roundtrip mismatch")

        return {
            "compressed_size": comp_path.stat().st_size,
            "compression_time": t1 - t0,
            "decompression_time": t2 - t1,
            "verified": True,
        }
    finally:
        if comp_path.is_file():
            comp_path.unlink()
        if decomp_path.is_dir():
            shutil.rmtree(decomp_path)
        elif decomp_path.exists():
            decomp_path.unlink()


def speed(size, elapsed):
    if elapsed <= 0:
        return 0.0
    return size / elapsed / 1e6


def result_row(path, algorithm, original_size, compressed_size,
               compression_time, decompression_time, verified):
    rel = path.relative_to(DATASET_DIR).as_posix()
    return {
        "corpus": corpus_name(path),
        "file": rel,
        "algorithm": algorithm,
        "original_size": original_size,
        "compressed_size": compressed_size,
        "ratio": compressed_size / original_size if original_size else 0.0,
        "compression_time": compression_time,
        "decompression_time": decompression_time,
        "compression_speed_MBps": speed(original_size, compression_time),
        "decompression_speed_MBps": speed(original_size, decompression_time),
        "verified": verified,
    }


def benchmark_file(path, algos):
    data = path.read_bytes()
    original_size = len(data)
    rows = []

    for name, (compress, decompress) in algos.items():
        try:
            t0 = time.time()
            compressed = compress(data)
            t1 = time.time()
            decompressed = decompress(compressed)
            t2 = time.time()
            verified = decompressed == data
            if not verified:
                raise RuntimeError("roundtrip mismatch")

            rows.append(result_row(
                path, name, original_size, len(compressed),
                t1 - t0, t2 - t1, verified,
            ))
        except Exception as exc:
            print(f"[WARN] {name} failed on {path}: {exc}", file=sys.stderr)

    qsc = run_qsc3(path)
    rows.append(result_row(
        path, "qsc3-v11", original_size, qsc["compressed_size"],
        qsc["compression_time"], qsc["decompression_time"], qsc["verified"],
    ))

    return rows


def write_results(rows):
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "corpus",
        "file",
        "algorithm",
        "original_size",
        "compressed_size",
        "ratio",
        "compression_time",
        "decompression_time",
        "compression_speed_MBps",
        "decompression_speed_MBps",
        "verified",
    ]

    with RESULTS_CSV.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def print_summary(rows):
    by_file = defaultdict(list)
    for row in rows:
        by_file[row["file"]].append(row)

    print("\n=== Best ratio by file ===")
    for file, file_rows in sorted(by_file.items()):
        best = min(file_rows, key=lambda r: r["ratio"])
        print(
            f"{file}: {best['algorithm']} "
            f"{best['compressed_size']} bytes "
            f"({best['ratio']:.6f})"
        )

    qsc_rows = [r for r in rows if r["algorithm"] == "qsc3-v11"]
    total_original = sum(int(r["original_size"]) for r in qsc_rows)
    total_compressed = sum(int(r["compressed_size"]) for r in qsc_rows)
    if total_original:
        print("\n=== QSC3 aggregate ===")
        print(f"files: {len(qsc_rows)}")
        print(f"original bytes: {total_original}")
        print(f"compressed bytes: {total_compressed}")
        print(f"ratio: {total_compressed / total_original:.6f}")


def main():
    if not QSC_BINARY.exists():
        raise SystemExit(f"Missing {QSC_BINARY}; run `make` first.")

    files = discover_files()
    if not files:
        raise SystemExit(f"No benchmark files found under {DATASET_DIR}")

    algos = build_algorithms()
    print(f"Benchmark files: {len(files)}")
    print("Algorithms:", ", ".join(list(algos) + ["qsc3-v11"]))

    rows = []
    for path in files:
        print(f"-> {path.relative_to(DATASET_DIR)}")
        rows.extend(benchmark_file(path, algos))

    write_results(rows)
    print_summary(rows)
    print(f"\nSaved -> {RESULTS_CSV.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
