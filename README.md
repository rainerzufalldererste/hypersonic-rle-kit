# hypersonic rle kit

### What is it?
- A selection of various RLE and related codecs optimized for all kinds of different inputs.
- Possibly the fastest run length en/decoder (obviously dependent on the dataset). **Single Core Decompression Speeds > 33 GiB/s have been observed.**
- Written in C.
- SIMD Variants for AVX-512F, AVX2, AVX, SSE4.1, SSSE3 and SSE2 are available for many decoders. Automatically picked by the en- & decoder based on the extensions available on the current platform.
- Specialized versions for various different scenarios. (Single RLE Symbol, Short Strings of RLE Symbols, Few Repeating RLE-Symbols, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit, 128 Bit)
- `OpenCL` variant available for some of the decoders.

### Benchmark
 - Single-Threaded
 - Running on an `AMD Ryzen 9 7950X`, `32 GB DDR5-6000 CL30` on Windows 11.
 - Compiled with `Visual Studio 2022`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) and [MRLE](https://encode.su/threads/2121-No-more-encoding-overhead-in-Run-Length-Encoding-Read-about-Mespotine-RLE-here-) (with AVX2 enabled; benchmarking results have been converted from MB/s to MiB/s)

#### [video-frame.raw](https://www.dropbox.com/s/yvsl1lg98c4maq1/video_frame.raw?dl=1) (heavily quantized video frame DCTs, 88,473,600 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| **rle8 Extreme  8 Bit**               | **19.4 %** | **2,366.1 MiB/s** | **22,990.5 MiB/s** |   9.47 %   |
| **rle8 Extreme  8 Bit Single**        | **18.3 %** |   1,265.4 MiB/s   | **19,731.8 MiB/s** |   9.69 %   |
| **rle8 Extreme 16 Bit**               | **20.3 %** |   1,736.8 MiB/s   | **17,659.4 MiB/s** |   9.56 %   |
| **rle8 Extreme 24 Bit**               | **21.5 %** | **1,795.9 MiB/s** | **19,589.7 MiB/s** |   9.45 %   |
| **rle8 Extreme 32 Bit**               | **22.0 %** | **2,207.0 MiB/s** | **17,710.2 MiB/s** |   9.67 %   |
| **rle8 Extreme 48 Bit**               | **23.4 %** | **2,181.7 MiB/s** | **20,051.1 MiB/s** |   9.60 %   |
| **rle8 Extreme 64 Bit**               | **24.5 %** |   2,502.0 MiB/s   | **21,495.2 MiB/s** |   9.61 %   |
| rle8 Extreme 128 Bit                  |   27.6 %   |   2,351.1 MiB/s   | **25,200.1 MiB/s** |   9.76 %   |
| rle8 SH (2 symbol huffman-esque)      | **12.5 %** |     305.6 MiB/s   |    1,225.3 MiB/s   |   9.82 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |   21.1 %   |   2,250.8 MiB/s   |    2,758.1 MiB/s   |  10.72 %   |
| rle8 Normal                           |   19.9 %   |     709.5 MiB/s   |    2,145.3 MiB/s   | **9.05 %**   |
| rle8 Normal Single                    |   20.0 %   |     721.5 MiB/s   |    3,526.1 MiB/s   | **9.10 %**   |
| rle8 Ultra                            |   24.0 %   |     667.2 MiB/s   |    2,379.2 MiB/s   |  11.61 %   |
| rle8 Ultra  Single                    |   24.1 %   |     721.8 MiB/s   |    4,918.3 MiB/s   |  11.69 %   |
| rle8 Raw Multi MTF 128 Bit            |  100.0 %   |   2,599.1 MiB/s   |    2,754.5 MiB/s   |  15.63 %   |
| rle8 Raw Multi MTF 256 Bit            |  100.0 %   |   3,983.9 MiB/s   |    4,048.8 MiB/s   |  15.88 %   |
| - | - | - | - | - | 
| memcpy                                |  100.0 %   |  28,510.8 MiB/s   |   28,689.2 MiB/s   |  14.03 %   |
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
| **rle8 Extreme  8 Bit**               | **23.0 %** | **2,498.1 MiB/s** | **21,756.1 MiB/s** |   12.08 %   |
| **rle8 Extreme  8 Bit Single**        | **20.6 %** |   1,341.3 MiB/s   | **18,903.9 MiB/s** |   12.06 %   |
| **rle8 Extreme 16 Bit**               | **24.9 %** | **1,757.8 MiB/s** | **17,486.1 MiB/s** |   12.27 %   |
| **rle8 Extreme 24 Bit**               | **27.3 %** | **1,747.5 MiB/s** | **20,567.8 MiB/s** |   11.95 %   |
| **rle8 Extreme 32 Bit**               | **28.4 %** | **1,898.8 MiB/s** | **17,563.0 MiB/s** |   12.27 %   |
| **rle8 Extreme 48 Bit**               | **31.2 %** | **2,270.4 MiB/s** | **21,732.9 MiB/s** |   12.02 %   |
| **rle8 Extreme 64 Bit**               | **33.4 %** |   2,525.2 MiB/s   | **21,296.6 MiB/s** |   11.98 %   |
| rle8 Extreme 128 Bit                  |   39.8 %   |   2,452.4 MiB/s   | **23,915.3 MiB/s** |   12.16 %   |
| rle8 SH (2 symbol huffman-esque)      | **16.8 %** |     334.7 MiB/s   |    1,475.2 MiB/s   |   12.36 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |   26.6 %   |   2,196.3 MiB/s   |    2,887.2 MiB/s   |   14.85 %   |
| rle8 Normal                           |   21.2 %   |     805.2 MiB/s   |    4,403.9 MiB/s   | **10.37 %** |
| rle8 Normal Single                    |   21.2 %   |     789.2 MiB/s   |    4,410.7 MiB/s   | **10.37 %** |
| rle8 Ultra                            |   24.9 %   |     749.7 MiB/s   |    5,461.4 MiB/s   |   14.20 %   |
| rle8 Ultra  Single                    |   24.9 %   |     818.7 MiB/s   |    5,460.6 MiB/s   |   14.20 %   |
| rle8 Raw Multi MTF 128 Bit            |  100.0 %   |   2,615.4 MiB/s   |    2,722.3 MiB/s   |   17.90 %   |
| rle8 Raw Multi MTF 256 Bit            |  100.0 %   |   4,109.8 MiB/s   |    4,153.8 MiB/s   |   18.35 %   |
| - | - | - | - | - |
| memcpy                                |  100.0 %   |  27,165.1 MiB/s   |   27,100.4 MiB/s   |   16.76 %   |
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
| **rle8 Extreme  8 Bit**               |    48.8 %   | **1,250.3 MiB/s** | **14,734.6 MiB/s** |   34.13 %   |
| rle8 Extreme  8 Bit Single            |    88.8 %   | **1,158.9 MiB/s** | **16,231.9 MiB/s** |   60.14 %   |
| **rle8 Extreme 16 Bit**               |    51.4 %   | **1,092.8 MiB/s** | **12,975.6 MiB/s** |   35.70 %   |
| **rle8 Extreme 24 Bit**               |    54.9 %   | **1,186.8 MiB/s** | **15,942.0 MiB/s** |   37.33 %   |
| **rle8 Extreme 32 Bit**               |    55.9 %   | **1,278.5 MiB/s** | **14,234.6 MiB/s** |   38.17 %   |
| **rle8 Extreme 48 Bit**               |    59.2 %   | **1,472.0 MiB/s** | **16,838.5 MiB/s** |   39.77 %   |
| rle8 Extreme 64 Bit                   |    61.7 %   |   1,557.3 MiB/s   | **17,915.4 MiB/s** |   41.02 %   |
| rle8 Extreme 128 Bit                  |    66.9 %   |   1,531.7 MiB/s   | **19,479.4 MiB/s** |   43.85 %   |
| rle8 SH (2 symbol huffman-esque)      |    42.5 %   |     279.5 MiB/s   |    1,269.4 MiB/s   |   32.69 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |    63.5 %   |   1,127.2 MiB/s   |    1,321.6 MiB/s   | **29.84 %** |
| rle8 Normal                           |    66.2 %   |     392.2 MiB/s   |      932.7 MiB/s   |   34.06 %   |
| rle8 Normal Single                    |    89.7 %   |     568.1 MiB/s   |    7,625.6 MiB/s   |   59.50 %   |
| rle8 Ultra                            |    64.2 %   |     379.5 MiB/s   |    1,422.7 MiB/s   |   36.11 %   |
| rle8 Ultra  Single                    |    89.2 %   |     614.1 MiB/s   |    9,937.4 MiB/s   |   61.05 %   |
| rle8 Raw Multi MTF 128 Bit            |   100.0 %   |   1,239.4 MiB/s   |    1,323.1 MiB/s   |   33.92 %   |
| rle8 Raw Multi MTF 256 Bit            |   100.0 %   |   2,022.4 MiB/s   |    1,945.4 MiB/s   |   35.65 %   |
| - | - | - | - | - |
| memcpy                                |   100.0 %   |  27,031.0 MiB/s   |   26,776.9 MiB/s   |   65.94 %   |
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
| **rle8 Extreme 8 Bit**                |  **99.9 %** | **7,251.4 MiB/s** |   19,463.7 MiB/s   |  76.18 %   |
| rle8 Extreme 16 Bit                   |    99.9 %   |   1,043.2 MiB/s   |   19,117.1 MiB/s   |  76.02 %   |
| **rle8 Extreme 24 Bit**               |   **1.8 %** | **4,584.6 MiB/s** | **33,015.7 MiB/s** | **1.32 %** |
| rle8 Extreme 32 Bit                   |    99.9 %   |   1,043.1 MiB/s   |   19,441.8 MiB/s   |  76.02 %   |
| **rle8 Extreme 48 Bit**               |   **2.8 %** | **7,963.2 MiB/s** | **32,913.6 MiB/s** | **2.12 %** |
| rle8 Extreme 64 Bit                   |    99.9 %   |   1,042.2 MiB/s   |   19,274.0 MiB/s   |  75.87 %   |
| rle8 Extreme 128 Bit                  |    99.9 %   |   1,042.5 MiB/s   |   19,223.3 MiB/s   |  75.87 %   |
| rle8 SH (2 symbol huffman-esque)      |    99.9 %   |     739.6 MiB/s   | **27,933.2 MiB/s** |  75.26 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |  **17.7 %** | **3,077.3 MiB/s** |    5,489.1 MiB/s   |  12.31 %   |
| rle8 Normal                           |   100.0 %   |     860.4 MiB/s   | **27,531.0 MiB/s** |  77.03 %   |
| rle8 Raw Multi MTF 128 Bit            |   100.0 %   |   3,223.1 MiB/s   |    3,603.1 MiB/s   |  19.94 %   |
| rle8 Raw Multi MTF 256 Bit            |   100.0 %   |   5,087.7 MiB/s   |    5,163.8 MiB/s   |  27.52 %   |
| - | - | - | - | - |
| memcpy                                |   100.0 %   |  28,288.5 MiB/s   |   28,261.3 MiB/s   |  77.03 %   |
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

### 3 Sym LUT / 3 Sym LUT Short
- Similar to the base variant, but keeps around three of the previously used RLE symbols, usually further improving compression ratios.
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
#include "rle8.h"

uint8_t *pUncompressedData; // Some Data.
uint32_t fileSize; // Some Size.

// Get Compress Bounds.
const uint32_t compressedBufferSize = rle8_compress_bounds(fileSize);
uint8_t *pCompressedData = (uint8_t *)malloc(compressedBufferSize);

// Compress.
const uint32_t compressedSize = rle8_compress(pUncompressedData, fileSize, pCompressedData, compressedBufferSize);

// Allocate Output Buffer.
uint8_t *pDecompressedData = (uint8_t *)malloc(fileSize);

// Decompress.
const uint32_t decompressedSize = rle8_decompress(pCompressedData, compressedSize, pDecompressedData, fileSize);

// Cleanup.
free(pCompressedData);
free(pDecompressedData);
```

### License
Two Clause BSD
