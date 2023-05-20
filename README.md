  <a href="https://github.com/rainerzufalldererste/rle8"><img src="https://raw.githubusercontent.com/rainerzufalldererste/rle8/master/.github.io/logo.png" alt="hypersonic rle kit logo" style="width: 533pt; max-width: 100%"></a>
  <br>

### What is it?
- A selection of 100+ RLE and related codecs optimized for all kinds of different inputs and scenarios.
- Possibly the fastest run length en/decoder (obviously dependent on the dataset). **Single Core Decompression Speeds > 33 GiB/s have been observed.**
- Written in C.
- SIMD Variants for AVX-512F, AVX2, AVX, SSE4.1, SSSE3, SSE2 and SSE variants are available for various decoders and encoders. Automatically picked at runtime based on the extensions available on the current platform.
- Variants include: Single RLE Symbol, Short Strings of RLE Symbols, Byte Alignmed, Symbol Aligned, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit, 128 Bit, Different probabilities of reoccuring symbols, ...
- `OpenCL` variant available for some of the decoders.

### Benchmark
 - Single-Threaded
 - Running on an `AMD Ryzen 9 7950X`, `32 GB DDR5-6000 CL30` on Windows 11.
 - Compiled with `Visual Studio 2022`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) and [MRLE](https://encode.su/threads/2121-No-more-encoding-overhead-in-Run-Length-Encoding-Read-about-Mespotine-RLE-here-) (with AVX2 enabled; benchmarking results have been converted from MB/s to MiB/s)
 - Contained Codecs w/ Compression / Decompression Speed vs. Ratio Pareto + Notable Entropy Highlighted

#### [video-frame.raw](https://www.dropbox.com/s/yvsl1lg98c4maq1/video_frame.raw?dl=1) (heavily quantized video frame DCTs, 88,473,600 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
|   8 Bit                            |  **19.34 %** | **25830.4 MiB/s** | **3319.6 MiB/s** |    9.50 %   |
|   8 Bit Short                      |    18.23 %   |   14888.3 MiB/s   |   2567.2 MiB/s   |    9.75 %   |
|   8 Bit Packed                     |  **17.95 %** | **19783.6 MiB/s** | **2907.5 MiB/s** |    9.69 %   |
|   8 Bit 1LUT Short                 |  **16.93 %** | **11045.0 MiB/s** | **2365.1 MiB/s** |    9.87 %   |
|   8 Bit 3LUT                       |    17.41 %   |   15819.2 MiB/s   |   2122.9 MiB/s   |    9.80 %   |
|   8 Bit 3LUT Short                 |  **16.49 %** |  **8100.4 MiB/s** | **2085.6 MiB/s** |    9.93 %   |
|   8 Bit 7LUT                       |  **17.46 %** | **16100.9 MiB/s** |   2094.6 MiB/s   |    9.94 %   |
|   8 Bit 7LUT Short                 |    16.91 %   |    6417.5 MiB/s   |   1889.1 MiB/s   |   10.43 %   |
|   8 Bit Single                     |  **18.31 %** | **23750.2 MiB/s** |   1654.7 MiB/s   |    9.71 %   |
|   8 Bit Single Short               |  **17.05 %** | **14143.6 MiB/s** |   1393.2 MiB/s   |    9.81 %   |
|   8 Bit Single Packed              |    17.96 %   |   17669.0 MiB/s   |   1479.9 MiB/s   |   10.33 %   |
|   16 Bit (Symbol)                  |    20.32 %   |   22374.1 MiB/s   |   2146.1 MiB/s   |    9.56 %   |
|   16 Bit Short (Symbol)            |    19.64 %   |   16970.4 MiB/s   |   1327.0 MiB/s   |    9.74 %   |
|   16 Bit Packed (Symbol)           |    18.55 %   |   20009.2 MiB/s   |   1844.0 MiB/s   |    9.48 %   |
|   16 Bit 1LUT Short (Symbol)       |    18.23 %   |   13705.5 MiB/s   |   1212.3 MiB/s   |    9.72 %   |
|   16 Bit 3LUT (Symbol)             |  **18.16 %** | **15025.4 MiB/s** |    941.9 MiB/s   |    9.82 %   |
|   16 Bit 3LUT Short (Symbol)       |    17.98 %   |   11762.9 MiB/s   |    909.1 MiB/s   |    9.87 %   |
|   16 Bit 7LUT (Symbol)             |    17.98 %   |   14159.3 MiB/s   |    762.8 MiB/s   |    9.97 %   |
|   16 Bit 7LUT Short (Symbol)       |    18.05 %   |    9822.9 MiB/s   |    730.9 MiB/s   |   10.15 %   |
|   16 Bit (Byte)                    |    20.10 %   |   22416.9 MiB/s   |   2036.8 MiB/s   |    9.61 %   |
|   16 Bit Short (Byte)              |    19.40 %   |   15811.8 MiB/s   |   1352.7 MiB/s   |    9.84 %   |
|   16 Bit Packed (Byte)             |    18.47 %   |   19366.7 MiB/s   |   1915.9 MiB/s   |    9.63 %   |
|   16 Bit 1LUT Short (Byte)         |    18.06 %   |   12951.5 MiB/s   |   1263.7 MiB/s   |    9.90 %   |
|   16 Bit 3LUT (Byte)               |    17.74 %   |   15577.4 MiB/s   |    984.7 MiB/s   |    9.82 %   |
|   16 Bit 3LUT Short (Byte)         |    17.80 %   |   11857.4 MiB/s   |    914.3 MiB/s   |   10.03 %   |
|   16 Bit 7LUT (Byte)               |    17.53 %   |   13249.8 MiB/s   |    787.2 MiB/s   |    9.96 %   |
|   16 Bit 7LUT Short (Byte)         |    18.07 %   |   10771.9 MiB/s   |    739.1 MiB/s   |   10.37 %   |
|   24 Bit (Symbol)                  |    21.50 %   |   25460.9 MiB/s   |   2401.9 MiB/s   |    9.45 %   |
|   24 Bit Short (Symbol)            |    21.01 %   |   20852.9 MiB/s   |   1311.1 MiB/s   |    9.56 %   |
|   24 Bit Packed (Symbol)           |    19.49 %   |   20685.2 MiB/s   |   2182.2 MiB/s   |    9.45 %   |
|   24 Bit 1LUT Short (Symbol)       |    19.34 %   |   17325.8 MiB/s   |   1214.6 MiB/s   |    9.50 %   |
|   24 Bit 3LUT (Symbol)             |    19.06 %   |   18637.8 MiB/s   |   1024.9 MiB/s   |    9.49 %   |
|   24 Bit 3LUT Short (Symbol)       |    19.19 %   |   16574.0 MiB/s   |   1015.1 MiB/s   |    9.62 %   |
|   24 Bit 7LUT (Symbol)             |    18.97 %   |   17841.3 MiB/s   |    804.3 MiB/s   |    9.67 %   |
|   24 Bit 7LUT Short (Symbol)       |    19.32 %   |   15730.7 MiB/s   |    831.3 MiB/s   |    9.85 %   | 
|   24 Bit (Byte)                    |    20.80 %   |   24017.9 MiB/s   |   2471.3 MiB/s   |    9.66 %   |
|   24 Bit Short (Byte)              |    20.48 %   |   18481.8 MiB/s   |   1379.1 MiB/s   |    9.82 %   |
|   24 Bit Packed (Byte)             |    19.19 %   |   21006.0 MiB/s   |   2347.6 MiB/s   |    9.57 %   |
|   24 Bit 1LUT Short (Byte)         |    19.05 %   |   17919.7 MiB/s   |   1392.3 MiB/s   |    9.74 %   |
|   24 Bit 3LUT (Byte)               |    18.52 %   |   18636.5 MiB/s   |   1171.2 MiB/s   |    9.49 %   |
|   24 Bit 3LUT Short (Byte)         |    18.85 %   |   16768.7 MiB/s   |    973.4 MiB/s   |    9.81 %   |
|   24 Bit 7LUT (Byte)               |    18.44 %   |   17679.4 MiB/s   |    911.7 MiB/s   |    9.59 %   |
|   24 Bit 7LUT Short (Byte)         |    19.00 %   |   17476.9 MiB/s   |    853.6 MiB/s   |    9.98 %   |
|   32 Bit (Symbol)                  |    22.01 %   |   22918.0 MiB/s   |   2859.5 MiB/s   |    9.67 %   |
|   32 Bit Short (Symbol)            |    21.83 %   |   20274.7 MiB/s   |   1431.8 MiB/s   |    9.70 %   |
|   32 Bit Packed (Symbol)           |    20.21 %   |   22783.7 MiB/s   |   2490.0 MiB/s   |    9.32 %   |
|   32 Bit 1LUT Short (Symbol)       |    20.11 %   |   19574.3 MiB/s   |   1352.8 MiB/s   |    9.50 %   |
|   32 Bit 3LUT (Symbol)             |    19.87 %   |   23101.2 MiB/s   |   1176.8 MiB/s   |    9.34 %   |
|   32 Bit 3LUT Short (Symbol)       |    20.01 %   |   19362.3 MiB/s   |   1075.7 MiB/s   |    9.58 %   |
|   32 Bit 7LUT (Symbol)             |    19.84 %   |   22139.9 MiB/s   |    943.9 MiB/s   |    9.41 %   |
|   32 Bit 7LUT Short (Symbol)       |    20.13 %   |   19252.3 MiB/s   |    874.6 MiB/s   |    9.70 %   |
|   32 Bit (Byte)                    |    21.39 %   |   23873.2 MiB/s   |   2802.9 MiB/s   |    9.70 %   |
|   32 Bit Short (Byte)              |    21.31 %   |   19997.9 MiB/s   |   1566.7 MiB/s   |    9.84 %   |
|   32 Bit Packed (Byte)             |    19.82 %   |   23569.8 MiB/s   |   2564.4 MiB/s   |    9.46 %   |
|   32 Bit 1LUT Short (Byte)         |    19.80 %   |   21489.2 MiB/s   |   1456.7 MiB/s   |    9.70 %   |
|   32 Bit 3LUT (Byte)               |    19.25 %   |   23580.3 MiB/s   |   1289.7 MiB/s   |    9.34 %   |
|   32 Bit 3LUT Short (Byte)         |    19.59 %   |   21495.7 MiB/s   |   1150.8 MiB/s   |    9.73 %   |
|   32 Bit 7LUT (Byte)               |    19.22 %   |   23033.8 MiB/s   |   1005.3 MiB/s   |    9.45 %   |
|   32 Bit 7LUT Short (Byte)         |    19.59 %   |   20569.2 MiB/s   |    915.0 MiB/s   |    9.76 %   |
|   48 Bit (Symbol)                  |    23.41 %   |   25692.0 MiB/s   |   3003.5 MiB/s   |    9.60 %   |
|   48 Bit Short (Symbol)            |    23.33 %   |   23543.4 MiB/s   |   1358.4 MiB/s   |    9.64 %   |
|   48 Bit Packed (Symbol)           |    21.39 %   |   25859.7 MiB/s   |   2867.0 MiB/s   |    9.24 %   |
|   48 Bit 1LUT Short (Symbol)       |    21.35 %   |   22728.5 MiB/s   |   1342.0 MiB/s   |    9.38 %   |
|   48 Bit 3LUT (Symbol)             |    21.15 %   |   25030.4 MiB/s   |   1339.2 MiB/s   |    9.25 %   |
|   48 Bit 3LUT Short (Symbol)       |    21.29 %   |   22619.4 MiB/s   |   1093.3 MiB/s   |    9.44 %   |
|   48 Bit 7LUT (Symbol)             |    21.16 %   |   25167.8 MiB/s   |    954.3 MiB/s   |    9.30 %   |
|   48 Bit 7LUT Short (Symbol)       |    21.37 %   |   23043.2 MiB/s   |    908.8 MiB/s   |    9.53 %   |
|   48 Bit (Byte)                    |  **22.66 %** |   26481.4 MiB/s   | **3329.7 MiB/s** |    9.63 %   |
|   48 Bit Short (Byte)              |    22.67 %   |   23735.5 MiB/s   |   1439.1 MiB/s   |    9.75 %   |
|   48 Bit Packed (Byte)             |  **20.84 %** | **25920.9 MiB/s** |   3009.6 MiB/s   |    9.40 %   |
|   48 Bit 1LUT Short (Byte)         |    20.78 %   |   25306.7 MiB/s   |   1479.6 MiB/s   |    9.48 %   |
|   48 Bit 3LUT (Byte)               |    20.41 %   |   25603.9 MiB/s   |   1223.9 MiB/s   |    9.25 %   |
|   48 Bit 3LUT Short (Byte)         |    20.64 %   |   24528.3 MiB/s   |   1181.4 MiB/s   |    9.51 %   |
|   48 Bit 7LUT (Byte)               |    20.42 %   |   25080.3 MiB/s   |   1067.5 MiB/s   |    9.34 %   |
|   48 Bit 7LUT Short (Byte)         |    20.61 %   |   24312.1 MiB/s   |    825.2 MiB/s   |    9.51 %   |
|   64 Bit (Symbol)                  |    24.53 %   |   25516.4 MiB/s   |   3397.1 MiB/s   |    9.61 %   |
|   64 Bit Short (Symbol)            |    24.47 %   |   24813.3 MiB/s   |   1482.1 MiB/s   |    9.64 %   |
|   64 Bit Packed (Symbol)           |    22.34 %   |   26375.4 MiB/s   |   2965.1 MiB/s   |    9.23 %   |
|   64 Bit 1LUT Short (Symbol)       |    22.30 %   |   25039.3 MiB/s   |   1355.2 MiB/s   |    9.36 %   |
|   64 Bit 3LUT (Symbol)             |  **22.13 %** | **26928.5 MiB/s** |   1223.4 MiB/s   |    9.24 %   |
|   64 Bit 3LUT Short (Symbol)       |    22.26 %   |   25477.9 MiB/s   |   1087.7 MiB/s   |    9.42 %   |
|   64 Bit 7LUT (Symbol)             |    22.16 %   |   27262.6 MiB/s   |   1006.5 MiB/s   |    9.29 %   |
|   64 Bit 7LUT Short (Symbol)       |    22.32 %   |   25897.8 MiB/s   |    906.0 MiB/s   |    9.48 %   |
|   64 Bit (Byte)                    |  **23.66 %** |   25360.7 MiB/s   | **3497.4 MiB/s** |    9.63 %   |
|   64 Bit Short (Byte)              |    23.70 %   |   25870.8 MiB/s   |   1592.0 MiB/s   |    9.74 %   |
|   64 Bit Packed (Byte)             |    21.63 %   |   26063.4 MiB/s   |   2822.7 MiB/s   |    9.40 %   |
|   64 Bit 1LUT Short (Byte)         |    21.54 %   |   26474.7 MiB/s   |   1505.6 MiB/s   |    9.39 %   |
|   64 Bit 3LUT (Byte)               |    21.30 %   |   27864.0 MiB/s   |   1341.1 MiB/s   |    9.25 %   |
|   64 Bit 3LUT Short (Byte)         |    21.44 %   |   26319.5 MiB/s   |   1131.6 MiB/s   |    9.42 %   |
|   64 Bit 7LUT (Byte)               |    21.32 %   |   26879.6 MiB/s   |   1006.1 MiB/s   |    9.32 %   |
|   64 Bit 7LUT Short (Byte)         |    21.43 %   |   26478.9 MiB/s   |    973.0 MiB/s   |    9.44 %   |
|   128 Bit (Symbol)                 |    27.60 %   |   27173.0 MiB/s   |   2689.9 MiB/s   |    9.76 %   |
|   128 Bit Packed (Symbol)          |    24.82 %   |   27761.3 MiB/s   |   3159.8 MiB/s   |    9.36 %   |
|   128 Bit (Byte)                   |    26.46 %   |   27446.2 MiB/s   |   3424.4 MiB/s   |    9.74 %   |
|   128 Bit Packed (Byte)            |    23.83 %   |   27795.2 MiB/s   |   3284.2 MiB/s   |    9.50 %   |
|   8 Bit RLE + Huffman-esque        |  **12.51 %** |  **1676.7 MiB/s** |  **332.0 MiB/s** |    9.82 %   |
|   8 Bit MMTF 128                   |    21.05 %   |    3786.4 MiB/s   |   2921.2 MiB/s   |   10.72 %   |
|   Low Entropy                      |    19.93 %   |    2690.6 MiB/s   |    800.2 MiB/s   |  **9.05 %** |
|   Low Entropy Single               |    19.96 %   |    4389.0 MiB/s   |    775.6 MiB/s   |  **9.10 %** |
|   Low Entropy Short                |    24.08 %   |    2805.3 MiB/s   |    809.5 MiB/s   |   11.61 %   |
|   Low Entropy Short Single         |    24.11 %   |    5302.1 MiB/s   |    714.0 MiB/s   |   11.69 %   |
|   Multi MTF 128 Bit (Transform)    |   100.00 %   |    3642.0 MiB/s   |   3352.0 MiB/s   |   15.63 %   |
|   Multi MTF 256 Bit (Transform)    |   100.00 %   |    5630.4 MiB/s   |   5472.6 MiB/s   |   15.88 %   |
| - | - | - | - | - | 
| memcpy                             |    100.0 %   |  28,510.8 MiB/s   | 28,689.2 MiB/s   |   14.03 %   |
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
| 8 Bit                         |  **23.02 %** | **21583.8 MiB/s** |  **2443.8 MiB/s** |   12.08 %   |
| 8 Bit Short                   |    21.12 %   |    9882.7 MiB/s   |    1896.9 MiB/s   |   12.63 %   |
| 8 Bit Packed                  |  **20.10 %** | **15151.8 MiB/s** |  **2199.4 MiB/s** |   12.74 %   |
| 8 Bit 1LUT Short              |  **18.91 %** |    7855.4 MiB/s   |  **1767.7 MiB/s** |   12.46 %   |
| 8 Bit 3LUT                    |  **19.77 %** |   14211.6 MiB/s   |  **1841.7 MiB/s** |   12.31 %   |
| 8 Bit 3LUT Short              |    19.11 %   |    7026.0 MiB/s   |    1708.3 MiB/s   |   12.64 %   |
| 8 Bit 7LUT                    |  **19.75 %** | **14629.5 MiB/s** |  **1768.7 MiB/s** |   12.50 %   |
| 8 Bit 7LUT Short              |    19.81 %   |    5285.0 MiB/s   |    1503.9 MiB/s   |   13.07 %   |
| 8 Bit Single                  |  **20.59 %** | **18469.7 MiB/s** |    1325.0 MiB/s   |   12.06 %   |
| 8 Bit Single Short            |  **18.55 %** |  **9756.7 MiB/s** |    1117.8 MiB/s   |   12.09 %   |
| 8 Bit Single Packed           |    20.16 %   |   14221.0 MiB/s   |    1270.5 MiB/s   |   12.32 %   |
| 16 Bit (Symbol)               |    24.89 %   |   16863.5 MiB/s   |    1722.0 MiB/s   |   12.27 %   |
| 16 Bit (Byte)                 |    24.39 %   |   17468.6 MiB/s   |    1670.5 MiB/s   |   12.36 %   |
| 16 Bit 3LUT (Byte)            |  **20.20 %** | **15500.6 MiB/s** |     893.8 MiB/s   |   12.30 %   |
| 24 Bit (Symbol)               |    27.27 %   |   20043.9 MiB/s   |    1906.0 MiB/s   |   11.95 %   |
| 24 Bit (Byte)                 |    25.90 %   |   19801.1 MiB/s   |    1884.7 MiB/s   |   12.41 %   |
| 32 Bit (Symbol)               |    28.39 %   |   17347.4 MiB/s   |    2162.4 MiB/s   |   12.27 %   |
| 32 Bit (Byte)                 |  **27.15 %** |   17496.2 MiB/s   |  **2057.2 MiB/s** |   12.40 %   |
| 32 Bit 3LUT (Byte)            |  **22.86 %** | **20829.1 MiB/s** |    1053.9 MiB/s   |   11.79 %   |
| 48 Bit (Symbol)               |    31.22 %   |   21104.2 MiB/s   |    2221.4 MiB/s   |   12.02 %   |
| 48 Bit (Byte)                 |  **29.76 %** |   22123.2 MiB/s   |  **2612.1 MiB/s** |   12.19 %   |
| 48 Bit Short (Byte)           |    30.02 %   |   18616.8 MiB/s   |    1129.4 MiB/s   |   12.53 %   |
| 48 Bit Packed (Byte)          |  **25.87 %** | **22210.4 MiB/s** |  **2200.6 MiB/s** |   11.95 %   |
| 48 Bit 7LUT Short (Byte)      |    25.91 %   |   19441.2 MiB/s   |     780.0 MiB/s   |   12.04 %   |
| 64 Bit (Symbol)               |    33.36 %   |   20578.4 MiB/s   |    2592.9 MiB/s   |   11.98 %   |
| 64 Bit (Byte)                 |  **31.66 %** |   21036.6 MiB/s   |  **2742.4 MiB/s** |   12.16 %   |
| 64 Bit Packed (Byte)          |  **27.29 %** |   22019.2 MiB/s   |  **2246.0 MiB/s** |   11.90 %   |
| 64 Bit 3LUT (Byte)            |  **26.90 %** | **23777.1 MiB/s** |    1039.7 MiB/s   |   11.63 %   |
| 128 Bit (Symbol)              |    39.84 %   |   23432.1 MiB/s   |    2387.9 MiB/s   |   12.16 %   |
| 128 Bit Packed (Symbol)       |    33.52 %   |   23879.3 MiB/s   |    2172.0 MiB/s   |   11.65 %   |
| 128 Bit (Byte)                |  **37.25 %** | **24011.4 MiB/s** |    2489.4 MiB/s   |   12.28 %   |
| 128 Bit Packed (Byte)         |  **31.21 %** | **23977.4 MiB/s** |    2293.4 MiB/s   |   11.98 %   |
| 8 Bit RLE + Huffman-esque     |  **16.76 %** |  **1456.9 MiB/s** |   **326.5 MiB/s** |   12.36 %   |
| 8 Bit MMTF 128                |    26.61 %   |    2819.2 MiB/s   |    2118.5 MiB/s   |   14.85 %   |
| Low Entropy                   |    21.15 %   |    4282.1 MiB/s   |     799.7 MiB/s   | **10.37 %** |
| Low Entropy Single            |    21.15 %   |    4292.2 MiB/s   |     718.2 MiB/s   | **10.37 %** |
| Multi MTF 128 Bit (Transform) |   100.00 %   |    2666.7 MiB/s   |    2539.3 MiB/s   |   17.90 %   |
| Multi MTF 256 Bit (Transform) |   100.00 %   |    4094.2 MiB/s   |    4064.7 MiB/s   |   18.35 %   |
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
| 8 Bit                            |  **48.80 %** | **16097.6  MiB/s** |  **1219.7 MiB/s** |   34.13 %   |
| 8 Bit Short                      |  **44.42 %** |  **6975.0  MiB/s** |   **899.3 MiB/s** |   32.26 %   |
| 8 Bit Packed                     |  **44.86 %** |  **7514.9  MiB/s** |  **1038.7 MiB/s** |   32.95 %   |
| 8 Bit 1LUT Short                 |  **41.87 %** |  **4088.0  MiB/s** |   **815.0 MiB/s** |   31.38 %   |
| 8 Bit 3LUT                       |  **43.29 %** |  **6116.6  MiB/s** |     775.1 MiB/s   |   32.27 %   |
| 8 Bit 3LUT Short                 |  **40.34 %** |  **3043.5  MiB/s** |   **765.3 MiB/s** | **30.63 %** |
| 8 Bit 7LUT                       |  **42.98 %** |  **4845.2  MiB/s** |     720.4 MiB/s   |   32.42 %   |
| 8 Bit 7LUT Short                 |    41.27 %   |    2038.7  MiB/s   |     655.3 MiB/s   |   31.31 %   |
| 16 Bit (Symbol)                  |    51.49 %   |   12708.1  MiB/s   |    1098.3 MiB/s   |   35.70 %   |
| 16 Bit Packed (Symbol)           |  **47.89 %** |  **9417.5  MiB/s** |     886.7 MiB/s   |   34.25 %   |
| 16 Bit (Byte)                    |    50.74 %   |   12981.9  MiB/s   |    1024.0 MiB/s   |   35.33 %   |
| 16 Bit Packed (Byte)             |  **46.99 %** |  **8322.4  MiB/s** |     913.4 MiB/s   |   33.96 %   |
| 24 Bit (Symbol)                  |    54.90 %   |   15413.8  MiB/s   |    1244.0 MiB/s   |   37.33 %   |
| 24 Bit (Byte)                    |    52.73 %   |   14838.0  MiB/s   |    1196.6 MiB/s   |   36.44 %   |
| 32 Bit (Symbol)                  |    55.92 %   |   13837.0  MiB/s   |    1380.0 MiB/s   |   38.17 %   |
| 32 Bit (Byte)                    |  **54.27 %** |   14032.1  MiB/s   |  **1327.9 MiB/s** |   37.28 %   |
| 48 Bit (Symbol)                  |    59.24 %   |   16115.9  MiB/s   |    1398.1 MiB/s   |   39.77 %   |
| 48 Bit (Byte)                    |  **57.58 %** |   16775.3  MiB/s   |  **1557.6 MiB/s** |   38.86 %   |
| 48 Bit Packed (Byte)             |  **55.01 %** | **16106.6  MiB/s** |  **1385.7 MiB/s** |   37.48 %   |
| 64 Bit (Symbol)                  |    61.69 %   |   17521.9  MiB/s   |    1557.9 MiB/s   |   41.02 %   |
| 64 Bit Packed (Symbol)           |  **58.87 %** | **17252.6  MiB/s** |    1353.7 MiB/s   |   39.42 %   |
| 64 Bit (Byte)                    |  **59.94 %** | **17737.0  MiB/s** |  **1606.6 MiB/s** |   40.05 %   |
| 64 Bit Packed (Byte)             |  **57.33 %** |   16491.9  MiB/s   |  **1421.2 MiB/s** |   38.65 %   |
| 64 Bit 3LUT (Byte)               |  **56.28 %** | **17019.3  MiB/s** |     532.3 MiB/s   |   38.04 %   |
| 128 Bit (Symbol)                 |    66.89 %   |   18760.9  MiB/s   |    1489.9 MiB/s   |   43.85 %   |
| 128 Bit Packed (Symbol)          |  **64.32 %** | **20061.6  MiB/s** |    1415.5 MiB/s   |   42.35 %   |
| 128 Bit (Byte)                   |    65.39 %   |   19088.4  MiB/s   |    1512.1 MiB/s   |   42.96 %   |
| 128 Bit Packed (Byte)            |  **62.94 %** | **19334.9  MiB/s** |    1461.5 MiB/s   |   41.61 %   |
| 8 Bit RLE + Huffman-esque        |    42.52 %   |    1236.2  MiB/s   |     271.7 MiB/s   |   32.69 %   |
| 8 Bit MMTF 128                   |    63.49 %   |    1299.9  MiB/s   |    1101.2 MiB/s   | **29.84 %** |
| Low Entropy                      |    64.19 %   |     913.9  MiB/s   |     381.1 MiB/s   |   34.06 %   |
| Low Entropy Short                |    66.17 %   |    1403.6  MiB/s   |     394.1 MiB/s   |   36.11 %   |
| Multi MTF 128 Bit (Transform)    |   100.00 %   |    1289.8  MiB/s   |    1205.9 MiB/s   |   33.92 %   |
| Multi MTF 256 Bit (Transform)    |   100.00 %   |    1966.1  MiB/s   |    2012.2 MiB/s   |   35.65 %   |
| - | - | - | - | - |
| memcpy                           |   100.0 %   |  27,031.0 MiB/s   |   26,776.9 MiB/s   |   65.94 %   |
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
| 8 Bit                         |  99.99 %   |   19424.5 MiB/s   |    7124.0 MiB/s   |   76.10 %   |
| 8 Bit Short                   |  99.99 %   |   19330.5 MiB/s   |    6909.2 MiB/s   |   75.86 %   |
| 8 Bit Packed                  |  99.99 %   |   19228.6 MiB/s   |    7121.2 MiB/s   |   75.71 %   |
| 8 Bit 1LUT Short              |  99.98 %   |   19662.0 MiB/s   |    7010.9 MiB/s   |   75.63 %   |
| 8 Bit 3LUT                    |  99.99 %   |   19476.0 MiB/s   |    6985.0 MiB/s   |   75.94 %   |
| 8 Bit 3LUT Short              |  99.98 %   |   19147.7 MiB/s   |    6876.9 MiB/s   |   75.94 %   |
| 8 Bit 7LUT                    |  99.99 %   |   19186.6 MiB/s   |    6857.3 MiB/s   |   75.86 %   |
| 8 Bit 7LUT Short              |  99.98 %   |   18994.0 MiB/s   |    6816.8 MiB/s   |   75.78 %   |
| 16 Bit (Symbol)               |  99.99 %   |   18923.3 MiB/s   |    1009.7 MiB/s   |   76.02 %   |
| 16 Bit (Byte)                 |  99.99 %   |   18740.3 MiB/s   |    1011.7 MiB/s   |   76.02 %   |
| 24 Bit (Symbol)               |   1.84 %   |   33067.5 MiB/s   |    5852.0 MiB/s   |    1.32 %   |
| 24 Bit Short (Symbol)         |   2.08 %   |   32713.6 MiB/s   |    5336.3 MiB/s   |    1.52 %   |
| 24 Bit Packed (Symbol)        |   2.00 %   |   32955.6 MiB/s   |    5487.5 MiB/s   |    1.39 %   |
| 24 Bit 1LUT Short (Symbol)    |   2.08 %   |   32751.7 MiB/s   |    4886.8 MiB/s   |    1.51 %   |
| 24 Bit 3LUT (Symbol)          | **1.31 %** | **33073.5 MiB/s** |  **5335.0 MiB/s** |    0.93 %   |
| 24 Bit 3LUT Short (Symbol)    |   1.53 %   |   32890.5 MiB/s   |    5194.7 MiB/s   |    1.04 %   |
| 24 Bit 7LUT (Symbol)          | **1.20 %** | **32846.9 MiB/s** |  **4670.1 MiB/s** |  **0.83 %** |
| 24 Bit 7LUT Short (Symbol)    |   1.41 %   |   31750.2 MiB/s   |    5222.2 MiB/s   |    0.92 %   |
| 24 Bit (Byte)                 | **2.13 %** | **33113.4 MiB/s** |    6052.7 MiB/s   |    1.44 %   |
| 24 Bit Short (Byte)           |   2.17 %   |   32825.6 MiB/s   |    5632.3 MiB/s   |    1.58 %   |
| 24 Bit Packed (Byte)          |   2.32 %   |   32812.0 MiB/s   |    5603.5 MiB/s   |    1.51 %   |
| 24 Bit 1LUT Short (Byte)      |   2.17 %   |   32835.9 MiB/s   |    5697.6 MiB/s   |    1.59 %   |
| 24 Bit 3LUT (Byte)            |   1.49 %   |   33010.6 MiB/s   |    5614.2 MiB/s   |    1.03 %   |
| 24 Bit 3LUT Short (Byte)      |   1.63 %   |   32713.6 MiB/s   |    4870.9 MiB/s   |    1.14 %   |
| 24 Bit 7LUT (Byte)            | **1.37 %** |   32629.3 MiB/s   |  **5493.1 MiB/s** |    0.98 %   |
| 24 Bit 7LUT Short (Byte)      |   1.52 %   |   31915.9 MiB/s   |    5489.0 MiB/s   |    0.97 %   |
| 32 Bit (Symbol)               |  99.99 %   |   19012.2 MiB/s   |    1006.2 MiB/s   |   76.02 %   |
| 32 Bit (Byte)                 |  99.99 %   |   18861.5 MiB/s   |    1010.5 MiB/s   |   76.10 %   |
| 48 Bit (Symbol)               |   2.78 %   |   32870.0 MiB/s   |    9145.2 MiB/s   |    2.12 %   |
| 48 Bit Short (Symbol)         | **2.79 %   |   32833.3 MiB/s   |  **9738.3 MiB/s** |    2.26 %   |
| 48 Bit Packed (Symbol)        | **2.87 %** |   32913.6 MiB/s   | **10349.4 MiB/s** |    2.17 %   |
| 48 Bit 1LUT Short (Symbol)    |   3.09 %   |   32750.9 MiB/s   |    9293.6 MiB/s   |    2.41 %   |
| 48 Bit 3LUT (Symbol)          | **1.71 %** |   32843.5 MiB/s   |  **9184.1 MiB/s** |    1.33 %   |
| 48 Bit 3LUT Short (Symbol)    |   1.99 %   |   32717.0 MiB/s   |    9188.6 MiB/s   |    1.47 %   |
| 48 Bit 7LUT (Symbol)          | **1.49 %** |   32619.2 MiB/s   |  **8992.3 MiB/s** |    1.15 %   |
| 48 Bit 7LUT Short (Symbol)    |   1.77 %   |   31846.0 MiB/s   |    8628.7 MiB/s   |    1.26 %   |
| 48 Bit (Byte)                 |   3.16 %   |   32911.9 MiB/s   |    9317.6 MiB/s   |    2.31 %   |
| 48 Bit Short (Byte)           |   3.20 %   |   32689.1 MiB/s   |    9709.3 MiB/s   |    2.48 %   |
| 48 Bit Packed (Byte)          |   3.35 %   |   32548.7 MiB/s   |   10199.2 MiB/s   |    2.40 %   |
| 48 Bit 1LUT Short (Byte)      |   3.20 %   |   32603.2 MiB/s   |    9691.9 MiB/s   |    2.48 %   |
| 48 Bit 3LUT (Byte)            | **1.98 %** |   32861.4 MiB/s   |  **9629.9 MiB/s** |    1.49 %   |
| 48 Bit 3LUT Short (Byte)      |   2.13 %   |   32717.9 MiB/s   |    9244.4 MiB/s   |    1.62 %   |
| 48 Bit 7LUT (Byte)            | **1.76 %** |   32615.9 MiB/s   |  **9347.5 MiB/s** |    1.34 %   |
| 48 Bit 7LUT Short (Byte)      |   1.90 %   |   31847.6 MiB/s   |    9069.5 MiB/s   |    1.35 %   |
| 64 Bit (Symbol)               |  99.99 %   |   19061.5 MiB/s   |    1006.1 MiB/s   |   75.87 %   |
| 64 Bit (Byte)                 |  99.99 %   |   18682.2 MiB/s   |    1008.2 MiB/s   |   76.10 %   |
| 128 Bit (Symbol)              |  99.99 %   |   19460.1 MiB/s   |    1002.6 MiB/s   |   75.87 %   |
| 128 Bit (Byte)                |  99.99 %   |   19046.0 MiB/s   |    1005.0 MiB/s   |   76.10 %   |
| 8 Bit RLE + Huffman-esque     |  99.98 %   |   27440.2 MiB/s   |     710.4 MiB/s   |   75.26 %   |
| 8 Bit MMTF 128                |  17.70 %   |    5287.0 MiB/s   |    2978.1 MiB/s   |   12.31 %   |
| Low Entropy                   | 100.00 %   |   28044.6 MiB/s   |     836.0 MiB/s   |   77.03 %   |
| Low Entropy Single            | 100.00 %   |   27929.5 MiB/s   |     838.8 MiB/s   |   77.03 %   |
| Multi MTF 128 Bit (Transform) | 100.00 %   |    3474.5 MiB/s   |    3121.4 MiB/s   |   19.94 %   |
| Multi MTF 256 Bit (Transform) | 100.00 %   |    5030.6 MiB/s   |    4952.9 MiB/s   |   27.52 %   |

| - | - | - | - | - |
| memcpy                      |   100.0 %   |  28,288.5 MiB/s   |   28,261.3 MiB/s   |  77.03 %   |
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
