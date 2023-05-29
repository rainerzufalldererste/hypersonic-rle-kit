  <a href="https://github.com/rainerzufalldererste/hypersonic-rle-kit"><img src="https://raw.githubusercontent.com/rainerzufalldererste/hypersonic-rle-kit/master/docs/logo.png" alt="hypersonic rle kit logo" style="width: 533pt; max-width: 100%; margin: 50pt auto;"></a>
  <br>

### What is it?
- A selection of 100+ RLE and related codecs optimized for all kinds of different inputs and scenarios.
- Usually the fastest run length en/decoder by far. **Single Core Decompression Speeds > 34 GB/s and Compression Speeds > 28 GB/s have been observed with large files.** (small files can exceed 120 GB/s decode, 60 GB/s encode)
- Written in C.
- SIMD Variants for AVX-512F, AVX2, AVX, SSE4.1, SSSE3 and SSE2 variants are available for various decoders and encoders. Automatically picked at runtime based on the extensions available on the current platform.
- Variants include: Single RLE Symbol, Short Strings of RLE Symbols, Byte Alignmed, Symbol Aligned, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit, 128 Bit, Different probabilities of reoccuring symbols, ...
- `OpenCL` variant available for some of the decoders.

### Benchmark
**See [Full Benchmark with Graphs](https://rainerzufalldererste.github.io/hypersonic-rle-kit/), the tables below only contain a tiny selection of the _100+ codecs_.**
 
 - Single-Threaded
 - Running on an `AMD Ryzen 9 7950X`, `32 GB DDR5-6000 CL30` on Windows 11.
 - Compiled with `Visual Studio 2022`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) and [MRLE](https://encode.su/threads/2121-No-more-encoding-overhead-in-Run-Length-Encoding-Read-about-Mespotine-RLE-here-) (with AVX2 enabled; benchmarking results have been converted from MB/s to MiB/s)
 - Contained Codecs w/ Compression / Decompression Speed vs. Ratio Pareto + Notable Entropy Highlighted

#### [video-frame.raw](https://www.dropbox.com/s/yvsl1lg98c4maq1/video_frame.raw?dl=1) (heavily quantized video frame DCTs, 88,473,600 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| 8 Bit                         |  **19.34 %** |    3,319.6 MiB/s   | **25,830.4 MiB/s** |    9.50 %   |
| 8 Bit Packed                  |  **17.95 %** |    2,907.5 MiB/s   | **19,783.6 MiB/s** |    9.69 %   |
| 8 Bit 1LUT Short              |  **16.93 %** |  **2,365.1 MiB/s** | **11,069.1 MiB/s** |    9.87 %   |
| 8 Bit 3LUT                    |  **17.41 %** |    2,146.7 MiB/s   | **15,819.2 MiB/s** |    9.80 %   |
| 8 Bit 3LUT Short              |  **16.49 %** |  **2,085.6 MiB/s** |  **8,100.4 MiB/s** |    9.93 %   |
| 8 Bit 7LUT                    |  **17.46 %** |    2,094.6 MiB/s   | **16,100.9 MiB/s** |    9.94 %   |
| 8 Bit Single                  |  **18.31 %** |    1,654.7 MiB/s   | **23,750.2 MiB/s** |    9.71 %   |
| 8 Bit Single Short            |  **17.05 %** |    1,414.4 MiB/s   | **14,143.6 MiB/s** |    9.81 %   |
| 16 Bit 1LUT Short (Symbol)    |  **18.23 %** |  **3,783.5 MiB/s** |   13,705.5 MiB/s   |    9.72 %   |
| 16 Bit 1LUT Short (Byte)      |  **18.06 %** |  **3,745.3 MiB/s** |   12,951.5 MiB/s   |    9.90 %   |
| 16 Bit 3LUT (Byte)            |  **17.74 %** |  **3,708.2 MiB/s** |   15,577.4 MiB/s   |    9.82 %   |
| 16 Bit 7LUT (Byte)            |  **17.53 %** |  **3,526.7 MiB/s** |   13,249.8 MiB/s   |    9.96 %   |
| 24 Bit Packed (Byte)          |  **19.19 %** |  **6,890.9 MiB/s** |   21,560.0 MiB/s   |    9.57 %   |
| 24 Bit 3LUT (Byte)            |  **18.52 %** |  **5,367.1 MiB/s** |   18,636.5 MiB/s   |    9.49 %   |
| 24 Bit 7LUT (Byte)            |  **18.44 %** |  **5,196.5 MiB/s** |   17,679.4 MiB/s   |    9.59 %   |
| 32 Bit Packed (Byte)          |  **19.82 %** |  **8,946.4 MiB/s** |   23,569.8 MiB/s   |    9.46 %   |
| 32 Bit 3LUT (Byte)            |  **19.25 %** |  **7,149.3 MiB/s** |   23,580.3 MiB/s   |    9.34 %   |
| 32 Bit 7LUT (Byte)            |  **19.22 %** |  **7,066.1 MiB/s** |   23,033.8 MiB/s   |    9.45 %   |
| 48 Bit Packed (Byte)          |  **20.84 %** | **10,911.9 MiB/s** | **26,397.7 MiB/s** |    9.40 %   |
| 48 Bit 3LUT (Byte)            |  **20.41 %** |  **9,304.9 MiB/s** |   25,603.9 MiB/s   |    9.25 %   |
| 48 Bit 7LUT (Byte)            |  **20.42 %** |  **9,457.1 MiB/s** |   25,088.5 MiB/s   |    9.34 %   |
| 64 Bit (Byte)                 |  **23.66 %** | **13,514.1 MiB/s** |   25,773.6 MiB/s   |    9.63 %   |
| 64 Bit Packed (Byte)          |  **21.63 %** | **13,306.5 MiB/s** |   26,183.1 MiB/s   |    9.40 %   |
| 64 Bit 3LUT (Byte)            |  **21.30 %** | **11,442.9 MiB/s** | **27,864.0 MiB/s** |    9.25 %   |
| 64 Bit 7LUT (Byte)            |  **21.32 %** | **11,479.3 MiB/s** |   26,879.6 MiB/s   |    9.32 %   |
| 8 Bit RLE + Huffman-esque     |  **12.51 %** |    **332.0 MiB/s** |  **1,676.7 MiB/s** |    9.82 %   |
| Low Entropy                   |    19.93 %   |      800.2 MiB/s   |    2,690.6 MiB/s   |  **9.05 %** |
| Low Entropy Single            |    19.96 %   |      802.4 MiB/s   |    4,389.0 MiB/s   |  **9.10 %** |
| - | - | - | - | - | 
| memcpy                        |   100.00 %   |   28,667.8 MiB/s   |   28,590.1 MiB/s   |   14.03 %   |
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
| 8 Bit                         |  **23.02 %** |    2,443.8 MiB/s   | **21,198.0 MiB/s** |   12.08 %   |
| 8 Bit Short                   |  **21.12 %** |    1,896.9 MiB/s   |  **9,882.7 MiB/s** |   12.63 %   |
| 8 Bit Packed                  |  **20.10 %** |  **2,199.4 MiB/s** |   15,151.8 MiB/s   |   12.74 %   |
| 8 Bit 1LUT Short              |  **18.91 %** |  **1,767.7 MiB/s** |    7,905.5 MiB/s   |   12.46 %   |
| 8 Bit 3LUT                    |  **19.77 %** |  **1,841.7 MiB/s** |   14,211.6 MiB/s   |   12.31 %   |
| 8 Bit 7LUT                    |  **19.75 %** |  **1,768.7 MiB/s** | **14,629.5 MiB/s** |   12.50 %   |
| 8 Bit Single                  |  **20.59 %** |    1,325.0 MiB/s   | **18,469.7 MiB/s** |   12.06 %   |
| 8 Bit Single Short            |  **18.55 %** |  **1,157.3 MiB/s** |  **9,756.9 MiB/s** |   12.09 %   |
| 16 Bit Packed (Byte)          |  **20.65 %** |  **3,378.7 MiB/s** |   15,516.6 MiB/s   |   12.59 %   |
| 16 Bit 3LUT (Byte)            |  **20.20 %** |    2,641.6 MiB/s   | **15,500.6 MiB/s** |   12.30 %   |
| 16 Bit 7LUT (Byte)            |  **20.14 %** |  **2,708.0 MiB/s** |   13,971.5 MiB/s   |   12.35 %   |
| 24 Bit Packed (Byte)          |  **22.05 %** |  **4,233.2 MiB/s** |   17,670.5 MiB/s   |   12.34 %   |
| 24 Bit 3LUT (Byte)            |  **21.52 %** |  **3,448.9 MiB/s** |   16,464.5 MiB/s   |   11.96 %   |
| 32 Bit Packed (Byte)          |  **23.44 %** |  **5,240.1 MiB/s** |   17,864.4 MiB/s   |   12.15 %   |
| 32 Bit 3LUT (Byte)            |  **22.86 %** |    4,289.1 MiB/s   | **20,829.1 MiB/s** |   11.79 %   |
| 32 Bit 7LUT (Byte)            |  **22.89 %** |  **4,294.6 MiB/s** |   20,065.2 MiB/s   |   11.86 %   |
| 48 Bit Packed (Symbol)        |  **27.10 %** |    6,109.0 MiB/s   | **21,838.9 MiB/s** |   11.69 %   |
| 48 Bit Packed (Byte)          |  **25.87 %** |  **6,516.7 MiB/s** |   22,210.4 MiB/s   |   11.95 %   |
| 48 Bit 3LUT (Byte)            |  **25.41 %** |  **5,712.4 MiB/s** |   20,904.4 MiB/s   |   11.66 %   |
| 64 Bit (Byte)                 |  **31.66 %** |  **8,034.7 MiB/s** |   21,153.2 MiB/s   |   12.16 %   |
| 64 Bit Packed (Byte)          |  **27.29 %** |  **7,836.3 MiB/s** |   22,219.0 MiB/s   |   11.90 %   |
| 64 Bit 3LUT (Byte)            |  **26.90 %** |  **6,780.3 MiB/s** | **23,777.1 MiB/s** |   11.63 %   |
| 64 Bit 7LUT (Byte)            |  **27.05 %** |  **6,839.2 MiB/s** |   22,970.8 MiB/s   |   11.76 %   |
| 128 Bit (Byte)                |  **37.25 %** |    6,763.4 MiB/s   | **24,011.4 MiB/s** |   12.28 %   |
| 128 Bit Packed (Byte)         |  **31.21 %** |    6,582.9 MiB/s   | **23,994.6 MiB/s** |   11.98 %   |
| 8 Bit RLE + Huffman-esque     |  **16.76 %** |    **326.5 MiB/s** |  **1,456.9 MiB/s** |   12.36 %   |
| Low Entropy                   |    21.15 %   |      799.7 MiB/s   |    4,282.1 MiB/s   |   10.37 %   |
| Low Entropy Single            |    21.15 %   |      765.9 MiB/s   |    4,292.2 MiB/s   |   10.37 %   |
| - | - | - | - | - |
| memcpy                        |   100.00 %   |   26,689.1 MiB/s   |   27,304.6 MiB/s   |   16.76 %   |
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
| 8 Bit                         |  **48.80 %** |    1,225.6 MiB/s   | **16,097.6 MiB/s** |   34.13 %   |
| 8 Bit Short                   |  **44.42 %** |      899.3 MiB/s   |  **6,975.0 MiB/s** |   32.26 %   |
| 8 Bit Packed                  |  **44.86 %** |    1,038.7 MiB/s   |  **7,514.9 MiB/s** |   32.95 %   |
| 8 Bit 1LUT Short              |  **41.87 %** |    **828.2 MiB/s** |  **4,088.0 MiB/s** |   31.38 %   |
| 8 Bit 3LUT                    |  **43.29 %** |      775.1 MiB/s   |  **6,116.6 MiB/s** |   32.27 %   |
| 8 Bit 3LUT Short              |  **40.34 %** |    **765.3 MiB/s** |  **3,046.5 MiB/s** |   30.63 %   |
| 8 Bit 7LUT                    |  **42.98 %** |      720.4 MiB/s   |  **4,845.2 MiB/s** |   32.42 %   |
| 16 Bit Packed (Symbol)        |  **47.89 %** |    1,871.4 MiB/s   |  **9,417.5 MiB/s** |   34.25 %   |
| 16 Bit 1LUT Short (Symbol)    |  **46.43 %** |  **1,425.4 MiB/s** |    5,926.8 MiB/s   |   33.47 %   |
| 16 Bit Packed (Byte)          |  **46.99 %** |  **1,975.6 MiB/s** |  **8,322.4 MiB/s** |   33.96 %   |
| 16 Bit 3LUT (Byte)            |  **44.91 %** |  **1,332.9 MiB/s** |    5,786.9 MiB/s   |   33.36 %   |
| 16 Bit 7LUT (Byte)            |  **44.17 %** |  **1,269.3 MiB/s** |    4,792.9 MiB/s   |   33.00 %   |
| 24 Bit Packed (Byte)          |  **49.39 %** |  **2,846.0 MiB/s** |   10,096.3 MiB/s   |   35.00 %   |
| 24 Bit 3LUT (Byte)            |  **47.63 %** |  **2,154.2 MiB/s** |    8,316.6 MiB/s   |   34.23 %   |
| 24 Bit 7LUT (Byte)            |  **47.15 %** |  **2,021.5 MiB/s** |    7,081.2 MiB/s   |   33.86 %   |
| 32 Bit Packed (Byte)          |  **51.63 %** |  **3,991.3 MiB/s** |   12,316.8 MiB/s   |   35.94 %   |
| 32 Bit 3LUT (Byte)            |  **50.02 %** |  **3,049.7 MiB/s** |   11,014.7 MiB/s   |   35.15 %   |
| 32 Bit 7LUT (Byte)            |  **49.72 %** |  **2,929.3 MiB/s** |    9,734.1 MiB/s   |   34.97 %   |
| 48 Bit (Byte)                 |  **57.58 %** |    5,568.3 MiB/s   | **17,036.4 MiB/s** |   38.86 %   |
| 48 Bit Packed (Byte)          |  **55.01 %** |  **5,463.9 MiB/s** | **16,106.6 MiB/s** |   37.48 %   |
| 48 Bit 3LUT (Byte)            |  **53.66 %** |  **4,511.6 MiB/s** |   14,552.7 MiB/s   |   36.78 %   |
| 48 Bit 7LUT (Byte)            |  **53.48 %** |  **4,362.4 MiB/s** |   13,340.3 MiB/s   |   36.64 %   |
| 64 Bit Packed (Symbol)        |  **58.87 %** |    6,995.2 MiB/s   | **17,252.6 MiB/s** |   39.42 %   |
| 64 Bit (Byte)                 |  **59.94 %** |  **7,594.6 MiB/s** | **17,821.4 MiB/s** |   40.05 %   |
| 64 Bit Packed (Byte)          |  **57.33 %** |  **7,285.0 MiB/s** |   16,557.0 MiB/s   |   38.65 %   |
| 64 Bit 1LUT Short (Byte)      |  **57.24 %** |  **6,176.4 MiB/s** |   16,263.8 MiB/s   |   38.60 %   |
| 64 Bit 3LUT (Byte)            |  **56.28 %** |  **6,050.7 MiB/s** | **17,019.3 MiB/s** |   38.04 %   |
| 64 Bit 7LUT (Byte)            |  **56.10 %** |  **5,933.0 MiB/s** |   16,061.7 MiB/s   |   37.92 %   |
| 128 Bit Packed (Symbol)       |  **64.32 %** |    5,640.8 MiB/s   | **20,237.7 MiB/s** |   42.35 %   |
| 128 Bit Packed (Byte)         |  **62.94 %** |    5,650.8 MiB/s   | **20,158.4 MiB/s** |   41.61 %   |
| Low Entropy                   |    64.19 %   |      383.3 MiB/s   |      913.9 MiB/s   |   34.06 %   |
| Low Entropy Single            |    89.22 %   |      561.6 MiB/s   |    7,491.0 MiB/s   |   59.50 %   |
| Low Entropy Short             |    66.17 %   |      394.1 MiB/s   |    1,412.4 MiB/s   |   36.11 %   |
| Low Entropy Short Single      |    89.65 %   |      576.5 MiB/s   |    9,841.2 MiB/s   |   61.05 %   |
| Multi MTF 128 Bit (Transform) |   100.00 %   |    1,206.2 MiB/s   |    1,289.8 MiB/s   |   33.92 %   |
| Multi MTF 256 Bit (Transform) |   100.00 %   |    2,012.2 MiB/s   |    1,966.1 MiB/s   |   35.65 %   |
| Bit MMTF 8 Bit (Transform)    |   100.00 %   |    1,843.2 MiB/s   |    1,973.5 MiB/s   |   35.87 %   |
| Bit MMTF 16 Bit (Transform)   |   100.00 %   |    2,309.8 MiB/s   |    2,272.9 MiB/s   |   38.34 %   |
| - | - | - | - | - |
| memcpy                        |   100.00 %   |   26,973.3 MiB/s   |   27,041.2 MiB/s   |   65.94 %   |
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
| 8 Bit                          |  99.99 %   |    6,995.3 MiB/s   |   18,357.0 MiB/s   |   76.10 %   |
| 8 Bit 1LUT Short               |  99.98 %   |    6,974.3 MiB/s   |   18,023.2 MiB/s   |   75.63 %   |
| 8 Bit 3LUT                     |  99.99 %   |    6,936.1 MiB/s   |   18,595.7 MiB/s   |   75.94 %   |
| 8 Bit 7LUT Short               |  99.98 %   |    6,760.8 MiB/s   |   18,196.6 MiB/s   |   75.78 %   |
| 8 Bit Single                   | 100.00 %   |    3,512.8 MiB/s   |   18,470.1 MiB/s   |   77.03 %   |
| 16 Bit Short (Byte)            |  99.99 %   |   18,127.2 MiB/s   |   18,953.0 MiB/s   |   75.56 %   |
| 16 Bit 3LUT Short (Byte)       |  99.99 %   |   17,991.9 MiB/s   |   19,252.9 MiB/s   |   75.71 %   |
| 24 Bit (Symbol)                | **1.84 %** | **24,537.1 MiB/s** |   33,067.5 MiB/s   |    1.32 %   |
| 24 Bit Short (Symbol)          |   2.08 %   |   21,904.2 MiB/s   |   32,713.6 MiB/s   |    1.52 %   |
| 24 Bit Packed (Symbol)         |   2.00 %   |   23,590.4 MiB/s   |   32,955.6 MiB/s   |    1.39 %   |
| 24 Bit 1LUT Short (Symbol)     |   2.08 %   |   19,894.5 MiB/s   |   32,751.7 MiB/s   |    1.51 %   |
| 24 Bit 3LUT (Symbol)           | **1.31 %** | **21,232.5 MiB/s** | **33,073.5 MiB/s** |  **0.93 %** |
| 24 Bit 3LUT Short (Symbol)     |   1.53 %   |   19,147.7 MiB/s   |   32,890.5 MiB/s   |    1.04 %   |
| 24 Bit 7LUT (Symbol)           | **1.20 %** | **20,599.0 MiB/s** | **32,846.9 MiB/s** |  **0.83 %** |
| 24 Bit 7LUT Short (Symbol)     |   1.41 %   |   18,916.5 MiB/s   |   31,750.2 MiB/s   |    0.92 %   |
| 24 Bit (Byte)                  | **2.13 %** | **25,939.7 MiB/s** | **33,113.4 MiB/s** |  **1.44 %** |
| 24 Bit Short (Byte)            |   2.17 %   |   23,579.9 MiB/s   |   32,825.6 MiB/s   |    1.58 %   |
| 24 Bit Packed (Byte)           | **2.32 %** | **26,738.2 MiB/s** |   32,812.0 MiB/s   |    1.51 %   |
| 24 Bit 1LUT Short (Byte)       |   2.17 %   |   21,550.6 MiB/s   |   32,835.9 MiB/s   |    1.59 %   |
| 24 Bit 3LUT (Byte)             | **1.49 %** | **23,408.0 MiB/s** |   33,010.6 MiB/s   |    1.03 %   |
| 24 Bit 3LUT Short (Byte)       |   1.63 %   |   21,063.2 MiB/s   |   32,713.6 MiB/s   |    1.14 %   |
| 24 Bit 7LUT (Byte)             | **1.37 %** | **22,589.6 MiB/s** |   32,629.3 MiB/s   |    0.98 %   |
| 24 Bit 7LUT Short (Byte)       |   1.52 %   |   20,560.6 MiB/s   |   31,915.9 MiB/s   |    0.97 %   |
| 32 Bit Packed (Byte)           |  99.99 %   |   16,961.2 MiB/s   |   19,048.9 MiB/s   |   75.71 %   |
| 32 Bit 7LUT Short (Byte)       |  99.99 %   |   16,887.8 MiB/s   |   19,240.9 MiB/s   |   75.79 %   |
| 48 Bit (Symbol)                |   2.78 %   |   23,645.9 MiB/s   |   32,870.0 MiB/s   |    2.12 %   |
| 48 Bit Short (Symbol)          |   2.79 %   |   21,386.8 MiB/s   |   32,833.3 MiB/s   |    2.26 %   |
| 48 Bit Packed (Symbol)         |   2.87 %   |   22,575.5 MiB/s   |   32,913.6 MiB/s   |    2.17 %   |
| 48 Bit 1LUT Short (Symbol)     |   3.09 %   |   19,362.4 MiB/s   |   32,750.9 MiB/s   |    2.41 %   |
| 48 Bit 3LUT (Symbol)           |   1.71 %   |   20,826.5 MiB/s   |   32,843.5 MiB/s   |    1.33 %   |
| 48 Bit 3LUT Short (Symbol)     |   1.99 %   |   18,834.0 MiB/s   |   32,717.0 MiB/s   |    1.47 %   |
| 48 Bit 7LUT (Symbol)           |   1.49 %   |   20,185.7 MiB/s   |   32,619.2 MiB/s   |    1.15 %   |
| 48 Bit 7LUT Short (Symbol)     |   1.77 %   |   18,184.3 MiB/s   |   31,846.0 MiB/s   |    1.26 %   |
| 48 Bit (Byte)                  |   3.16 %   |   25,473.5 MiB/s   |   32,911.9 MiB/s   |    2.31 %   |
| 48 Bit Short (Byte)            |   3.20 %   |   23,439.2 MiB/s   |   32,689.1 MiB/s   |    2.48 %   |
| 48 Bit Packed (Byte)           |   3.35 %   |   25,371.9 MiB/s   |   32,548.7 MiB/s   |    2.40 %   |
| 48 Bit 1LUT Short (Byte)       |   3.20 %   |   20,935.0 MiB/s   |   32,603.2 MiB/s   |    2.48 %   |
| 48 Bit 3LUT (Byte)             |   1.98 %   |   22,790.5 MiB/s   |   32,861.4 MiB/s   |    1.49 %   |
| 48 Bit 3LUT Short (Byte)       |   2.13 %   |   20,273.7 MiB/s   |   32,717.9 MiB/s   |    1.62 %   |
| 48 Bit 7LUT (Byte)             |   1.76 %   |   21,776.5 MiB/s   |   32,615.9 MiB/s   |    1.34 %   |
| 48 Bit 7LUT Short (Byte)       |   1.90 %   |   20,145.3 MiB/s   |   31,847.6 MiB/s   |    1.35 %   |
| 64 Bit (Symbol)                |  99.99 %   |   14,820.8 MiB/s   |   19,040.6 MiB/s   |   75.87 %   |
| 64 Bit 7LUT (Byte)             |  99.99 %   |   14,692.8 MiB/s   |   19,217.2 MiB/s   |   75.78 %   |
| 8 Bit MMTF 128                 |  17.70 %   |    5,287.0 MiB/s   |    2,978.1 MiB/s   |   12.31 %   |
| Multi MTF 128 Bit (Transform)  | 100.00 %   |    3,474.5 MiB/s   |    3,482.1 MiB/s   |   19.94 %   |
| Multi MTF 256 Bit (Transform)  | 100.00 %   |    5,030.6 MiB/s   |    5,028.6 MiB/s   |   27.52 %   |
| Bit MMTF 8 Bit (Transform)     | 100.00 %   |    1,861.6 MiB/s   |    1,984.2 MiB/s   |   73.85 %   |
| Bit MMTF 16 Bit (Transform)    | 100.00 %   |    2,340.1 MiB/s   |    2,293.9 MiB/s   |   72.48 %   |
| - | - | - | - | - |
| memcpy                         | 100.00 %   |   28,288.5 MiB/s   |   28,261.3 MiB/s   |   77.03 %   |
| trle    | 100.0 % |   433.10 MiB/s |  5,558.8 MiB/s | - |
| srle 0  | 100.0 % |   437.91 MiB/s | 19,632.5 MiB/s | - |
| srle 8  | 100.0 % |   478.14 MiB/s | 19,482.1 MiB/s | - |
| srle 16 | 100.0 % |   593.47 MiB/s | 20,315.8 MiB/s | - |
| srle 32 | 100.0 % | 1,249.68 MiB/s | 20,287.0 MiB/s | - |
| srle 64 | 100.0 % | 2,456.70 MiB/s | 19,643.0 MiB/s | - |
| mrle    | 100.0 % |   394.79 MiB/s |  2,595.1 MiB/s | - |

### Variants
#### 8, 16, 24, 32, 48, 64, 128 Bit (Byte Aligned + Symbol Aligned)
- Extremely Fast.
- Variants for always aligning with the symbol width or allowing byte-wide repeats even for > 8 bit symbols.
- Decoder interprets blocks of data to boil down to a highly optimized `memcpy`, `memset`, `memcpy`, `memset` (with various different byte-lengths).
- Encoder searches for repeats and their respective lengths using movemask instructions.
- Optional variant for 8 bit single symbol encoding.

#### 8, 16, 24, 32, 48, 64, 128 Bit Packed (Byte Aligned + Symbol Aligned)
- Similar to the base variant, but keeps around the previously used RLE symbol which is usually very beneficial to the compression ratio and tries to pack lengths a bit more optimistically.
- Also Extremely Fast, whilst providing better compression ratio for many inputs
- Also has those variants for always aligning with the symbol width or allowing byte-wide repeats even for > 8 bit symbols.
- Optional variant for 8 bit single symbol encoding (only has the optimistic packing, as it already knows what the next symbol is going to be anyways).

#### 1 Sym LUT / 3 Sym LUT / 3 Sym LUT Short / 7 Sym LUT / 7 Sym LUT Short
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
git clone https://github.com/rainerzufalldererste/hypersonic-rle-kit.git
cd hypersonic-rle-kit
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
