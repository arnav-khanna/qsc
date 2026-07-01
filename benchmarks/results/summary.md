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
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.237 | 424.338 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 7.214 | 105.002 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 26.359 | 76.125 |
| qsc3-v8 | 29 | 6,062,277 | 1,466,441 | 0.241896 | 3.587 | 17.535 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 69.519 | 1191.649 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 34.991 | 919.650 |

## Best Ratio By File

| File | Best algorithm | Original bytes | Compressed bytes | Ratio |
| --- | --- | ---: | ---: | ---: |
| calgary/bib | bzip2 | 111,261 | 27,467 | 0.246870 |
| calgary/book1 | bzip2 | 768,771 | 232,598 | 0.302558 |
| calgary/book2 | bzip2 | 610,856 | 157,443 | 0.257742 |
| calgary/geo | qsc3-v8 | 102,400 | 51,710 | 0.504980 |
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
| canterbury/kennedy.xls | qsc3-v8 | 1,029,744 | 39,539 | 0.038397 |
| canterbury/lcet10.txt | bzip2 | 426,754 | 107,706 | 0.252384 |
| canterbury/plrabn12.txt | bzip2 | 481,861 | 145,577 | 0.302114 |
| canterbury/ptt5 | brotli-default | 513,216 | 40,939 | 0.079770 |
| canterbury/sum | lzma | 38,240 | 9,452 | 0.247176 |
| canterbury/xargs.1 | brotli-default | 4,227 | 1,464 | 0.346345 |

## QSC3 v8 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 32,983 | 0.296447 | 0.195 | 14.082 | True |
| calgary/book1 | 768,771 | 280,878 | 0.365360 | 4.479 | 19.877 | True |
| calgary/book2 | 610,856 | 179,431 | 0.293737 | 2.064 | 22.288 | True |
| calgary/geo | 102,400 | 51,710 | 0.504980 | 3.429 | 8.494 | True |
| calgary/news | 377,109 | 125,491 | 0.332771 | 2.840 | 17.862 | True |
| calgary/obj1 | 21,504 | 10,016 | 0.465774 | 3.106 | 5.211 | True |
| calgary/obj2 | 246,814 | 74,879 | 0.303382 | 9.491 | 17.518 | True |
| calgary/paper1 | 53,161 | 18,200 | 0.342356 | 2.762 | 10.410 | True |
| calgary/paper2 | 82,199 | 28,822 | 0.350637 | 2.705 | 12.420 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 2.586 | 9.335 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 1.423 | 3.972 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 1.312 | 2.994 | True |
| calgary/paper6 | 38,105 | 13,167 | 0.345545 | 2.681 | 8.152 | True |
| calgary/pic | 513,216 | 49,415 | 0.096285 | 12.743 | 46.882 | True |
| calgary/progc | 39,611 | 13,193 | 0.333064 | 2.758 | 9.025 | True |
| calgary/progl | 71,646 | 16,032 | 0.223767 | 3.343 | 15.377 | True |
| calgary/progp | 49,379 | 11,028 | 0.223334 | 3.439 | 12.290 | True |
| calgary/trans | 93,695 | 18,122 | 0.193415 | 6.872 | 18.283 | True |
| canterbury/alice29.txt | 152,089 | 51,114 | 0.336080 | 2.453 | 15.163 | True |
| canterbury/asyoulik.txt | 125,179 | 47,101 | 0.376269 | 2.569 | 13.394 | True |
| canterbury/cp.html | 24,603 | 7,924 | 0.322075 | 1.927 | 6.596 | True |
| canterbury/fields.c | 11,150 | 3,136 | 0.281256 | 1.457 | 3.972 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.781 | 1.540 | True |
| canterbury/kennedy.xls | 1,029,744 | 39,539 | 0.038397 | 4.583 | 117.698 | True |
| canterbury/lcet10.txt | 426,754 | 125,805 | 0.294795 | 2.264 | 21.355 | True |
| canterbury/plrabn12.txt | 481,861 | 175,351 | 0.363904 | 1.822 | 18.129 | True |
| canterbury/ptt5 | 513,216 | 49,416 | 0.096287 | 12.803 | 45.962 | True |
| canterbury/sum | 38,240 | 12,069 | 0.315612 | 4.335 | 8.634 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.800 | 1.613 | True |

## Change Notes

- QSC3 improved from the previous published aggregate of 1,491,485 bytes to 1,466,441 bytes on this corpus.
- Average QSC3 compression speed improved to 3.587 MB/s in this run with the default `-O3` build.
- QSC3 v8 wins `calgary/geo` and `canterbury/kennedy.xls` in this benchmark run, but Brotli still has the best aggregate ratio on Canterbury + Calgary.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, and process startup.
- Python baselines run in process on byte buffers.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
