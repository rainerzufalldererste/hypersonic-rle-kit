# rle8

### What is it?
- Possibly the fastest run length en/decoder (obviously dependent on the dataset). **Single Core Decompression Speeds > 17.5 GB/s have been observed.**
- Many different variants optimized for various scenarios.
- Written in C.
- SIMD Variants for AVX-512F, AVX2, AVX, SSSE3 and SSE2 are available for the decoder. Automatically picked by the en- & decoder based on the extensions available on the current platform.
- Specialized versions for various different scenarios. (Single RLE Symbol, Short Strings of RLE Symbol, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit, 128 Bit)
- `OpenCL` variant available for some of the decoders.

### Benchmark
 - Single-Threaded
 - Running on an `Intel(R) Core(TM) i9-9900K CPU @ 3.60GHz` on Windows 10. (Thanks, Silv3rfire!)
 - Compiled with `Visual Studio 2017`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) and [MRLE](https://encode.su/threads/2121-No-more-encoding-overhead-in-Run-Length-Encoding-Read-about-Mespotine-RLE-here-) (with AVX2 enabled; benchmarking results have been converted from MB/s to MiB/s)

#### [video-frame.raw](https://www.dropbox.com/s/yvsl1lg98c4maq1/video_frame.raw?dl=1) (heavily quantized video frame DCTs, 88,473,600 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| **rle8 Extreme  8 Bit**               | **19.4 %** | **2,147.4 MiB/s** | **13,791.5 MiB/s** |   9.47 %   |
| **rle8 Extreme  8 Bit Single**        | **18.3 %** |     528.4 MiB/s   | **12,222.8 MiB/s** |   9.69 %   |
| **rle8 Extreme 16 Bit**               | **20.3 %** |   1,354.3 MiB/s   | **12,384.4 MiB/s** |   9.56 %   |
| **rle8 Extreme 24 Bit**               | **21.5 %** |   1,620.4 MiB/s   | **14,162.6 MiB/s** |   9.45 %   |
| **rle8 Extreme 32 Bit**               | **22.0 %** |   1,810.8 MiB/s   | **12,994.4 MiB/s** |   9.67 %   |
| **rle8 Extreme 48 Bit**               | **23.4 %** | **2,328.5 MiB/s** | **14,190.0 MiB/s** |   9.60 %   |
| **rle8 Extreme 64 Bit**               | **24.5 %** |   2,059.8 MiB/s   | **13,783.4 MiB/s** |   9.61 %   |
| rle8 Extreme 128 Bit                  |   27.6 %   |   2,016.4 MiB/s   | **14,262.9 MiB/s** |   9.76 %   |
| rle8 SH (2 symbol huffman-esque)      | **12.5 %** |     188.0 MiB/s   |    1,178.8 MiB/s   |   9.82 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |   21.1 %   |   1,943.9 MiB/s   |    2,509.4 MiB/s   |  10.72 %   |
| rle8 Normal                           |   19.9 %   |     581.4 MiB/s   |    1,961.4 MiB/s   | **9.05 %** |
| rle8 Normal Single                    |   20.0 %   |     580.6 MiB/s   |    3,318.0 MiB/s   |   9.10 %   |
| rle8 Ultra                            |   24.0 %   |     578.1 MiB/s   |    2,314.4 MiB/s   |  11.61 %   |
| rle8 Ultra  Single                    |   24.1 %   |     570.6 MiB/s   |    3,644.5 MiB/s   |  11.69 %   |
| rle8 Raw Multi MTF 128 Bit            |  100.0 %   |   2,151.6 MiB/s   |    2,428.1 MiB/s   |  15.63 %   |
| rle8 Raw Multi MTF 256 Bit            |  100.0 %   |   3,615.9 MiB/s   |    4,103.3 MiB/s   |  15.88 %   |
| - | - | - | - | - | 
| memcpy                                |  100.0 %   |  13,213.9 MiB/s   |   13,061.1 MiB/s   |  14.03 %   |
| trle    | 16.0 % |   965.95 MiB/s |  4,201.95 MiB/s | - |
| srle 0  | 17.8 % |   588.80 MiB/s |  8,057.97 MiB/s | - |
| srle 8  | 18.7 % | 1,367.13 MiB/s |  8,928.58 MiB/s | - |
| srle 16 | 21.3 % | 1,225.40 MiB/s | 10,657.44 MiB/s | - |
| srle 32 | 24.2 % | 2,098.15 MiB/s | 12,287.03 MiB/s | - |
| srle 64 | 27.5 % | 3,696.44 MiB/s | 13,943.98 MiB/s | - |
| mrle    | 19.7 % |   259.77 MiB/s |  1,670.99 MiB/s | - |

#### [1034.db](http://encode.su/threads/2077-EGTB-compression?p=41392&viewfull=1#post41392) (Checkers program "End Game Table Base", 419,225,625 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| **rle8 Extreme  8 Bit**               | **23.0 %** | **1,482.5 MiB/s** | **12,473.0 MiB/s** |   12.08 %   |
| **rle8 Extreme  8 Bit Single**        | **20.6 %** |     473.3 MiB/s   | **10,676.4 MiB/s** |   12.06 %   |
| **rle8 Extreme 16 Bit**               | **24.9 %** | **1,118.5 MiB/s** | **10,503.6 MiB/s** |   12.27 %   |
| **rle8 Extreme 24 Bit**               | **27.3 %** | **1,345.8 MiB/s** | **12,788.1 MiB/s** |   11.95 %   |
| **rle8 Extreme 32 Bit**               | **28.4 %** | **1,485.0 MiB/s** | **10,928.2 MiB/s** |   12.27 %   |
| **rle8 Extreme 48 Bit**               | **31.2 %** | **1,758.2 MiB/s** | **13,129.2 MiB/s** |   12.02 %   |
| **rle8 Extreme 64 Bit**               | **33.4 %** |   1,623.6 MiB/s   | **12,586.9 MiB/s** |   11.98 %   |
| rle8 Extreme 128 Bit                  |   39.8 %   |   1,521.6 MiB/s   |   13,386.2 MiB/s   |   12.16 %   |
| rle8 SH (2 symbol huffman-esque)      | **16.8 %** |     180.3 MiB/s   |      962.6 MiB/s   |   12.36 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |   26.6 %   |   1,435.7 MiB/s   |    1,943.9 MiB/s   |   14.85 %   |
| rle8 Normal                           |   21.2 %   |     535.2 MiB/s   |    3,327.8 MiB/s   | **10.37 %** |
| rle8 Normal Single                    |   21.2 %   |     536.7 MiB/s   |    3,352.0 MiB/s   | **10.37 %** |
| rle8 Ultra                            |   24.9 %   |     526.6 MiB/s   |    3,704.0 MiB/s   |   14.20 %   |
| rle8 Ultra  Single                    |   24.9 %   |     521.2 MiB/s   |    3,698.2 MiB/s   |   14.20 %   |
| rle8 Raw Multi MTF 128 Bit            |  100.0 %   |   1,655.4 MiB/s   |    1,855.1 MiB/s   |   17.90 %   |
| rle8 Raw Multi MTF 256 Bit            |  100.0 %   |   2,688.3 MiB/s   |    3,089.9 MiB/s   |   18.35 %   |
| - | - | - | - | - |
| memcpy                                |  100.0 %   |  12,839.1 MiB/s   |   12,832.8 MiB/s   |   16.76 %   |
| trle    | 17.4 % |   599.75 MiB/s |  3,241.93 MiB/s | - |
| srle 0  | 20.2 % |   370.41 MiB/s |  6,296.73 MiB/s | - |
| srle 8  | 22.0 % |   837.80 MiB/s |  7,024.35 MiB/s | - |
| srle 16 | 27.1 % |   763.00 MiB/s |  8,050.19 MiB/s | - |
| srle 32 | 32.7 % | 1,249.88 MiB/s | 10,299.73 MiB/s | - |
| srle 64 | 39.5 % | 2,039.26 MiB/s | 12,279.01 MiB/s | - |
| mrle    | 21.0 % |   197.12 MiB/s |  2,392.61 MiB/s | - |

The 24 Bit and 48 Bit Variants allow for run length encoding of common data layouts that are usually not covered by RLE implementations:

#### [Pixel Art Bitmap Image](https://i.redd.it/tj5oyhhuehv11.png) (PNG converted to BMP, 123,710,454 Bytes)
| Type | Ratio | Encoding Speed | Decoding Speed | Entropy<br/>Compressible To |
| -- | --: | --: | --: | --: |
| **rle8 Extreme 8 Bit**                |  **99.9 %** | **4,892.4 MiB/s** |   10,863.6 MiB/s   |  76.18 %   |
| rle8 Extreme 16 Bit                   |    99.9 %   |     581.5 MiB/s   |   11,166.5 MiB/s   |  76.02 %   |
| **rle8 Extreme 24 Bit**               |   **1.8 %** | **3,710.0 MiB/s** | **16,767.0 MiB/s** | **1.32 %** |
| rle8 Extreme 32 Bit                   |    99.9 %   |     609.9 MiB/s   |   11,268.7 MiB/s   |  76.02 %   |
| **rle8 Extreme 48 Bit**               |   **2.8 %** | **6,407.1 MiB/s** | **16,446.9 MiB/s** | **2.12 %** |
| rle8 Extreme 64 Bit                   |    99.9 %   |     592.6 MiB/s   |   10,949.2 MiB/s   |  75.87 %   |
| rle8 Extreme 128 Bit                  |    99.9 %   |     598.1 MiB/s   |   10,896.0 MiB/s   |  75.87 %   |
| rle8 SH (2 symbol huffman-esque)      |    99.9 %   |     404.2 MiB/s   |   15,345.4 MiB/s   |  75.26 %   |
| rle8 Extreme 8 MMTF 128 (dynamic-int) |  **17.7 %** |   1,565.5 MiB/s   |    2,769.1 MiB/s   |  12.31 %   |
| rle8 Normal                           |   100.0 %   |     691.6 MiB/s   |   11,875.4 MiB/s   |  77.03 %   |
| rle8 Raw Multi MTF 128 Bit            |   100.0 %   |   1,737.8 MiB/s   |    1,812.7 MiB/s   |  19.94 %   |
| rle8 Raw Multi MTF 256 Bit            |   100.0 %   |   2,408.1 MiB/s   |    2,474.1 MiB/s   |  27.52 %   |
| - | - | - | - | - |
| memcpy                                |   100.0 %   |  13,089.0 MiB/s   |   13,086.8 MiB/s   |  77.03 %   |
| trle    | 100.0 % |   273.17 MiB/s |  3,817.19 MiB/s | - |
| srle 0  | 100.0 % |   103.62 MiB/s | 11,594.23 MiB/s | - |
| srle 8  | 100.0 % |   295.08 MiB/s | 11,309.32 MiB/s | - |
| srle 16 | 100.0 % |   288.40 MiB/s | 11,384.59 MiB/s | - |
| srle 32 | 100.0 % |   563.25 MiB/s | 11,509.88 MiB/s | - |
| srle 64 | 100.0 % | 1,129.42 MiB/s | 11,360.07 MiB/s | - |
| mrle    | 100.0 % |   125.91 MiB/s |  1,942.42 MiB/s | - |

### Variants
#### rle8 Normal
- Tries to keep symbol general symbol frequency to improve compression ratio of an entropy encoder that could go after the Run Length Encoding like ANS, Arithmetic Coding or Huffman.
- Parses the output for run-length-encodable symbols, which are specified in the header.
- Has a single symbol variant, that only encodes the most run-length-encodable symbol (useful for some image codecs).

#### rle8 Ultra
- Same as rle8 Normal, but optimized for shorter strings of run-length-encodable symbols.

#### rle8 Extreme
- 8, 16, 24, 32, 48, 64, 128 bit variants
- Decoder interprets blocks of data to boil down to a highly optimized `memcpy`, `memset`, `memcpy`, `memset` (with various different byte-lengths).
- 8 bit encoder highly optimized as well, optional variant single symbol encoding.

#### rle8 Extreme MMTF
- Runs a block-wide vectorized MTF transform on the input and depending on how many bits this needs to represent a given block (if it's not entirely representable by a variant of `memset`) uses only the required amount of bits to encode the block.
- Performs well on a wide variety of inputs, but usually doesn't produce the best compression ratios or (de-) compression speeds, as all blocks need to be decoded and cannot simply be `memcpy`d if they don't contain an encodable symbol.

#### rle8 SH
- Uses a separate header, that contains a huffman-esque instructions to place recent high-prevalence symbols, copy or set a specific symbol to a block.
- Usually very high compression ratios, but comparably slow to encode.

#### rle8 Raw MMTF
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
