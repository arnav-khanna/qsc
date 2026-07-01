# Benchmark Results

Generated with `python3 benchmarks/run.py`.

## Environment

- Git revision: see repository commit history for this file
- Python: 3.9.6
- Platform: macOS-26.3-arm64-arm-64bit
- Corpora: Canterbury and Calgary
- Benchmark files: 29

## Aggregate By Algorithm

| Algorithm | Files | Original bytes | Compressed bytes | Ratio | Total-time compress MB/s | Total-time decompress MB/s |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.013 | 505.719 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.459 | 114.151 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 27.483 | 74.134 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 70.042 | 1390.367 |
| qsc3-v9 | 29 | 6,062,277 | 1,686,590 | 0.278211 | 9.601 | 22.632 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 11.688 | 998.274 |

## Best Ratio By File

| File | Best algorithm | Original bytes | Compressed bytes | Ratio |
| --- | --- | ---: | ---: | ---: |
| calgary/bib | bzip2 | 111,261 | 27,467 | 0.246870 |
| calgary/book1 | bzip2 | 768,771 | 232,598 | 0.302558 |
| calgary/book2 | bzip2 | 610,856 | 157,443 | 0.257742 |
| calgary/geo | qsc3-v9 | 102,400 | 51,934 | 0.507168 |
| calgary/news | brotli-default | 377,109 | 112,968 | 0.299563 |
| calgary/obj1 | brotli-default | 21,504 | 9,341 | 0.434384 |
| calgary/obj2 | lzma | 246,814 | 61,504 | 0.249192 |
| calgary/paper1 | brotli-default | 53,161 | 15,457 | 0.290758 |
| calgary/paper2 | brotli-default | 82,199 | 24,850 | 0.302315 |
| calgary/paper3 | brotli-default | 46,526 | 14,639 | 0.314641 |
| calgary/paper4 | brotli-default | 13,286 | 4,283 | 0.322369 |
| calgary/paper5 | brotli-default | 11,954 | 4,073 | 0.340723 |
| calgary/paper6 | brotli-default | 38,105 | 11,135 | 0.292219 |
| calgary/pic | brotli-default | 513,216 | 40,939 | 0.079770 |
| calgary/progc | brotli-default | 39,611 | 11,619 | 0.293328 |
| calgary/progl | brotli-default | 71,646 | 14,003 | 0.195447 |
| calgary/progp | brotli-default | 49,379 | 9,880 | 0.200085 |
| calgary/trans | brotli-default | 93,695 | 15,402 | 0.164384 |
| canterbury/alice29.txt | bzip2 | 152,089 | 43,202 | 0.284057 |
| canterbury/asyoulik.txt | bzip2 | 125,179 | 39,569 | 0.316099 |
| canterbury/cp.html | brotli-default | 24,603 | 6,894 | 0.280210 |
| canterbury/fields.c | brotli-default | 11,150 | 2,717 | 0.243677 |
| canterbury/grammar.lsp | brotli-default | 3,721 | 1,124 | 0.302069 |
| canterbury/kennedy.xls | qsc3-v9 | 1,029,744 | 42,234 | 0.041014 |
| canterbury/lcet10.txt | bzip2 | 426,754 | 107,706 | 0.252384 |
| canterbury/plrabn12.txt | bzip2 | 481,861 | 145,577 | 0.302114 |
| canterbury/ptt5 | brotli-default | 513,216 | 40,939 | 0.079770 |
| canterbury/sum | lzma | 38,240 | 9,452 | 0.247176 |
| canterbury/xargs.1 | brotli-default | 4,227 | 1,464 | 0.346345 |

## QSC3 v9 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 34,480 | 0.309902 | 5.365 | 11.983 | True |
| calgary/book1 | 768,771 | 435,765 | 0.566833 | 14.401 | 171.843 | True |
| calgary/book2 | 610,856 | 182,936 | 0.299475 | 6.581 | 20.172 | True |
| calgary/geo | 102,400 | 51,934 | 0.507168 | 6.414 | 7.267 | True |
| calgary/news | 377,109 | 126,774 | 0.336173 | 6.600 | 16.062 | True |
| calgary/obj1 | 21,504 | 12,756 | 0.593192 | 5.588 | 8.991 | True |
| calgary/obj2 | 246,814 | 98,312 | 0.398324 | 22.387 | 72.590 | True |
| calgary/paper1 | 53,161 | 19,279 | 0.362653 | 4.728 | 8.394 | True |
| calgary/paper2 | 82,199 | 29,465 | 0.358459 | 5.071 | 10.397 | True |
| calgary/paper3 | 46,526 | 19,195 | 0.412565 | 4.271 | 7.432 | True |
| calgary/paper4 | 13,286 | 5,612 | 0.422400 | 1.663 | 3.324 | True |
| calgary/paper5 | 11,954 | 5,079 | 0.424879 | 1.441 | 2.740 | True |
| calgary/paper6 | 38,105 | 14,243 | 0.373783 | 4.126 | 7.327 | True |
| calgary/pic | 513,216 | 49,452 | 0.096357 | 31.662 | 41.140 | True |
| calgary/progc | 39,611 | 14,077 | 0.355381 | 4.402 | 7.292 | True |
| calgary/progl | 71,646 | 16,765 | 0.233998 | 6.368 | 12.298 | True |
| calgary/progp | 49,379 | 11,810 | 0.239170 | 5.504 | 9.625 | True |
| calgary/trans | 93,695 | 25,114 | 0.268040 | 18.032 | 37.538 | True |
| canterbury/alice29.txt | 152,089 | 51,756 | 0.340301 | 5.814 | 13.114 | True |
| canterbury/asyoulik.txt | 125,179 | 47,404 | 0.378690 | 5.420 | 11.366 | True |
| canterbury/cp.html | 24,603 | 10,353 | 0.420802 | 2.316 | 5.186 | True |
| canterbury/fields.c | 11,150 | 5,538 | 0.496682 | 1.495 | 2.790 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.697 | 1.111 | True |
| canterbury/kennedy.xls | 1,029,744 | 42,234 | 0.041014 | 51.451 | 103.881 | True |
| canterbury/lcet10.txt | 426,754 | 128,030 | 0.300009 | 6.787 | 19.256 | True |
| canterbury/plrabn12.txt | 481,861 | 179,340 | 0.372182 | 6.020 | 16.441 | True |
| canterbury/ptt5 | 513,216 | 49,453 | 0.096359 | 30.667 | 39.667 | True |
| canterbury/sum | 38,240 | 16,291 | 0.426020 | 8.403 | 15.521 | True |
| canterbury/xargs.1 | 4,227 | 1,834 | 0.433877 | 0.822 | 1.188 | True |

## Change Notes

- QSC3 v9 switches to a practical-speed profile: capped match search, single-path transform selection, direct match-copy decompression, and a byte-aligned fast payload for raw fallback chunks.
- On Canterbury + Calgary, QSC3 v9 compresses faster than Python `lzma` in total-time throughput: 9.601 MB/s vs 6.459 MB/s in this run.
- QSC3 v9 does not beat LZMA ratio or decompression speed on this corpus. Aggregate QSC3 v9 ratio is 0.278211 vs LZMA 0.227306, and QSC3 v9 decompression is 22.632 MB/s vs LZMA 114.151 MB/s.
- QSC3 v9 still beats zlib-9 ratio on this corpus: 0.278211 vs 0.294853.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, process startup, and output-directory cleanup.
- Python baselines run in process on byte buffers.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
