# Benchmark Results

Generated with `python3 benchmarks/run.py`.

## Environment

- Git revision: see repository commit history for this file
- Python: 3.9.6
- Platform: macOS-26.3-arm64-arm-64bit
- Corpora: Canterbury and Calgary
- Benchmark files: 29

## Aggregate By Algorithm

| Algorithm | Files | Original bytes | Compressed bytes | Ratio | Avg compress MB/s | Avg decompress MB/s |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.247 | 428.419 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 25.923 | 76.005 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.895 | 103.156 |
| qsc3-v5 | 29 | 6,062,277 | 1,507,747 | 0.248710 | 2.452 | 14.095 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 33.245 | 886.664 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 70.520 | 1250.772 |

## Best Ratio By File

| File | Best algorithm | Original bytes | Compressed bytes | Ratio |
| --- | --- | ---: | ---: | ---: |
| calgary/bib | bzip2 | 111,261 | 27,467 | 0.246870 |
| calgary/book1 | bzip2 | 768,771 | 232,598 | 0.302558 |
| calgary/book2 | bzip2 | 610,856 | 157,443 | 0.257742 |
| calgary/geo | brotli-default | 102,400 | 52,915 | 0.516748 |
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
| canterbury/kennedy.xls | lzma | 1,029,744 | 49,116 | 0.047697 |
| canterbury/lcet10.txt | bzip2 | 426,754 | 107,706 | 0.252384 |
| canterbury/plrabn12.txt | bzip2 | 481,861 | 145,577 | 0.302114 |
| canterbury/ptt5 | brotli-default | 513,216 | 40,939 | 0.079770 |
| canterbury/sum | lzma | 38,240 | 9,452 | 0.247176 |
| canterbury/xargs.1 | brotli-default | 4,227 | 1,464 | 0.346345 |

## QSC3 v5 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 32,949 | 0.296142 | 0.296 | 13.339 | True |
| calgary/book1 | 768,771 | 279,150 | 0.363112 | 2.136 | 20.064 | True |
| calgary/book2 | 610,856 | 179,149 | 0.293275 | 0.559 | 21.927 | True |
| calgary/geo | 102,400 | 64,649 | 0.631338 | 5.466 | 6.592 | True |
| calgary/news | 377,109 | 125,452 | 0.332668 | 0.631 | 17.166 | True |
| calgary/obj1 | 21,504 | 10,016 | 0.465774 | 3.725 | 4.104 | True |
| calgary/obj2 | 246,814 | 74,853 | 0.303277 | 8.432 | 15.174 | True |
| calgary/paper1 | 53,161 | 18,197 | 0.342300 | 0.586 | 9.095 | True |
| calgary/paper2 | 82,199 | 28,814 | 0.350540 | 0.554 | 11.271 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 0.600 | 8.121 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 0.519 | 3.386 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 0.495 | 3.006 | True |
| calgary/paper6 | 38,105 | 13,167 | 0.345545 | 0.590 | 7.762 | True |
| calgary/pic | 513,216 | 52,488 | 0.102273 | 11.558 | 44.466 | True |
| calgary/progc | 39,611 | 13,193 | 0.333064 | 0.575 | 7.824 | True |
| calgary/progl | 71,646 | 16,024 | 0.223655 | 0.628 | 13.436 | True |
| calgary/progp | 49,379 | 11,028 | 0.223334 | 0.653 | 9.556 | True |
| calgary/trans | 93,695 | 18,086 | 0.193031 | 9.801 | 14.927 | True |
| canterbury/alice29.txt | 152,089 | 51,091 | 0.335928 | 0.648 | 14.275 | True |
| canterbury/asyoulik.txt | 125,179 | 47,232 | 0.377316 | 0.031 | 12.681 | True |
| canterbury/cp.html | 24,603 | 7,924 | 0.322075 | 0.594 | 5.876 | True |
| canterbury/fields.c | 11,150 | 3,136 | 0.281256 | 0.525 | 2.993 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.366 | 1.195 | True |
| canterbury/kennedy.xls | 1,029,744 | 64,583 | 0.062718 | 3.570 | 49.612 | True |
| canterbury/lcet10.txt | 426,754 | 125,585 | 0.294280 | 0.601 | 20.526 | True |
| canterbury/plrabn12.txt | 481,861 | 174,808 | 0.362777 | 0.511 | 17.687 | True |
| canterbury/ptt5 | 513,216 | 52,489 | 0.102275 | 11.206 | 43.933 | True |
| canterbury/sum | 38,240 | 12,065 | 0.315507 | 4.873 | 7.505 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.375 | 1.265 | True |

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, and process startup.
- Python baselines run in process on byte buffers.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
