# rle8

### What is it?
- Possibly the fastest run length en/decoder (obviously dependent on the dataset).
- Tries to keep symbol general symbol frequency to improve compression ratio of an entropy encoder that could go after the Run Length Encoding like ANS, Arithmetic Coding or Huffman.
- Written in C.
- SIMD Variants for AVX2, AVX, SSSE3 and SSE2 are available for the decoder. Automatically picked by the en- & decoder based on the extensions available on the current platform.
- Specialized versions for various different scenarios. (Single RLE Symbol, Short Strings of RLE Symbol, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit)
- `OpenCL` variant available for some of the decoders.

### Benchmark
 - Single-Threaded
 - Running on an `Intel(R) Core(TM) i9-9900K CPU @ 3.60GHz` on Windows 10. (Thanks, Silv3rfire!)
 - Compiled with `Visual Studio 2017`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) (with AVX2 enabled)

#### [video-frame.raw](https://www.dropbox.com/s/yvsl1lg98c4maq1/video_frame.raw?dl=1) (heavily quantized video frame DCTs, 88.473.600 Bytes)
| Type | Compression Ratio | Encoding Speed | Decoding Speed | Entropy Compressible To |
| -- | -- | -- | -- | -- |
| rle8 Normal                    |   19.93 %   |    581.4 MiB/s   |    1961.4 MiB/s   |  9.05 % |
| rle8 Normal Single             |   19.96 %   |    580.6 MiB/s   |    3318.0 MiB/s   |  9.10 % |
| rle8 Ultra                     |   24.08 %   |    578.1 MiB/s   |    2314.4 MiB/s   | 11.61 % |
| rle8 Ultra  Single             |   24.11 %   |    570.6 MiB/s   |    3644.5 MiB/s   | 11.69 % |
| **rle8 Extreme  8 Bit**        | **19.38 %** | **2147.4 MiB/s** | **13791.5 MiB/s** |  9.47 % |
| **rle8 Extreme  8 Bit Single** | **18.36 %** |    528.4 MiB/s   | **12222.8 MiB/s** |  9.69 % |
| **rle8 Extreme 16 Bit**        | **20.32 %** |   1354.3 MiB/s   | **12384.4 MiB/s** |  9.56 % |
| **rle8 Extreme 24 Bit**        | **21.50 %** |   1620.4 MiB/s   | **14162.6 MiB/s** |  9.45 % |
| **rle8 Extreme 32 Bit**        | **22.01 %** |   1810.8 MiB/s   | **12994.4 MiB/s** |  9.67 % |
| **rle8 Extreme 48 Bit**        | **23.41 %** |   2328.5 MiB/s   | **14190.0 MiB/s** |  9.60 % |
| **rle8 Extreme 64 Bit**        | **24.53 %** |   2059.8 MiB/s   | **13783.4 MiB/s** |  9.61 % |
| rle8 Extreme 128 Bit           |   27.60 %   |   2016.4 MiB/s   |   14262.9 MiB/s   |  9.76 % |
| - | - | - | - | - |
| memcpy                         | 100.00 %    | 13213.9 MiB/s    | 13061.1 MiB/s     | 14.03 % |
| trle    | 16.0 % | 1012.87 MB/s |  4406.06 MB/s | - |
| srle 0  | 17.8 % |  617.40 MB/s |  8449.39 MB/s | - |
| srle 8  | 18.7 % | 1433.54 MB/s |  9362.29 MB/s | - |
| srle 16 | 21.3 % | 1284.93 MB/s | 11175.14 MB/s | - |
| srle 32 | 24.2 % | 2200.07 MB/s | 12883.88 MB/s | - |
| srle 64 | 27.5 % | 3876.00 MB/s | 14621.32 MB/s | - |
| mrle    | 19.7 % |  272.39 MB/s |  1752.16 MB/s | - |

