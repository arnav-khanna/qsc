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
| qsc3-v11 | 29 | 6,062,277 | 1,321,396 | 0.217970 | 0.891 | 13.276 |
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.040 | 506.363 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.712 | 118.571 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 28.476 | 78.704 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 71.492 | 1257.830 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 12.147 | 1038.304 |

## Best Ratio By File

| File | Best algorithm | Original bytes | Compressed bytes | Ratio |
| --- | --- | ---: | ---: | ---: |
| calgary/bib | bzip2 | 111,261 | 27,467 | 0.246870 |
| calgary/book1 | bzip2 | 768,771 | 232,598 | 0.302558 |
| calgary/book2 | bzip2 | 610,856 | 157,443 | 0.257742 |
| calgary/geo | qsc3-v11 | 102,400 | 51,645 | 0.504346 |
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
| canterbury/kennedy.xls | qsc3-v11 | 1,029,744 | 36,918 | 0.035852 |
| canterbury/lcet10.txt | bzip2 | 426,754 | 107,706 | 0.252384 |
| canterbury/plrabn12.txt | bzip2 | 481,861 | 145,577 | 0.302114 |
| canterbury/ptt5 | brotli-default | 513,216 | 40,939 | 0.079770 |
| canterbury/sum | lzma | 38,240 | 9,452 | 0.247176 |
| canterbury/xargs.1 | brotli-default | 4,227 | 1,464 | 0.346345 |

## QSC3 v11 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 28,136 | 0.252883 | 0.161 | 8.748 | True |
| calgary/book1 | 768,771 | 232,926 | 0.302985 | 0.724 | 9.753 | True |
| calgary/book2 | 610,856 | 158,619 | 0.259667 | 1.162 | 10.996 | True |
| calgary/geo | 102,400 | 51,645 | 0.504346 | 2.562 | 7.379 | True |
| calgary/news | 377,109 | 120,423 | 0.319332 | 1.712 | 9.433 | True |
| calgary/obj1 | 21,504 | 10,012 | 0.465588 | 2.509 | 4.561 | True |
| calgary/obj2 | 246,814 | 74,785 | 0.303001 | 7.448 | 15.734 | True |
| calgary/paper1 | 53,161 | 18,196 | 0.342281 | 2.558 | 8.567 | True |
| calgary/paper2 | 82,199 | 25,738 | 0.313118 | 1.889 | 7.154 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 2.344 | 7.791 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 1.176 | 3.055 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 1.106 | 3.039 | True |
| calgary/paper6 | 38,105 | 13,163 | 0.345440 | 2.459 | 7.154 | True |
| calgary/pic | 513,216 | 49,123 | 0.095716 | 0.397 | 42.449 | True |
| calgary/progc | 39,611 | 13,191 | 0.333014 | 2.442 | 7.421 | True |
| calgary/progl | 71,646 | 15,975 | 0.222971 | 2.066 | 12.119 | True |
| calgary/progp | 49,379 | 11,009 | 0.222949 | 2.629 | 9.942 | True |
| calgary/trans | 93,695 | 18,005 | 0.192166 | 5.479 | 15.603 | True |
| canterbury/alice29.txt | 152,089 | 43,945 | 0.288943 | 1.679 | 8.825 | True |
| canterbury/asyoulik.txt | 125,179 | 40,460 | 0.323217 | 1.807 | 7.822 | True |
| canterbury/cp.html | 24,603 | 7,916 | 0.321749 | 1.612 | 5.730 | True |
| canterbury/fields.c | 11,150 | 3,131 | 0.280807 | 1.209 | 3.053 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.641 | 1.080 | True |
| canterbury/kennedy.xls | 1,029,744 | 36,918 | 0.035852 | 2.497 | 117.888 | True |
| canterbury/lcet10.txt | 426,754 | 108,642 | 0.254578 | 1.394 | 10.761 | True |
| canterbury/plrabn12.txt | 481,861 | 146,645 | 0.304331 | 0.962 | 9.673 | True |
| canterbury/ptt5 | 513,216 | 49,124 | 0.095718 | 0.396 | 41.868 | True |
| canterbury/sum | 38,240 | 12,050 | 0.315115 | 2.951 | 6.979 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.666 | 1.352 | True |

## Change Notes

- QSC3 v11 adds a BWT + MTF2 text transform with a direct adaptive arithmetic byte payload, plus a row-XOR + zero-run transform for row-correlated binary/image-like data.
- On Canterbury + Calgary, QSC3 v11 improves aggregate compressed size from 1,459,754 bytes to 1,321,396 bytes.
- QSC3 v11 beats Brotli default on aggregate ratio in this benchmark: 0.217970 vs Brotli 0.222249.
- QSC3 v11 is slower than Brotli in total compression throughput in this run: 0.891 MB/s vs Brotli 1.040 MB/s.
- QSC3 v11 wins `calgary/geo` and `canterbury/kennedy.xls` by per-file ratio; the aggregate win comes mainly from stronger text compression on `book1`, `book2`, `bib`, and Canterbury text files.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, process startup, and output-directory cleanup.
- Python baselines run in process on byte buffers.
- Brotli uses the Python library default settings, not an exhaustive Brotli level sweep.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
