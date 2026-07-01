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
| brotli-default | 29 | 6,062,277 | 1,347,334 | 0.222249 | 1.048 | 503.815 |
| lzma | 29 | 6,062,277 | 1,377,992 | 0.227306 | 6.768 | 118.525 |
| bzip2 | 29 | 6,062,277 | 1,409,211 | 0.232456 | 28.700 | 77.904 |
| qsc3-v10 | 29 | 6,062,277 | 1,459,754 | 0.240793 | 2.222 | 19.472 |
| zstd-10 | 29 | 6,062,277 | 1,573,971 | 0.259634 | 70.072 | 1253.304 |
| zlib-9 | 29 | 6,062,277 | 1,787,481 | 0.294853 | 12.248 | 1082.646 |

## Best Ratio By File

| File | Best algorithm | Original bytes | Compressed bytes | Ratio |
| --- | --- | ---: | ---: | ---: |
| calgary/bib | bzip2 | 111,261 | 27,467 | 0.246870 |
| calgary/book1 | bzip2 | 768,771 | 232,598 | 0.302558 |
| calgary/book2 | bzip2 | 610,856 | 157,443 | 0.257742 |
| calgary/geo | qsc3-v10 | 102,400 | 51,645 | 0.504346 |
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
| canterbury/kennedy.xls | qsc3-v10 | 1,029,744 | 36,918 | 0.035852 |
| canterbury/lcet10.txt | bzip2 | 426,754 | 107,706 | 0.252384 |
| canterbury/plrabn12.txt | bzip2 | 481,861 | 145,577 | 0.302114 |
| canterbury/ptt5 | brotli-default | 513,216 | 40,939 | 0.079770 |
| canterbury/sum | lzma | 38,240 | 9,452 | 0.247176 |
| canterbury/xargs.1 | brotli-default | 4,227 | 1,464 | 0.346345 |

## QSC3 v10 Results

| File | Original bytes | Compressed bytes | Ratio | Compress MB/s | Decompress MB/s | Verified |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| calgary/bib | 111,261 | 32,939 | 0.296052 | 2.467 | 13.942 | True |
| calgary/book1 | 768,771 | 278,722 | 0.362555 | 1.669 | 19.607 | True |
| calgary/book2 | 610,856 | 179,043 | 0.293102 | 1.355 | 21.739 | True |
| calgary/geo | 102,400 | 51,645 | 0.504346 | 2.646 | 7.766 | True |
| calgary/news | 377,109 | 125,372 | 0.332456 | 2.070 | 16.698 | True |
| calgary/obj1 | 21,504 | 10,012 | 0.465588 | 2.052 | 4.340 | True |
| calgary/obj2 | 246,814 | 74,785 | 0.303001 | 7.684 | 15.412 | True |
| calgary/paper1 | 53,161 | 18,196 | 0.342281 | 2.545 | 8.735 | True |
| calgary/paper2 | 82,199 | 28,812 | 0.350515 | 2.568 | 10.911 | True |
| calgary/paper3 | 46,526 | 17,854 | 0.383742 | 2.363 | 7.700 | True |
| calgary/paper4 | 13,286 | 5,572 | 0.419389 | 1.243 | 3.406 | True |
| calgary/paper5 | 11,954 | 5,053 | 0.422704 | 1.145 | 2.813 | True |
| calgary/paper6 | 38,105 | 13,163 | 0.345440 | 2.424 | 7.039 | True |
| calgary/pic | 513,216 | 49,410 | 0.096275 | 27.354 | 41.123 | True |
| calgary/progc | 39,611 | 13,191 | 0.333014 | 2.475 | 7.631 | True |
| calgary/progl | 71,646 | 15,975 | 0.222971 | 2.906 | 12.411 | True |
| calgary/progp | 49,379 | 11,009 | 0.222949 | 2.638 | 9.684 | True |
| calgary/trans | 93,695 | 18,005 | 0.192166 | 4.107 | 14.750 | True |
| canterbury/alice29.txt | 152,089 | 51,089 | 0.335915 | 2.204 | 13.865 | True |
| canterbury/asyoulik.txt | 125,179 | 47,080 | 0.376101 | 2.490 | 12.096 | True |
| canterbury/cp.html | 24,603 | 7,916 | 0.321749 | 1.689 | 6.017 | True |
| canterbury/fields.c | 11,150 | 3,131 | 0.280807 | 1.287 | 2.871 | True |
| canterbury/grammar.lsp | 3,721 | 1,309 | 0.351787 | 0.617 | 1.119 | True |
| canterbury/kennedy.xls | 1,029,744 | 36,918 | 0.035852 | 2.555 | 113.358 | True |
| canterbury/lcet10.txt | 426,754 | 125,517 | 0.294120 | 1.673 | 20.737 | True |
| canterbury/plrabn12.txt | 481,861 | 174,744 | 0.362644 | 1.069 | 17.193 | True |
| canterbury/ptt5 | 513,216 | 49,411 | 0.096277 | 27.906 | 42.227 | True |
| canterbury/sum | 38,240 | 12,050 | 0.315115 | 2.399 | 7.000 | True |
| canterbury/xargs.1 | 4,227 | 1,831 | 0.433168 | 0.657 | 1.295 | True |

## Change Notes

- QSC3 v10 restores ratio-first arithmetic-coded transform selection, raises LZ search depth to 32,768, and adds early exits for very strong structural transforms.
- On Canterbury + Calgary, QSC3 v10 improves from the previous v8/v10 baseline of 1,466,441 bytes to 1,459,754 bytes.
- QSC3 v10 compresses faster than Brotli in this command-line benchmark run: 2.222 MB/s vs Brotli 1.048 MB/s.
- QSC3 v10 still does not beat Brotli ratio: 0.240793 vs Brotli 0.222249.
- QSC3 v10 wins `calgary/geo` and `canterbury/kennedy.xls` by ratio in this corpus.

## Caveats

- QSC is measured through the command-line binary, including archive framing, file I/O, process startup, and output-directory cleanup.
- Python baselines run in process on byte buffers.
- These results are useful for transparency and development tracking, but they are not a perfectly fair production-codec shootout.
