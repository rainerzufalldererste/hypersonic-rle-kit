  <a href="https://github.com/rainerzufalldererste/rle8"><img src="https://raw.githubusercontent.com/rainerzufalldererste/rle8/master/docs/logo.png" alt="hypersonic rle kit logo" style="width: 533pt; max-width: 100%"></a>
  <br>

### What is it?
- A selection of 100+ RLE and related codecs optimized for all kinds of different inputs and scenarios.
- Possibly the fastest run length en/decoder (obviously dependent on the dataset). **Single Core Decompression Speeds > 33 GiB/s have been observed.**
- Written in C.
- SIMD Variants for AVX-512F, AVX2, AVX, SSE4.1, SSSE3, SSE2 and SSE variants are available for various decoders and encoders. Automatically picked at runtime based on the extensions available on the current platform.
- Variants include: Single RLE Symbol, Short Strings of RLE Symbols, Byte Alignmed, Symbol Aligned, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit, 128 Bit, Different probabilities of reoccuring symbols, ...
- `OpenCL` variant available for some of the decoders.

### Benchmark
 See [Full Benchmark with Graphs](https://raw.githubusercontent.com/rainerzufalldererste/rle8/master/docs/index.html), the tables below only contain a tiny selection of the 100+ codecs.
 
 - Single-Threaded
 - Running on an `AMD Ryzen 9 7950X`, `32 GB DDR5-6000 CL30` on Windows 11.
 - Compiled with `Visual Studio 2022`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) and [MRLE](https://encode.su/threads/2121-No-more-encoding-overhead-in-Run-Length-Encoding-Read-about-Mespotine-RLE-here-) (with AVX2 enabled; benchmarking results have been converted from MB/s to MiB/s)
 - Contained Codecs w/ Compression / Decompression Speed vs. Ratio Pareto + Notable Entropy Highlighted

#### [video-frame.raw](https://www.dropbox.com/s/yvsl1lg98c4maq1/video_frame.raw?dl=1) (heavily quantized video frame DCTs, 88,473,600 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
|   8 Bit                            |  **19.34 %** | **25,830.4 MiB/s** | **3,319.6 MiB/s** |    9.50 %   |
|   8 Bit Packed                     |  **17.95 %** | **19,783.6 MiB/s** | **2,907.5 MiB/s** |    9.69 %   |
|   8 Bit 1LUT Short                 |  **16.93 %** | **11,045.0 MiB/s** | **2,365.1 MiB/s** |    9.87 %   |
|   8 Bit 3LUT Short                 |  **16.49 %** |  **8,100.4 MiB/s** | **2,085.6 MiB/s** |    9.93 %   |
|   8 Bit 7LUT                       |  **17.46 %** | **16,100.9 MiB/s** |   2,094.6 MiB/s   |    9.94 %   |
|   8 Bit Single                     |  **18.31 %** | **23,750.2 MiB/s** |   1,654.7 MiB/s   |    9.71 %   |
|   8 Bit Single Short               |  **17.05 %** | **14,143.6 MiB/s** |   1,393.2 MiB/s   |    9.81 %   |
|   16 Bit 3LUT (Symbol)             |  **18.16 %** | **15,025.4 MiB/s** |     941.9 MiB/s   |    9.82 %   |
|   48 Bit (Byte)                    |  **22.66 %** |   26,481.4 MiB/s   | **3,329.7 MiB/s** |    9.63 %   |
|   48 Bit Packed (Byte)             |  **20.84 %** | **25,920.9 MiB/s** |   3,009.6 MiB/s   |    9.40 %   |
|   64 Bit 3LUT (Symbol)             |  **22.13 %** | **26,928.5 MiB/s** |   1,223.4 MiB/s   |    9.24 %   |
|   64 Bit (Byte)                    |  **23.66 %** |   25,360.7 MiB/s   | **3,497.4 MiB/s** |    9.63 %   |
|   8 Bit RLE + Huffman-esque        |  **12.51 %** |  **1,676.7 MiB/s** |  **,332.0 MiB/s** |    9.82 %   |
|   Low Entropy                      |    19.93 %   |    2,690.6 MiB/s   |     800.2 MiB/s   |  **9.05 %** |
|   Low Entropy Single               |    19.96 %   |    4,389.0 MiB/s   |     775.6 MiB/s   |  **9.10 %** |
|   Multi MTF 128 Bit (Transform)    |   100.00 %   |    3,642.0 MiB/s   |   3,352.0 MiB/s   |   15.63 %   |
|   Multi MTF 256 Bit (Transform)    |   100.00 %   |    5,630.4 MiB/s   |   5,472.6 MiB/s   |   15.88 %   |
| - | - | - | - | - | 
| memcpy                             |    100.0 %   |   28,510.8 MiB/s   |  28,689.2 MiB/s   |   14.03 %   |
| trle    | 16.0 % | 1,124.6 MiB/s |  4,109.9 MiB/s | - |
| srle 0  | 17.8 % | 1,107.6 MiB/s |  9,175.2 MiB/s | - |
| srle 8  | 18.7 % | 1,801.2 MiB/s |  9,198.3 MiB/s | - |
| srle 16 | 21.3 % | 1,649.2 MiB/s | 10,667.0 MiB/s | - |
| srle 32 | 24.2 % | 2,950.6 MiB/s | 14,922.9 MiB/s | - |
| srle 64 | 27.5 % | 5,003.2 MiB/s | 20,526.3 MiB/s | - |
| mrle    | 19.7 % |   427.6 MiB/s |  2,570.0 MiB/s | - |

#### [1034.db](http://encode.su/threads/2077-EGTB-compression?p=41392&viewfull=1#post41392) (Checkers program "End Game Table Base", 419,225,625 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| 8 Bit                         |  **23.02 %** | **21,583.8 MiB/s** |  **2,443.8 MiB/s** |   12.08 %   |
| 8 Bit Packed                  |  **20.10 %** | **15,151.8 MiB/s** |  **2,199.4 MiB/s** |   12.74 %   |
| 8 Bit 1LUT Short              |  **18.91 %** |    7,855.4 MiB/s   |  **1,767.7 MiB/s** |   12.46 %   |
| 8 Bit 3LUT                    |  **19.77 %** |   14,211.6 MiB/s   |  **1,841.7 MiB/s** |   12.31 %   |
| 8 Bit 7LUT                    |  **19.75 %** | **14,629.5 MiB/s** |  **1,768.7 MiB/s** |   12.50 %   |
| 8 Bit Single                  |  **20.59 %** | **18,469.7 MiB/s** |    1,325.0 MiB/s   |   12.06 %   |
| 8 Bit Single Short            |  **18.55 %** |  **9,756.7 MiB/s** |    1,117.8 MiB/s   |   12.09 %   |
| 16 Bit 3LUT (Byte)            |  **20.20 %** | **15,500.6 MiB/s** |      893.8 MiB/s   |   12.30 %   |
| 32 Bit (Byte)                 |  **27.15 %** |   17,496.2 MiB/s   |  **2,057.2 MiB/s** |   12.40 %   |
| 32 Bit 3LUT (Byte)            |  **22.86 %** | **20,829.1 MiB/s** |    1,053.9 MiB/s   |   11.79 %   |
| 48 Bit (Byte)                 |  **29.76 %** |   22,123.2 MiB/s   |  **2,612.1 MiB/s** |   12.19 %   |
| 48 Bit Packed (Byte)          |  **25.87 %** | **22,210.4 MiB/s** |  **2,200.6 MiB/s** |   11.95 %   |
| 64 Bit (Byte)                 |  **31.66 %** |   21,036.6 MiB/s   |  **2,742.4 MiB/s** |   12.16 %   |
| 64 Bit Packed (Byte)          |  **27.29 %** |   22,019.2 MiB/s   |  **2,246.0 MiB/s** |   11.90 %   |
| 64 Bit 3LUT (Byte)            |  **26.90 %** | **23,777.1 MiB/s** |    1,039.7 MiB/s   |   11.63 %   |
| 128 Bit (Byte)                |  **37.25 %** | **24,011.4 MiB/s** |    2,489.4 MiB/s   |   12.28 %   |
| 128 Bit Packed (Byte)         |  **31.21 %** | **23,977.4 MiB/s** |    2,293.4 MiB/s   |   11.98 %   |
| 8 Bit RLE + Huffman-esque     |  **16.76 %** |  **1,456.9 MiB/s** |   **,326.5 MiB/s** |   12.36 %   |
| 8 Bit MMTF 128                |    26.61 %   |    2,819.2 MiB/s   |    2,118.5 MiB/s   |   14.85 %   |
| Low Entropy                   |    21.15 %   |    4,282.1 MiB/s   |      799.7 MiB/s   | **10.37 %** |
| Low Entropy Single            |    21.15 %   |    4,292.2 MiB/s   |      718.2 MiB/s   | **10.37 %** |
| Multi MTF 128 Bit (Transform) |   100.00 %   |    2,666.7 MiB/s   |    2,539.3 MiB/s   |   17.90 %   |
| Multi MTF 256 Bit (Transform) |   100.00 %   |    4,094.2 MiB/s   |    4,064.7 MiB/s   |   18.35 %   |
| - | - | - | - | - |
| memcpy                        |  100.0 %   |  27,165.1 MiB/s   |   27,100.4 MiB/s   |   16.76 %   |
| trle    | 17.4 % | 1,137.2 MiB/s |   4,291.4 MiB/s | - |
| srle 0  | 20.2 % | 1,130.2 MiB/s |   9,113.0 MiB/s | - |
| srle 8  | 22.0 % | 1,863.1 MiB/s |   9,092.7 MiB/s | - |
| srle 16 | 27.1 % | 1,657.0 MiB/s |  10,108.9 MiB/s | - |
| srle 32 | 32.7 % | 2,910.3 MiB/s |  13,160.2 MiB/s | - |
| srle 64 | 39.5 % | 4,990.2 MiB/s |  19,314.2 MiB/s | - |
| mrle    | 21.0 % |   461.4 MiB/s |   3,182.7 MiB/s | - |

#### `enwik9.bwt` (Wikipedia extract [enwiki9](http://mattmahoney.net/dc/textdata.html) encoded using [libdivsufsort](https://github.com/y-256/libdivsufsort), 1,000,000,124 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| 8 Bit                            |  **48.80 %** | **16,097.6  MiB/s** |  **1,219.7 MiB/s** |   34.13 %   |
| 8 Bit Short                      |  **44.42 %** |  **6,975.0  MiB/s** |   **,899.3 MiB/s** |   32.26 %   |
| 8 Bit Packed                     |  **44.86 %** |  **7,514.9  MiB/s** |  **1,038.7 MiB/s** |   32.95 %   |
| 8 Bit 1LUT Short                 |  **41.87 %** |  **4,088.0  MiB/s** |    **815.0 MiB/s** |   31.38 %   |
| 8 Bit 3LUT                       |  **43.29 %** |  **6,116.6  MiB/s** |      775.1 MiB/s   |   32.27 %   |
| 8 Bit 3LUT Short                 |  **40.34 %** |  **3,043.5  MiB/s** |    **765.3 MiB/s** | **30.63 %** |
| 8 Bit 7LUT                       |  **42.98 %** |  **4,845.2  MiB/s** |      720.4 MiB/s   |   32.42 %   |
| 16 Bit Packed (Symbol)           |  **47.89 %** |  **9,417.5  MiB/s** |      886.7 MiB/s   |   34.25 %   |
| 16 Bit Packed (Byte)             |  **46.99 %** |  **8,322.4  MiB/s** |      913.4 MiB/s   |   33.96 %   |
| 32 Bit (Byte)                    |  **54.27 %** |   14,032.1  MiB/s   |  **1,327.9 MiB/s** |   37.28 %   |
| 48 Bit (Byte)                    |  **57.58 %** |   16,775.3  MiB/s   |  **1,557.6 MiB/s** |   38.86 %   |
| 48 Bit Packed (Byte)             |  **55.01 %** | **16,106.6  MiB/s** |  **1,385.7 MiB/s** |   37.48 %   |
| 64 Bit Packed (Symbol)           |  **58.87 %** | **17,252.6  MiB/s** |    1,353.7 MiB/s   |   39.42 %   |
| 64 Bit (Byte)                    |  **59.94 %** | **17,737.0  MiB/s** |  **1,606.6 MiB/s** |   40.05 %   |
| 64 Bit Packed (Byte)             |  **57.33 %** |   16,491.9  MiB/s   |  **1,421.2 MiB/s** |   38.65 %   |
| 64 Bit 3LUT (Byte)               |  **56.28 %** | **17,019.3  MiB/s** |      532.3 MiB/s   |   38.04 %   |
| 128 Bit Packed (Symbol)          |  **64.32 %** | **20,061.6  MiB/s** |    1,415.5 MiB/s   |   42.35 %   |
| 128 Bit Packed (Byte)            |  **62.94 %** | **19,334.9  MiB/s** |    1,461.5 MiB/s   |   41.61 %   |
| 8 Bit MMTF 128                   |    63.49 %   |    1,299.9  MiB/s   |    1,101.2 MiB/s   | **29.84 %** |
| Low Entropy                      |    64.19 %   |      913.9  MiB/s   |      381.1 MiB/s   |   34.06 %   |
| Low Entropy Short                |    66.17 %   |    1,403.6  MiB/s   |      394.1 MiB/s   |   36.11 %   |
| Multi MTF 128 Bit (Transform)    |   100.00 %   |    1,289.8  MiB/s   |    1,205.9 MiB/s   |   33.92 %   |
| Multi MTF 256 Bit (Transform)    |   100.00 %   |    1,966.1  MiB/s   |    2,012.2 MiB/s   |   35.65 %   |
| - | - | - | - | - |
| memcpy                           |   100.0 %    |    27,031.0 MiB/s   |   26,776.9 MiB/s   |   65.94 %   |
| trle    | 42.4 % |   697.6 MiB/s |  2,341.0 MiB/s | - |
| srle 0  | 46.9 % |   680.6 MiB/s |  6,341.4 MiB/s | - |
| srle 8  | 46.9 % |   858.6 MiB/s |  6,310.3 MiB/s | - |
| srle 16 | 54.4 % |   906.4 MiB/s |  8,818.9 MiB/s | - |
| srle 32 | 61.0 % | 1,766.0 MiB/s | 13,030.9 MiB/s | - |
| srle 64 | 66.8 % | 3,289.0 MiB/s | 18,660.4 MiB/s | - |
| mrle    | 64.1 % |   320.2 MiB/s |  1,131.5 MiB/s | - |

The 24 Bit and 48 Bit Variants allow for run length encoding of common data layouts that are usually not covered by RLE implementations:

#### [Pixel Art Bitmap Image](https://i.redd.it/tj5oyhhuehv11.png) (PNG converted to BMP, 123,710,454 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| 8 Bit                         |  99.99 %   |   19,424.5 MiB/s   |    7,124.0 MiB/s   |   76.10 %   |
| 8 Bit 7LUT Short              |  99.98 %   |   18,994.0 MiB/s   |    6,816.8 MiB/s   |   75.78 %   |
| 24 Bit (Symbol)               |   1.84 %   |   33,067.5 MiB/s   |    5,852.0 MiB/s   |    1.32 %   |
| 24 Bit Short (Symbol)         |   2.08 %   |   32,713.6 MiB/s   |    5,336.3 MiB/s   |    1.52 %   |
| 24 Bit Packed (Symbol)        |   2.00 %   |   32,955.6 MiB/s   |    5,487.5 MiB/s   |    1.39 %   |
| 24 Bit 1LUT Short (Symbol)    |   2.08 %   |   32,751.7 MiB/s   |    4,886.8 MiB/s   |    1.51 %   |
| 24 Bit 3LUT (Symbol)          | **1.31 %** | **33,073.5 MiB/s** |  **5,335.0 MiB/s** |    0.93 %   |
| 24 Bit 3LUT Short (Symbol)    |   1.53 %   |   32,890.5 MiB/s   |    5,194.7 MiB/s   |    1.04 %   |
| 24 Bit 7LUT (Symbol)          | **1.20 %** | **32,846.9 MiB/s** |  **4,670.1 MiB/s** |  **0.83 %** |
| 24 Bit 7LUT Short (Symbol)    |   1.41 %   |   31,750.2 MiB/s   |    5,222.2 MiB/s   |    0.92 %   |
| 24 Bit (Byte)                 | **2.13 %** | **33,113.4 MiB/s** |    6,052.7 MiB/s   |    1.44 %   |
| 24 Bit Short (Byte)           |   2.17 %   |   32,825.6 MiB/s   |    5,632.3 MiB/s   |    1.58 %   |
| 24 Bit Packed (Byte)          |   2.32 %   |   32,812.0 MiB/s   |    5,603.5 MiB/s   |    1.51 %   |
| 24 Bit 1LUT Short (Byte)      |   2.17 %   |   32,835.9 MiB/s   |    5,697.6 MiB/s   |    1.59 %   |
| 24 Bit 3LUT (Byte)            |   1.49 %   |   33,010.6 MiB/s   |    5,614.2 MiB/s   |    1.03 %   |
| 24 Bit 3LUT Short (Byte)      |   1.63 %   |   32,713.6 MiB/s   |    4,870.9 MiB/s   |    1.14 %   |
| 24 Bit 7LUT (Byte)            | **1.37 %** |   32,629.3 MiB/s   |  **5,493.1 MiB/s** |    0.98 %   |
| 24 Bit 7LUT Short (Byte)      |   1.52 %   |   31,915.9 MiB/s   |    5,489.0 MiB/s   |    0.97 %   |
| 48 Bit (Symbol)               |   2.78 %   |   32,870.0 MiB/s   |    9,145.2 MiB/s   |    2.12 %   |
| 48 Bit Short (Symbol)         | **2.79 %** |   32,833.3 MiB/s   |  **9,738.3 MiB/s** |    2.26 %   |
| 48 Bit Packed (Symbol)        | **2.87 %** |   32,913.6 MiB/s   | **10,349.4 MiB/s** |    2.17 %   |
| 48 Bit 1LUT Short (Symbol)    |   3.09 %   |   32,750.9 MiB/s   |    9,293.6 MiB/s   |    2.41 %   |
| 48 Bit 3LUT (Symbol)          | **1.71 %** |   32,843.5 MiB/s   |  **9,184.1 MiB/s** |    1.33 %   |
| 48 Bit 3LUT Short (Symbol)    |   1.99 %   |   32,717.0 MiB/s   |    9,188.6 MiB/s   |    1.47 %   |
| 48 Bit 7LUT (Symbol)          | **1.49 %** |   32,619.2 MiB/s   |  **8,992.3 MiB/s** |    1.15 %   |
| 48 Bit 7LUT Short (Symbol)    |   1.77 %   |   31,846.0 MiB/s   |    8,628.7 MiB/s   |    1.26 %   |
| 48 Bit (Byte)                 |   3.16 %   |   32,911.9 MiB/s   |    9,317.6 MiB/s   |    2.31 %   |
| 48 Bit Short (Byte)           |   3.20 %   |   32,689.1 MiB/s   |    9,709.3 MiB/s   |    2.48 %   |
| 48 Bit Packed (Byte)          |   3.35 %   |   32,548.7 MiB/s   |   10,199.2 MiB/s   |    2.40 %   |
| 48 Bit 1LUT Short (Byte)      |   3.20 %   |   32,603.2 MiB/s   |    9,691.9 MiB/s   |    2.48 %   |
| 48 Bit 3LUT (Byte)            | **1.98 %** |   32,861.4 MiB/s   |  **9,629.9 MiB/s** |    1.49 %   |
| 48 Bit 3LUT Short (Byte)      |   2.13 %   |   32,717.9 MiB/s   |    9,244.4 MiB/s   |    1.62 %   |
| 48 Bit 7LUT (Byte)            | **1.76 %** |   32,615.9 MiB/s   |  **9,347.5 MiB/s** |    1.34 %   |
| 48 Bit 7LUT Short (Byte)      |   1.90 %   |   31,847.6 MiB/s   |    9,069.5 MiB/s   |    1.35 %   |
| 8 Bit MMTF 128                |  17.70 %   |    5,287.0 MiB/s   |    2,978.1 MiB/s   |   12.31 %   |
| Multi MTF 128 Bit (Transform) | 100.00 %   |    3,474.5 MiB/s   |    3,121.4 MiB/s   |   19.94 %   |
| Multi MTF 256 Bit (Transform) | 100.00 %   |    5,030.6 MiB/s   |    4,952.9 MiB/s   |   27.52 %   |
| - | - | - | - | - |
| memcpy                        |   100.0 %  |   28,288.5 MiB/s   |   28,261.3 MiB/s   |   77.03 %   |
| trle    | 100.0 % |   433.10 MiB/s |  5,558.8 MiB/s | - |
| srle 0  | 100.0 % |   437.91 MiB/s | 19,632.5 MiB/s | - |
| srle 8  | 100.0 % |   478.14 MiB/s | 19,482.1 MiB/s | - |
| srle 16 | 100.0 % |   593.47 MiB/s | 20,315.8 MiB/s | - |
| srle 32 | 100.0 % | 1,249.68 MiB/s | 20,287.0 MiB/s | - |
| srle 64 | 100.0 % | 2,456.70 MiB/s | 19,643.0 MiB/s | - |
| mrle    | 100.0 % |   394.79 MiB/s |  2,595.1 MiB/s | - |

### Variants
#### 8, 16, 24, 32, 48, 64, 128 Bit (Byte Aligned + Symbol Aligned)
- Extremely Fast (especially decoding files).
- Variants for always aligning with the symbol width or allowing byte-wide repeats even for > 8 bit symbols.
- Decoder interprets blocks of data to boil down to a highly optimized `memcpy`, `memset`, `memcpy`, `memset` (with various different byte-lengths).
- 8 bit encoder highly optimized as well, optional variant single symbol encoding.

### 8, 16, 24, 32, 48, 64, 128 Bit Packed (Byte Aligned + Symbol Aligned)
- Similar to the base variant, but keeps around the previously used RLE symbol which is usually very beneficial to the compression ratio and tries to pack lengths a bit more optimistically.
- Also Extremely Fast (especially decoding files), whilst providing better compression ratio for many inputs
- Also has those variants for always aligning with the symbol width or allowing byte-wide repeats even for > 8 bit symbols.
- 8 bit encoder highly optimized as well, optional variant single symbol encoding (only has the optimistic packing, as it already knows what the next symbol is going to be anyways).

### 1 Sym LUT / 3 Sym LUT / 3 Sym LUT Short / 7 Sym LUT / 7 Sym LUT Short
- Similar to the base variant, but keeps around one / three / seven of the previously used RLE symbols, usually further improving compression ratios.
- Short Variant: Packs Range & Count Bits to fit the entire RLE command into just one byte for short ranges.

#### Low Entropy / Low Entropy Short
- Tries to keep symbol general symbol frequency to improve compression ratio of an entropy encoder that could go after the Run Length Encoding like ANS, Arithmetic Coding or Huffman.
- Parses the output for run-length-encodable symbols, which are specified in the header.
- Has a single-symbol variant, that only encodes the most run-length-encodable symbol (useful for some image codecs).
- Short Variant: Same as Low Entropy, but optimized for shorter strings of run-length-encodable symbols, usually faster, also has a single-symbol variant.

#### 8 Bit RLE + MMTF (Multi Move-To-Front Transformation)
- Runs a block-wide vectorized MTF transform on the input and depending on how many bits this needs to represent a given block (if it's not entirely representable by a variant of `memset`) uses only the required amount of bits to encode the block.
- Performs well on a wide variety of inputs, but usually doesn't produce the best compression ratios or (de-) compression speeds, as all blocks need to be decoded and cannot simply be `memcpy`d if they don't contain an encodable symbol.

#### RLE + Huffman-esque
- Uses a separate header, that contains a huffman-esque instructions to place recent high-prevalence symbols, copy or set a specific symbol to a block.
- Usually very high compression ratios, but comparably slow to en- & decode.

#### Raw MMTF (Multi Move-To-Front Transformation)
- Simply runs the block-wide vectorized MTF transform on the input to improve compressability for some scenarios.
- Doesn't compress itself.

### Setup
``` bash
git clone https://github.com/rainerzufalldererste/rle8.git
cd rle8
git submodule update --init --recursive
```
#### On Windows
```
create_project.bat
```
Choose your preferred compiler toolset
```
MSBuild /p:Configuration=Release /nologo /v:m
```
#### On Linux
```
premake/premake5 gmake2
config=release_x64 make
```

### How to use it?

```c
#include "rle.h"

uint8_t *pUncompressedData; // Some Data.
uint32_t fileSize; // Some Size.

// Get Compress Bounds.
const uint32_t compressedBufferSize = rle_compress_bounds(fileSize);
uint8_t *pCompressedData = (uint8_t *)malloc(compressedBufferSize);

// Compress.
const uint32_t compressedSize = rle8_multi_compress(pUncompressedData, fileSize, pCompressedData, compressedBufferSize);

// Allocate Output Buffer.
uint8_t *pDecompressedData = (uint8_t *)malloc(fileSize + rle_decompress_additional_size());

// Decompress.
const uint32_t decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, fileSize);

// Cleanup.
free(pCompressedData);
free(pDecompressedData);
```

### License
Two Clause BSD
