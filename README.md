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
 - Running on an `Intel(R) Core(TM) i5-3470S CPU @ 2.90GHz` / `Intel(R) Core(TM) i7-8809G CPU @ 3.10GHz` on Windows 10.
 - Compiled with the `Visual Studio 2015` toolset in `Visual Studio 2017`.

##### `video_frame.raw` (DCTs of an encoded heavily quantized raw YUV video frame)
- Multiple Run Length Encodable Symbols

`Intel(R) Core(TM) i5-3470S CPU @ 2.90GHz`: AVX Variant
```
Compressed 88473600 bytes -> 7802546 bytes (8.819067 %) in 210.656250 ms.
Decompressed in 40.343750 ms.
```

`Intel(R) Core(TM) i7-8809G CPU @ 3.10GHz`: AVX Variant
```
Compressed 88473600 bytes -> 7802546 bytes (8.819067 %) in 129.437500 ms.
Decompressed in 28.375000 ms.
```

##### `data.txt` (Text file with one symbol that is worth being run length encoded)
- Single Run Length Encodable Symbol.

`Intel(R) Core(TM) i5-3470S CPU @ 2.90GHz`: SSE2 Variant
```
Compressed 133436430 bytes -> 17700604 bytes (13.265196 %) in 301.468750 ms.
Decompressed in 21.875000 ms.
```

`Intel(R) Core(TM) i7-8809G CPU @ 3.10GHz`: AVX2 Variant
```
Compressed 133436430 bytes -> 17700604 bytes (13.265196 %) in 141.562500 ms.
Decompressed in 14.187500 ms.
```

##### `non-compressible.txt` (Text file with lots of random characters and very few repeating symbols)
- Multiple Run Length Encodable Symbols

`Intel(R) Core(TM) i5-3470S CPU @ 2.90GHz`: AVX Variant
```
Compressed 223722720 bytes -> 213763765 bytes (95.548528 %) in 623.555556 ms.
Decompressed in 243.777778 ms.
```

`Intel(R) Core(TM) i7-8809G CPU @ 3.10GHz`: AVX Variant
```
Compressed 223722720 bytes -> 213763765 bytes (95.548528 %) in 399.312500 ms.
Decompressed in 117.562500 ms.
```

##### `non-compressible-single.txt` (Text file with lots of random characters and one repeating symbol)
- Single Run Length Encodable Symbol.

`Intel(R) Core(TM) i5-3470S CPU @ 2.90GHz`: SSE2 Variant
```
Compressed 67679040 bytes -> 44772338 bytes (66.153920 %) in 187.050000 ms.
Decompressed in 52.150000 ms.
```

`Intel(R) Core(TM) i7-8809G CPU @ 3.10GHz`: AVX2 Variant
```
Compressed 67679040 bytes -> 44772338 bytes (66.153920 %) in 99.562500 ms.
Decompressed in 29.375000 ms.
```

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