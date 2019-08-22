# rle8

### What is it?
- A fast run length en/decoder.
- Tries to keep symbol general symbol frequency to improve compression ratio of an entropy encoder that could go after the Run Length Encoding like ANS, Arithmetic Coding or Huffman.
- Written in C.
- SIMD Variants for AVX2, AVX and SSE2 are available for the decoder. Automatically picked by the decoder based on the features available on the CPU it's running on.
- `OpenCL` variant available for some of the decoders.
- Specialized versions for single run length encodable symbol.
- Specialized versions for various different scenarios.

### Benchmark
 - Single-Threaded
 - Running on an `Intel(R) Xeon(R) CPU E5-1680 v3 @ 3.20GHz` on Windows 10.
 - Compiled with `Visual Studio 2015`.
 - Compared to [TurboRLE](https://github.com/powturbo/TurboRLE)
##### [1034.db](http://encode.su/threads/2077-EGTB-compression?p=41392&viewfull=1#post41392) (Checkers program "End Game Table Base")

| Type | Compressed Size (Ratio) | Encoding Speed | Decoding Speed | Size (Ratio) with `rans_static_32x16` |
| -- | -- | -- | -- | -- |
| - | 419.225.625 Bytes (100.00%) | - | - | 56.728.176 Bytes (13.53%) |
| rle8 | 88.666.372 Bytes (21.15%) | 412.500954 MB/s | 2469.096279 MB/s | 43.940.318 Bytes (10.48%) |
| rle8 single | 88.666.372 Bytes (21.15%) | 414.679 MB/s | 2474.987 MB/s | 43.940.318 Bytes (10.48%) |
| rle8 ultra | 104.249.934 Bytes (24.87%) | 393.790 MB/s | 2715.048 MB/s | 48.666.568 Bytes (11.61%) |
| rle8 ultra single | 104.249.934 Bytes (24.87%) | 394.349 MB/s | 2710.763 MB/s | 48.666.568 Bytes (11.61%) |
| rle8 extreme 8 bit | 96.495.695 Bytes (23.02%) | 677.071 MB/s | **6521.398 MB/s** | 51.048.980 Bytes (12.18%) |
| rle8 extreme 8 bit single | 86.326.906 Bytes (20.59%) | 361.772 MB/s | **6820.208 MB/s** | 50.940.427 Bytes (12.15%) |
| rle8 extreme 16 bit | 104.335.593 Bytes (24.89%) | 753.408 MB/s | **6456.242 MB/s** | 51.955.612 Bytes (12.39%) |
| rle8 extreme 32 bit | 118.999.253 Bytes (28.39%) | 1149.847 MB/s | **6663.612 MB/s** | 52.076.188 Bytes (12.42%) |
| rle8 extreme 64 bit | 139.860.053 Bytes (33.36%) | 1419.075 MB/s | **7263.287 MB/s** | 50.986.906 Bytes (12.16%) |
| - | - | - | - | - |
| trle | 73.108.990 (17.4%) | 633.02 MB/s | 2493.27 MB/s | - |
| srle 0 | 84.671.759 (20.2%) | 390.43 MB/s | 4783.11 MB/s | - |
| srle 8 | 92.369.848 (22.0%) | 886.09 MB/s | 5300.75 MB/s | - |
| srle 16 | 113.561.537 (27.1%) | 804.69 MB/s | 5948.99 MB/s | - |
| srle 32 | 136.918.311 (32.7%) | 1310.77 MB/s | 7372.94 MB/s | - |
| srle 64 | 165.547.365 (39.5%) | 2140.93 MB/s | 8391.23 MB/s | - |
| mrle | 88.055.360 (21.0%) | 207.23 MB/s | 1206.61 MB/s | - |
| memcpy | 419.225.625 (100.0%) | 7686.57 MB/s | - | - |

##### `video-frame.raw` (heavily quantized video frame DCTs)
| Type | Compressed Size (Ratio) | Encoding Speed | Decoding Speed | Size (Ratio) with `rans_static_32x16` |
| -- | -- | -- | -- | -- |
| - | 88.473.600 Bytes (100.00%) | - | - | 11.378.953 Bytes (12.86%) |
| rle8 | 17.630.322 Bytes (19.93%) | 449.978 MB/s | 1497.118 MB/s | 8.099.993 Bytes (9.16%) |
| rle8 single | 176.578.37 Bytes (19.96%) | 450.186 MB/s | 2378.925 MB/s | 8.138.195 Bytes (9.20%) |
| rle8 ultra | 21.306.466 Bytes (24.08%) | 432.428 MB/s | 1602.872 MB/s | 9.306.772 Bytes (10.52%) |
| rle8 ultra single | 21.332.661 Bytes (24.11%) | 428.820 MB/s | 2657.480 MB/s | 9.342.219 Bytes (10.56%) |
| rle8 extreme 8 bit | 17.147.077 Bytes (19.38%) | 771.743 MB/s | **7730.117 MB/s** | 8.435.522 Bytes (9.53%) |
| rle8 extreme 8 bit single | 16.242.653 Bytes (18.36%) | 403.398 MB/s | **7738.341 MB/s** | 8.628.014 Bytes (9.75%) |
| rle8 extreme 16 bit | 17.980.330 Bytes (20.32%) | 1009.200 MB/s | **7662.235 MB/s** | 8.526.123 Bytes (9.64%) |
| rle8 extreme 32 bit | 19.473.112 Bytes (22.01%) | 1522.979 MB/s | **7853.659 MB/s** | 8.636.199 Bytes (9.76%) |
| rle8 extreme 64 bit | 21.703.102 Bytes (24.53%) | 1858.194 MB/s | **8227.052 MB/s** | 8.595.611 Bytes (9.72%) |
| - | - | - | - | - |
| trle | 14187432 (16.0%) | 690.37 MB/s | 2974.10 MB/s | - |
| srle 0 | 15743523 (17.8%) | 423.49 MB/s | 5686.69 MB/s | - |
| srle 8 | 16555349 (18.7%) | 1003.01 MB/s | 6193.03 MB/s | - |
| srle 16 | 18868388 (21.3%) | 1033.75 MB/s | 7139.00 MB/s | - |
| srle 32 | 21390380 (24.2%) | 1689.23 MB/s | 8122.06 MB/s | - |
| srle 64 | 24311530 (27.5%) | 2820.68 MB/s | 8809.48 MB/s | - |
| mrle | 17420113 (19.7%) | 215.72 MB/s | 1320.74 MB/s | - |
| memcpy | 88473600 (100.0%) | 7568.31 MB/s | - | - |

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