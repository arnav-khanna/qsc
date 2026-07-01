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
| qsc3-v11 | 29 | 6,062,277 | 1,321,396 | 0.217970 | 1.398 | 12.838 |
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.040 | 488.831 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.572 | 119.184 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 28.093 | 75.767 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 69.924 | 1274.602 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 12.018 | 1014.201 |

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
| calgary/bib | 111,261 | 28,136 | 0.252883 | 0.183 | 6.438 | True |
| calgary/book1 | 768,771 | 232,926 | 0.302985 | 7.156 | 9.621 | True |
| calgary/book2 | 610,856 | 158,619 | 0.259667 | 7.515 | 10.844 | True |
| calgary/geo | 102,400 | 51,645 | 0.504346 | 2.557 | 7.217 | True |
| calgary/news | 377,109 | 120,423 | 0.319332 | 6.238 | 9.404 | True |
| calgary/obj1 | 21,504 | 10,012 | 0.465588 | 2.436 | 4.047 | True |
| calgary/obj2 | 246,814 | 74,785 | 0.303001 | 7.445 | 15.341 | True |
| calgary/paper1 | 53,161 | 18,196 | 0.342281 | 2.547 | 8.611 | True |
| calgary/paper2 | 82,199 | 25,738 | 0.313118 | 5.855 | 6.993 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 2.301 | 7.882 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 1.141 | 3.521 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 1.068 | 2.984 | True |
| calgary/paper6 | 38,105 | 13,163 | 0.345440 | 2.332 | 7.188 | True |
| calgary/pic | 513,216 | 49,123 | 0.095716 | 0.401 | 41.670 | True |
| calgary/progc | 39,611 | 13,191 | 0.333014 | 2.469 | 7.025 | True |
| calgary/progl | 71,646 | 15,975 | 0.222971 | 2.107 | 11.790 | True |
| calgary/progp | 49,379 | 11,009 | 0.222949 | 2.586 | 9.629 | True |
| calgary/trans | 93,695 | 18,005 | 0.192166 | 5.415 | 15.151 | True |
| canterbury/alice29.txt | 152,089 | 43,945 | 0.288943 | 6.409 | 8.565 | True |
| canterbury/asyoulik.txt | 125,179 | 40,460 | 0.323217 | 5.981 | 7.748 | True |
| canterbury/cp.html | 24,603 | 7,916 | 0.321749 | 1.616 | 4.900 | True |
| canterbury/fields.c | 11,150 | 3,131 | 0.280807 | 1.169 | 2.888 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.587 | 1.121 | True |
| canterbury/kennedy.xls | 1,029,744 | 36,918 | 0.035852 | 2.468 | 100.570 | True |
| canterbury/lcet10.txt | 426,754 | 108,642 | 0.254578 | 7.667 | 10.495 | True |
| canterbury/plrabn12.txt | 481,861 | 146,645 | 0.304331 | 7.021 | 9.350 | True |
| canterbury/ptt5 | 513,216 | 49,124 | 0.095718 | 0.396 | 38.945 | True |
| canterbury/sum | 38,240 | 12,050 | 0.315115 | 2.977 | 7.054 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.605 | 1.180 | True |

## Change Notes

- QSC3 v11 adds a large-text fast decision path: when BWT + MTF2 direct arithmetic coding is already clearly strong, the encoder skips losing raw LZ and dictionary candidates.
- QSC3 v11 keeps the BWT + MTF2 text transform with a direct adaptive arithmetic byte payload, plus a row-XOR + zero-run transform for row-correlated binary/image-like data.
- On Canterbury + Calgary, QSC3 v11 improves aggregate compressed size from 1,459,754 bytes to 1,321,396 bytes.
- QSC3 v11 beats Brotli default on aggregate ratio in this benchmark: 0.217970 vs Brotli 0.222249.
- QSC3 v11 also beats Brotli default on total compression throughput in this run: 1.398 MB/s vs Brotli 1.040 MB/s.
- QSC3 v11 decompression remains much slower than Brotli: 12.838 MB/s vs Brotli 488.831 MB/s.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, process startup, and output-directory cleanup.
- Python baselines run in process on byte buffers.
- Brotli uses the Python library default settings, not an exhaustive Brotli level sweep.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
