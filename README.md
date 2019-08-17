# rle8

### What is it?
- A fast 8 bit run length en/decoder.
- Tries to keep symbol general symbol frequency to improve compression ratio of an entropy encoder that could go after the Run Length Encoding like ANS, Arithmetic Coding or Huffman.
- Written in C.
- SIMD Variants for AVX2, AVX and SSE2 are available for the decoder. Automatically picked by the decoder based on the features available on the CPU it's running on.
- `OpenCL` variant available for the decoder.
- Specialized versions for single run length encodable symbol.

### Benchmark
 - Single-Threaded
 - Running on an `Intel(R) Xeon(R) CPU E5-1680 v3 @ 3.20GHz` on Windows 10.
 - Compiled with `Visual Studio 2015`.
 - 32 Test runs in rle8 (`-r 32`)
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE) (includes to non-8 bit run length encodings)

#### File with single run length encodable symbol. (Contains longer regions containing the same char) [132.002.960 Bytes]
| Mode | Compressed File Size | Compression Rate | Compression Speed | Decompression Speed | Compression rate of result (using `rans_static_32x16`) |
| -- | -- | -- | -- | -- | -- |
| - | 132.002.960 Bytes | 100 % | - | - | 33.838 % |
| rle8 Normal | 74.804.293 Bytes | 56.669 % | 235.125 ms / **528.593 MB/s** | 24.563 ms / **5125.2 MB/s** | 41.538 % |
| rle8 Ultra | 77.725.733 Bytes | 58.882 % | 248.563 ms / **506.463 MB/s** | 28.25 ms / 4417.117 MB/s | 43.64 % |
| trle | 73.804.815 Bytes | 55.9 % | - / 307.96 MB/s | - / 2327.31 MB/s | - |
| srle 0 | 74.573.601 Bytes | 56.5 % | - / 306.58 MB/s | - / 4975.80 MB/s | - |
| srle 8 | 74.573.600 Bytes | 56.5 % | - / 354.67 MB/s | - / 4983.12 MB/s | - |
| srle 16 | 768.800.00 Bytes | 58.2 % | - / 383.60 MB/s | - / 7022.18 MB/s | - |
| srle 32 | 81.262.160 Bytes | 61.6% | - / 762.91  MB/s | - / 7710.45 MB/s | - |
| srle 64 | 89.565.200 Bytes | 67.9% | - / 1427.06  MB/s | - / 7621.42 MB/s | - |
| mrle | 74.804.272 Bytes | 56.7% | - / 135.02 MB/s | - / 1837.48 MB/s | - |
| memcpy | 132.002.960 Bytes | 100.0% | - / 7680.84 MB/s | - / 7680.84 MB/s | - |

#### File with multiple run length encodable symbols. (Heavily quantized DCTs of a video frame) [88.473.600 Bytes]
| Mode | Compressed File Size | Compression Rate | Compression Speed | Decompression Speed | Compression rate of result (using `rans_static_32x16`) |
| -- | -- | -- | -- | -- | -- |
| - | 88.473.600 Bytes | 100 % | - | - | 12.861 %
| rle8 Normal (Single Symbol Mode) | 17.657.837 Bytes | 19.958 % | 189.813 ms / 444.518 MB/s | 37.313 ms / 2261.307 MB/s | 46.088 % |
| rle8 Ultra (Single Symbol Mode) | 21.306.466 Bytes | 24.082 % | 198.656 ms / 424.728 MB/s |  32.906 ms / **2564.103 MB/s** | 43.793 % |
| rle8 Normal | 17.630.322 Bytes | 19.927 % | 189.031 ms / 446.355 MB/s | 57.281 ms / 1472.995 MB/s | 45.944 % |
| rle8 Ultra | 21.306.466 Bytes | 24.082 % | 198.438 ms / 425.197 MB/s | 54.094 ms / 1559.792 MB/s | 43.681 % |
trle | 15.244.992 Bytes | 17.2 % | - / 699.13 MB/s | - / 1707.79 MB/s | - |
srle 0 | 16.555.350 Bytes | 18.7 % | - / 686.07 MB/s | - / 2522.70 MB/s | - |
srle 8 | 16.555.349 Bytes | 18.7 % | - / 983.68 MB/s | - / 2420.88 MB/s | - |
srle 16 | 18.868.388 Bytes | 21.3 % | - / 965.81 MB/s | - / 4149.60 MB/s | - |
srle 32 | 21.390.380 Bytes | 24.2 % | - / 1656.93 MB/s | - / 7370.96 MB/s | - |
srle 64 | 24.311.530 Bytes | 27.5 % | - / 2839.15 MB/s | - / 8882.89 MB/s | - |
mrle | 17.420.113 Bytes | 19.7 % | - / 208.05 MB/s | - / 1261.91 MB/s | - |
memcpy | 88.473.600 Bytes | 100.0 % | - / 7572.85 MB/s | - / 7572.85 MB/s | - |

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