#### [1034.db](http://encode.su/threads/2077-EGTB-compression?p=41392&viewfull=1#post41392) (Checkers program "End Game Table Base", 419.225.625 Bytes)
| Type | Compression Ratio | Encoding Speed | Decoding Speed | Entropy Compressible To |
| -- | -- | -- | -- | -- |
| rle8 Normal                    |   21.15 %   |    535.2 MiB/s   |    3327.8 MiB/s   | 10.37 % |
| rle8 Normal Single             |   21.15 %   |    536.7 MiB/s   |    3352.0 MiB/s   | 10.37 % |
| rle8 Ultra                     |   24.87 %   |    526.6 MiB/s   |    3704.0 MiB/s   | 14.20 % |
| rle8 Ultra  Single             |   24.87 %   |    521.2 MiB/s   |    3698.2 MiB/s   | 14.20 % |
| **rle8 Extreme  8 Bit**        | **23.02 %** | **1482.5 MiB/s** | **12473.0 MiB/s** | 12.08 % |
| **rle8 Extreme  8 Bit Single** | **20.59 %** |    473.3 MiB/s   | **10676.4 MiB/s** | 12.06 % |
| **rle8 Extreme 16 Bit**        | **24.89 %** | **1118.5 MiB/s** | **10503.6 MiB/s** | 12.27 % |
| **rle8 Extreme 24 Bit**        | **27.27 %** | **1345.8 MiB/s** | **12788.1 MiB/s** | 11.95 % |
| **rle8 Extreme 32 Bit**        | **28.39 %** | **1485.0 MiB/s** | **10928.2 MiB/s** | 12.27 % |
| **rle8 Extreme 48 Bit**        | **31.22 %** | **1758.2 MiB/s** | **13129.2 MiB/s** | 12.02 % |
| rle8 Extreme 64 Bit            |   33.36 %   |   1623.6 MiB/s   |   12586.9 MiB/s   | 11.98 % |
| **rle Extreme 128 Bit**        | **39.84 %** |   1521.6 MiB/s   | **13386.2 MiB/s** | 12.16 % |
| - | - | - | - | - |
| memcpy                         |  100.00 %   |   12839.1 MiB/s  |  12832.8 MiB/s   | 16.76 % |
| trle    | 17.4 % |  628.88 MB/s |  3399.41 MB/s | - |
| srle 0  | 20.2 % |  388.40 MB/s |  6602.60 MB/s | - |
| srle 8  | 22.0 % |  878.50 MB/s |  7365.56 MB/s | - |
| srle 16 | 27.1 % |  800.06 MB/s |  8441.24 MB/s | - |
| srle 32 | 32.7 % | 1310.59 MB/s | 10800.05 MB/s | - |
| srle 64 | 39.5 % | 2138.32 MB/s | 12875.48 MB/s | - |
| mrle    | 21.0 % |  206.70 MB/s |  2508.83 MB/s | - |

The 24 Bit and 48 Bit Variants allow for run length encoding of common data layouts that are usually not covered by RLE implementations:

#### [Pixel Art Bitmap Image](https://i.redd.it/tj5oyhhuehv11.png) (PNG converted to BMP, 123.710.454 Bytes)
| Type | Compression Ratio | Encoding Speed | Decoding Speed | Entropy Compressible To |
| -- | -- | -- | -- | -- |
| rle Normal                   |   100.00 %   |    691.6 MiB/s   |   11875.4 MiB/s   | 77.03 % |
| **rle Extreme 8 Bit**        |  **99.99 %** | **4892.4 MiB/s** |   10863.6 MiB/s   | 76.18 % |
| rle Extreme 16 Bit           |    99.99 %   |    581.5 MiB/s   |   11166.5 MiB/s   | 76.02 % |
| **rle Extreme 24 Bit**       |   **1.84 %** | **3710.0 MiB/s** | **16767.0 MiB/s** |  1.32 % |
| rle Extreme 32 Bit           |    99.99 %   |    609.9 MiB/s   |   11268.7 MiB/s   | 76.02 % |
| **rle Extreme 48 Bit**       |   **2.78 %** | **6407.1 MiB/s** | **16446.9 MiB/s** |  2.12 % |
| rle Extreme 64 Bit           |    99.99 %   |    592.6 MiB/s   |   10949.2 MiB/s   | 75.87 % |
| rle Extreme 128 Bit          |    99.99 %   |    598.1 MiB/s   |   10896.0 MiB/s   | 75.87 % |
| - | - | - | - | - |
| memcpy                       |   100.00 %   |  13089.0 MiB/s   |   13086.8 MiB/s   | 77.03 % |
| trle    | 100.0 % |  286.44 |  4002.61 MB/s | - |
| srle 0  | 100.0 % |  108.65 | 12157.43 MB/s | - |
| srle 8  | 100.0 % |  309.41 | 11858.68 MB/s | - |
| srle 16 | 100.0 % |  302.41 | 11937.61 MB/s | - |
| srle 32 | 100.0 % |  590.61 | 12068.98 MB/s | - |
| srle 64 | 100.0 % | 1184.28 | 11911.90 MB/s | - |
| mrle    | 100.0 % |  132.03 |  2036.78 MB/s | - |

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

### SIMD Decoder Variants
##### For Single Run Length Encodable Symbol
- AVX2
- SSE2
- OpenCL

##### For Multiple Run Length Encodable Symbol
- AVX
- SSE2
- OpenCL

### License
Two Clause BSD