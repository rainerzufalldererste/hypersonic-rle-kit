# rle8

### What is it?
- Possibly the fastest run length en/decoder (obviously dependent on the dataset).
- Tries to keep symbol general symbol frequency to improve compression ratio of an entropy encoder that could go after the Run Length Encoding like ANS, Arithmetic Coding or Huffman.
- Written in C.
- SIMD Variants for AVX2, AVX, SSSE3 and SSE2 are available for the decoder. Automatically picked by the decoder based on the extensions available on the current platform.
- Specialized versions for various different scenarios. (Single RLE Symbol, Short Strings of RLE Symbol, 8 Bit, 16 Bit, 24 Bit, 32 Bit, 48 Bit, 64 Bit)
- `OpenCL` variant available for some of the decoders.

### Benchmark
 - Single-Threaded
 - Running on an `Intel(R) Xeon(R) CPU E5-1680 v3 @ 3.20GHz` on Windows 10.
 - Compiled with `Visual Studio 2015`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE)

#### `video-frame.raw` (heavily quantized video frame DCTs, 88.473.600 Bytes)
| Type | Compression Ratio | Encoding Speed | Decoding Speed | Entropy Compressible To |
| -- | -- | -- | -- | -- |
| rle8 Normal                |  19.93 % |  446.0 MiB/s | 1497.3 MiB/s |  9.05 % |
| rle8 Normal Single         |  19.96 % |  446.9 MiB/s | 2362.0 MiB/s |  9.10 % |
| rle8 Ultra                 |  24.08 % |  425.5 MiB/s | 1585.4 MiB/s | 11.61 % |
| rle8 Ultra  Single         |  24.11 % |  426.2 MiB/s | 2617.4 MiB/s | 11.69 % |
| **rle8 Extreme  8 Bit**        | **19.38 %** |  772.0 MiB/s | **7629.2 MiB/s** |  9.47 % |
| **rle8 Extreme  8 Bit Single** | **18.36 %** |  406.5 MiB/s | **7740.2 MiB/s** |  9.69 % |
| **rle8 Extreme 16 Bit**        | **20.32 %** | **1084.4 MiB/s** | **7565.9 MiB/s** |  9.56 % |
| **rle8 Extreme 24 Bit**        | **21.50 %** | **1256.8 MiB/s** | **7706.5 MiB/s** |  9.45 % |
| **rle8 Extreme 32 Bit**        | **22.01 %** | **1382.3 MiB/s** | **7794.0 MiB/s** |  9.67 % |
| **rle8 Extreme 48 Bit**        | **23.41 %** | **1508.5 MiB/s** | **7821.3 MiB/s** |  9.60 % |
| **rle8 Extreme 64 Bit**        | **24.53 %** | 1807.0 MiB/s | **8146.0 MiB/s** |  9.61 % |
| - | - | - | - | - |
| memcpy                     | 100.00 % | 7454.5 MiB/s | 7442.8 MiB/s | 14.025 % |
| trle    | 16.0 % |  688.86 | 2976.10 | - |
| srle 0  | 17.8 % |  421.62 | 5771.27 | - |
| srle 8  | 18.7 % | 1035.85 | 6289.44 | - |
| srle 16 | 21.3 % |  975.94 | 7225.28 | - |
| srle 32 | 24.2 % | 1696.55 | 7940.55 | - |
| srle 64 | 27.5 % | 2824.83 | 8846.48 | - |
| mrle    | 19.7 % |  217.13 | 1389.76 | - |

