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
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.188 | 396.740 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.515 | 99.497 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 24.943 | 73.499 |
| qsc3-v7 | 29 | 6,062,277 | 1,491,485 | 0.246027 | 3.203 | 13.312 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 66.324 | 1123.396 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 32.883 | 866.395 |

## Best Ratio By File

| File | Best algorithm | Original bytes | Compressed bytes | Ratio |
| --- | --- | ---: | ---: | ---: |
| calgary/bib | bzip2 | 111,261 | 27,467 | 0.246870 |
| calgary/book1 | bzip2 | 768,771 | 232,598 | 0.302558 |
| calgary/book2 | bzip2 | 610,856 | 157,443 | 0.257742 |
| calgary/geo | qsc3-v7 | 102,400 | 51,710 | 0.504980 |
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

## QSC3 v7 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 32,983 | 0.296447 | 2.548 | 13.690 | True |
| calgary/book1 | 768,771 | 280,878 | 0.365360 | 2.676 | 18.750 | True |
| calgary/book2 | 610,856 | 179,431 | 0.293737 | 1.669 | 20.497 | True |
| calgary/geo | 102,400 | 51,710 | 0.504980 | 2.943 | 7.115 | True |
| calgary/news | 377,109 | 125,491 | 0.332771 | 2.570 | 16.358 | True |
| calgary/obj1 | 21,504 | 10,016 | 0.465774 | 2.546 | 4.460 | True |
| calgary/obj2 | 246,814 | 74,879 | 0.303382 | 8.548 | 15.199 | True |
| calgary/paper1 | 53,161 | 18,200 | 0.342356 | 1.800 | 8.200 | True |
| calgary/paper2 | 82,199 | 28,822 | 0.350637 | 2.512 | 10.602 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 2.293 | 7.901 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 1.164 | 3.033 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 1.064 | 2.924 | True |
| calgary/paper6 | 38,105 | 13,167 | 0.345545 | 2.429 | 6.669 | True |
| calgary/pic | 513,216 | 49,415 | 0.096285 | 11.857 | 40.878 | True |
| calgary/progc | 39,611 | 13,193 | 0.333064 | 2.330 | 7.054 | True |
| calgary/progl | 71,646 | 16,032 | 0.223767 | 3.042 | 11.896 | True |
| calgary/progp | 49,379 | 11,028 | 0.223334 | 2.942 | 9.984 | True |
| calgary/trans | 93,695 | 18,122 | 0.193415 | 5.666 | 14.181 | True |
| canterbury/alice29.txt | 152,089 | 51,114 | 0.336080 | 2.305 | 13.414 | True |
| canterbury/asyoulik.txt | 125,179 | 47,101 | 0.376269 | 2.383 | 11.729 | True |
| canterbury/cp.html | 24,603 | 7,924 | 0.322075 | 1.591 | 5.045 | True |
| canterbury/fields.c | 11,150 | 3,136 | 0.281256 | 1.240 | 2.822 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.610 | 1.041 | True |
| canterbury/kennedy.xls | 1,029,744 | 64,583 | 0.062718 | 4.145 | 47.322 | True |
| canterbury/lcet10.txt | 426,754 | 125,805 | 0.294795 | 2.135 | 19.845 | True |
| canterbury/plrabn12.txt | 481,861 | 175,351 | 0.363904 | 1.506 | 16.813 | True |
| canterbury/ptt5 | 513,216 | 49,416 | 0.096287 | 12.090 | 40.760 | True |
| canterbury/sum | 38,240 | 12,069 | 0.315612 | 3.662 | 6.668 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.607 | 1.205 | True |

## Change Notes

- QSC3 improved from the previous published aggregate of 1,497,090 bytes to 1,491,485 bytes on this corpus.
- Average QSC3 compression speed in this run is 3.203 MB/s; speed varies by transform selection and benchmark process overhead.
- QSC3 v7 now wins the `calgary/geo` file in this benchmark run, but Brotli still has the best aggregate ratio on Canterbury + Calgary.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, and process startup.
- Python baselines run in process on byte buffers.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
