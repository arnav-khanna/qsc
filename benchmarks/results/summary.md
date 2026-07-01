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
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.243 | 417.808 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.962 | 105.178 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 26.085 | 77.500 |
| qsc3-v6 | 29 | 6,062,277 | 1,497,090 | 0.246952 | 3.204 | 14.166 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 72.096 | 1240.738 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 34.428 | 875.178 |

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

## QSC3 v6 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 32,962 | 0.296258 | 0.175 | 13.250 | True |
| calgary/book1 | 768,771 | 279,798 | 0.363955 | 3.470 | 20.088 | True |
| calgary/book2 | 610,856 | 179,230 | 0.293408 | 1.891 | 22.524 | True |
| calgary/geo | 102,400 | 59,208 | 0.578203 | 3.219 | 7.386 | True |
| calgary/news | 377,109 | 125,472 | 0.332721 | 2.723 | 17.892 | True |
| calgary/obj1 | 21,504 | 10,016 | 0.465774 | 2.877 | 4.578 | True |
| calgary/obj2 | 246,814 | 74,862 | 0.303313 | 9.397 | 16.954 | True |
| calgary/paper1 | 53,161 | 18,197 | 0.342300 | 2.778 | 9.133 | True |
| calgary/paper2 | 82,199 | 28,813 | 0.350527 | 2.509 | 11.363 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 2.462 | 8.442 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 1.274 | 3.533 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 1.175 | 3.153 | True |
| calgary/paper6 | 38,105 | 13,167 | 0.345545 | 2.442 | 7.594 | True |
| calgary/pic | 513,216 | 49,409 | 0.096273 | 10.736 | 42.758 | True |
| calgary/progc | 39,611 | 13,193 | 0.333064 | 2.432 | 7.223 | True |
| calgary/progl | 71,646 | 16,025 | 0.223669 | 3.031 | 12.484 | True |
| calgary/progp | 49,379 | 11,024 | 0.223253 | 3.032 | 9.695 | True |
| calgary/trans | 93,695 | 18,098 | 0.193159 | 5.984 | 15.570 | True |
| canterbury/alice29.txt | 152,089 | 51,093 | 0.335941 | 2.324 | 13.883 | True |
| canterbury/asyoulik.txt | 125,179 | 47,084 | 0.376133 | 2.495 | 12.109 | True |
| canterbury/cp.html | 24,603 | 7,924 | 0.322075 | 1.716 | 5.380 | True |
| canterbury/fields.c | 11,150 | 3,136 | 0.281256 | 1.202 | 2.995 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.623 | 1.186 | True |
| canterbury/kennedy.xls | 1,029,744 | 64,581 | 0.062716 | 4.586 | 51.446 | True |
| canterbury/lcet10.txt | 426,754 | 125,683 | 0.294509 | 2.156 | 21.302 | True |
| canterbury/plrabn12.txt | 481,861 | 175,020 | 0.363217 | 1.554 | 18.037 | True |
| canterbury/ptt5 | 513,216 | 49,410 | 0.096275 | 10.171 | 41.796 | True |
| canterbury/sum | 38,240 | 12,066 | 0.315533 | 3.810 | 7.716 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.681 | 1.342 | True |

## Change Notes

- QSC3 improved from the previous published aggregate of 1,507,747 bytes to 1,497,090 bytes on this corpus.
- Average QSC3 compression speed improved from the previous published 2.452 MB/s to 3.204 MB/s in this benchmark harness.
- Brotli still has the best aggregate ratio on Canterbury + Calgary in this run; QSC3 is not yet a Brotli-beating general-purpose codec.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, and process startup.
- Python baselines run in process on byte buffers.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