#### [1034.db](http://encode.su/threads/2077-EGTB-compression?p=41392&viewfull=1#post41392) (Checkers program "End Game Table Base", 419.225.625 Bytes)
| Type | Compression Ratio | Encoding Speed | Decoding Speed | Entropy Compressible To |
| -- | -- | -- | -- | -- |
| rle8 Normal                | 21.15 % |  409.0 MiB/s | 2445.9 MiB/s | 10.37 % |
| rle8 Normal Single         | 21.15 % |  412.7 MiB/s | 2457.1 MiB/s | 10.37 % |
| rle8 Ultra                 | 24.87 % |  392.4 MiB/s | 2713.7 MiB/s | 14.20 % |
| rle8 Ultra  Single         | 24.87 % |  391.9 MiB/s | 2704.4 MiB/s | 14.20 % |
| **rle8 Extreme  8 Bit** | **23.02 %** |  676.4 MiB/s | **6523.0 MiB/s** | 12.08 % |
| **rle8 Extreme  8 Bit Single** | **20.59 %** |  364.5 MiB/s | **6844.6 MiB/s** | 12.06 % |
| **rle8 Extreme 16 Bit** | **24.89 %** |  853.1 MiB/s | **6365.2 MiB/s** | 12.27 % |
| **rle8 Extreme 24 Bit** | **27.27 %** |  **978.7 MiB/s** | **6487.3 MiB/s** | 11.95 % |
| **rle8 Extreme 32 Bit** | 28.39 % |  992.8 MiB/s | 6597.8 MiB/s | 12.27 % |
| **rle8 Extreme 48 Bit** | 31.22 % | 1129.1 MiB/s | 6687.6 MiB/s | 12.02 % |
| **rle8 Extreme 64 Bit** | 33.36 % | 1259.9 MiB/s | 7187.6 MiB/s | 11.98 % |
| - | - | - | - | - |
| memcpy                     | 100.00 % | 7296.5 MiB/s | 7284.0 MiB/s | 16.76 % |
| trle    | 17.4 % |  628.88 MB/s | 2467.25 MB/s | - |
| srle 0  | 20.2 % |  388.40 MB/s | 4768.86 MB/s | - |
| srle 8  | 22.0 % |  878.50 MB/s | 5474.78 MB/s | - |
| srle 16 | 27.1 % |  800.06 MB/s | 5925.12 MB/s | - |
| srle 32 | 32.7 % | 1310.59 MB/s | 7329.50 MB/s | - |
| srle 64 | 39.5 % | 2138.32 MB/s | 8332.19 MB/s | - |
| mrle    | 21.0 % |  206.70 MB/s | 1210.46 MB/s | - |

The 24 Bit and 48 Bit Variants allow for run length encoding of common data layouts that are usually not covered by RLE implementations:

#### [Pixel Art Bitmap Image](https://i.redd.it/tj5oyhhuehv11.png) (PNG converted to BMP, 123.710.454 Bytes)
| Type | Compression Ratio | Encoding Speed | Decoding Speed | Entropy Compressible To |
| -- | -- | -- | -- | -- |
| rle8 Normal         | 100.00 % |  534.4 MiB/s | 6996.0 MiB/s | 77.03 % |
| rle8 Extreme  8 Bit |  99.99 % |  723.2 MiB/s | 6447.9 MiB/s | 76.10 % |
| rle8 Extreme 16 Bit |  99.99 % |  601.1 MiB/s | 6303.2 MiB/s | 76.02 % |
| **rle8 Extreme 24 Bit** |   **1.84 %** | **2387.7 MiB/s** | **8585.1 MiB/s** |  1.32 % |
| rle8 Extreme 32 Bit |  99.99 % |  597.4 MiB/s | 6469.2 MiB/s | 76.02 % |
| **rle8 Extreme 48 Bit** |   **2.78 %** | **3528.2 MiB/s** | **8612.6 MiB/s** |  2.12 % |
| rle8 Extreme 64 Bit |  99.99 % |  589.6 MiB/s | 6543.7 MiB/s | 75.87 % |
| - | - | - | - | - |
| memcpy              | 100.00 % | 7425.3 MiB/s | 7399.2 MiB/s | 77.03 % |
| trle    | 100.0 % | 190.00 MB/s | 2275.99 MB/s | - |
| srle 0  | 100.0 % |  73.89 MB/s | 6887.61 MB/s | - |
| srle 8  | 100.0 % | 202.73 MB/s | 6541.97 MB/s | - |
| srle 16 | 100.0 % | 230.61 MB/s | 6846.39 MB/s | - |
| srle 32 | 100.0 % | 448.01 MB/s | 6539.39 MB/s | - |
| srle 64 | 100.0 % | 832.76 MB/s | 6741.36 MB/s | - |
| mrle    | 100.0 % |  92.28 MB/s | 1576.94 MB/s | - |

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