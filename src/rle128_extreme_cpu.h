#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
  #define RLE128_EXTREME_MAX_COPY_RANGE (127)
  #define RLE128_EXTRRME_FULL_COPY_SIZE (4 + 1)
#else
  #define RLE128_EXTREME_MAX_COPY_RANGE (255)
  #define RLE128_EXTRRME_FULL_COPY_SIZE (4)
#endif

#ifndef PACKED
	#define RLE128_EXTREME_MULTI_MIN_RANGE_SHORT ((16 + 1 + 1) + 2)
	#define RLE128_EXTREME_MULTI_MIN_RANGE_LONG ((16 + 1 + 4 + RLE128_EXTRRME_FULL_COPY_SIZE) + 2)
#else
	#define RLE128_EXTREME_MULTI_MIN_RANGE_SHORT ((1 + 1) + 1)
	#define RLE128_EXTREME_MULTI_MIN_RANGE_MEDIUM ((16 + 1 + 1) + 1)
	#define RLE128_EXTREME_MULTI_MIN_RANGE_LONG ((16 + 1 + 4 + RLE128_EXTRRME_FULL_COPY_SIZE) + 1)
#endif

#ifndef UNBOUND
  #ifdef PACKED
    #define CODEC sym_packed
  #else
    #define CODEC sym
  #endif
#else
  #ifdef PACKED
    #define CODEC byte_packed
  #else
    #define CODEC byte
  #endif
#endif

uint32_t CONCAT3(rle128_, CODEC, _compress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
	if (pIn == NULL || inSize == 0 || pOut == NULL || outSize < rle_compress_bounds(inSize))
		return 0;

	size_t index = 0;

	*(uint32_t *)(&(pOut[index])) = inSize;
	index += sizeof(uint32_t);
	*(uint32_t *)(&(pOut[index])) = 0; // will be compressed size.
	index += sizeof(uint32_t);
	int64_t i = 0;
	int64_t lastRLE = 0;

	typedef __m128i symbol_t;
	const size_t symbolSize = sizeof(symbol_t);

	int64_t count = 0;
	symbol_t symbol = _mm_xor_si128(_mm_setzero_si128(), _mm_loadu_si128((const symbol_t *)pIn));

#ifdef PACKED
	symbol_t lastSymbol = _mm_setzero_si128();
#endif

	const int64_t inSizeSimdScan = inSize - sizeof(__m128i) * 2;
	const int64_t inSizeSimd = inSize - sizeof(__m128i);

	while (i < inSize)
	{
	continue_outer_loop:;
		
		{
			while (i < inSizeSimd)
			{
				const uint32_t cmpMask = _mm_movemask_epi8(_mm_cmpeq_epi8(symbol, _mm_loadu_si128((const symbol_t *)&pIn[i])));

				if (cmpMask == 0xFFFF)
				{
					count += sizeof(symbol_t);
					i += sizeof(symbol_t);
				}
				else
				{
#ifdef UNBOUND
#ifdef _MSC_VER
					unsigned long offset;
					_BitScanForward(&offset, ~cmpMask);
#else
					const uint32_t offset = __builtin_ctz(~cmpMask);
#endif

					i += offset;
					count += offset;
#endif
					break;
				}
			}
		}
		
		{
			{
				const int64_t range = i - lastRLE - count + 1;
				
#ifndef PACKED
        if (range <= RLE128_EXTREME_MAX_COPY_RANGE && count >= RLE128_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
        if (range <= RLE128_EXTREME_MAX_COPY_RANGE && ((count >= RLE128_EXTREME_MULTI_MIN_RANGE_SHORT && _mm_movemask_epi8(_mm_cmpeq_epi8(symbol, lastSymbol)) == 0xFFFF) || (count >= RLE128_EXTREME_MULTI_MIN_RANGE_MEDIUM)))
#endif
				{
#ifndef PACKED
					_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
					index += symbolSize;
#endif

#ifndef UNBOUND
					const int64_t storedCount = (count / symbolSize) - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
					const int64_t storedCount = count - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
					if (storedCount <= 255)
					{
						pOut[index] = (uint8_t)storedCount;
						index++;
					}
					else
					{
						pOut[index] = 0;
						index++;
						*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
						index += sizeof(uint32_t);
					}
#else
					const uint8_t isSameSymbolMask = ((_mm_movemask_epi8(_mm_cmpeq_epi8(symbol, lastSymbol)) == 0xFFFF) << 7);
					lastSymbol = symbol;

					if (storedCount <= 0b01111111)
					{
						pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
						index++;
					}
					else
					{
						pOut[index] = isSameSymbolMask;
						index++;
						*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
						index += sizeof(uint32_t);
					}

					if (!isSameSymbolMask)
					{
						_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
						index += symbolSize;
					}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
					pOut[index] = (uint8_t)(range << 1);
					index++;
#else
					pOut[index] = (uint8_t)range;
					index++;
#endif

					const size_t copySize = i - count - lastRLE;

					memcpy(pOut + index, pIn + lastRLE, copySize);
					index += copySize;

					lastRLE = i;
				}
				else if (count >= RLE128_EXTREME_MULTI_MIN_RANGE_LONG)
				{
#ifndef PACKED
					_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
					index += symbolSize;
#endif
					
#ifndef UNBOUND
					const int64_t storedCount = (count / symbolSize) - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
					const int64_t storedCount = count - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
					if (storedCount <= 255)
					{
						pOut[index] = (uint8_t)storedCount;
						index++;
					}
					else
					{
						pOut[index] = 0;
						index++;
						*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
						index += sizeof(uint32_t);
					}
#else
					const uint8_t isSameSymbolMask = ((_mm_movemask_epi8(_mm_cmpeq_epi8(symbol, lastSymbol)) == 0xFFFF) << 7);
					lastSymbol = symbol;

					if (storedCount <= 0b01111111)
					{
						pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
						index++;
					}
					else
					{
						pOut[index] = isSameSymbolMask;
						index++;
						*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
						index += sizeof(uint32_t);
					}

					if (!isSameSymbolMask)
					{
						_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
						index += symbolSize;
					}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
					*((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | 1;
					index += sizeof(uint32_t);
#else
					pOut[index] = 0;
					index++;
					*((uint32_t *)&pOut[index]) = (uint32_t)range;
					index += sizeof(uint32_t);
#endif

					const size_t copySize = i - count - lastRLE;

					memcpy(pOut + index, pIn + lastRLE, copySize);
					index += copySize;

					lastRLE = i;
				}
			}

			while (i < inSizeSimdScan)
			{
				const __m128i current = _mm_loadu_si128((const __m128i *)(pIn + i));
				const __m128i other = _mm_loadu_si128((const __m128i *)(pIn + i + 16));
				const __m128i matchMask = _mm_cmpeq_epi8(current, other);
				const uint32_t bitMask = _mm_movemask_epi8(matchMask);

				if (bitMask == 0xFFFF)
				{
					symbol = _mm_loadu_si128((const symbol_t *)&pIn[i]);

					i += symbolSize * 2;
					count = symbolSize * 2;

					goto continue_outer_loop;
				}
				else if ((bitMask & 0x8000) == 0)
				{
					i += symbolSize;
				}
				else
				{
					const uint32_t hiMask = (~bitMask) << 16;

#ifdef _MSC_VER
		      unsigned long bit;
		      _BitScanReverse(&bit, hiMask);
#else
		      const uint32_t bit = 31 - __builtin_clz(hiMask);
#endif

					i += (bit - 15);
				}
			}

			symbol = _mm_loadu_si128((const symbol_t *)&pIn[i]);

			if (i + symbolSize <= inSize && _mm_movemask_epi8(_mm_cmpeq_epi8(symbol, _mm_loadu_si128((const symbol_t *)((&pIn[i]) + symbolSize)))) == 0xFFFF)
			{
				count = symbolSize * 2;
				i += symbolSize * 2;
			}
			else
			{
				count = 0;
				i++;
			}
		}
	}

	{
		const int64_t range = i - lastRLE - count + 1;

#ifndef PACKED
		if (range <= RLE128_EXTREME_MAX_COPY_RANGE && count >= RLE128_EXTREME_MULTI_MIN_RANGE_SHORT)
#else
		if (range <= RLE128_EXTREME_MAX_COPY_RANGE && ((count >= RLE128_EXTREME_MULTI_MIN_RANGE_SHORT && _mm_movemask_epi8(_mm_cmpeq_epi8(symbol, lastSymbol)) == 0xFFFF) || (count >= RLE128_EXTREME_MULTI_MIN_RANGE_MEDIUM)))
#endif
		{
#ifndef PACKED
			_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
			index += symbolSize;
#endif

#ifndef UNBOUND
			const int64_t storedCount = (count / symbolSize) - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
			const int64_t storedCount = count - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
			if (storedCount <= 255)
			{
				pOut[index] = (uint8_t)storedCount;
				index++;
			}
			else
			{
				pOut[index] = 0;
				index++;
				*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
				index += sizeof(uint32_t);
			}
#else
			const uint8_t isSameSymbolMask = ((_mm_movemask_epi8(_mm_cmpeq_epi8(symbol, lastSymbol)) == 0xFFFF) << 7);
			lastSymbol = symbol;

			if (storedCount <= 0b01111111)
			{
				pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
				index++;
			}
			else
			{
				pOut[index] = isSameSymbolMask;
				index++;
				*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
				index += sizeof(uint32_t);
			}

			if (!isSameSymbolMask)
			{
				_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
				index += symbolSize;
			}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
			pOut[index] = (uint8_t)(range << 1);
			index++;
#else
			pOut[index] = (uint8_t)range;
			index++;
#endif

			const size_t copySize = i - count - lastRLE;

			memcpy(pOut + index, pIn + lastRLE, copySize);
			index += copySize;

#ifndef PACKED
			_mm_store_si128((symbol_t *)(&pOut[index]), _mm_setzero_si128());
			index += symbolSize;
			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);
#else
			pOut[index] = 0b10000000;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);
#endif

			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);

			lastRLE = i;
		}
		else if (count >= RLE128_EXTREME_MULTI_MIN_RANGE_LONG)
		{
#ifndef PACKED
			_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
			index += symbolSize;
#endif
			
#ifndef UNBOUND
			const int64_t storedCount = (count / symbolSize) - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / symbolSize) + 1;
#else
			const int64_t storedCount = count - (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT) + 1;
#endif

#ifndef PACKED
			if (storedCount <= 255)
			{
				pOut[index] = (uint8_t)storedCount;
				index++;
			}
			else
			{
				pOut[index] = 0;
				index++;
				*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
				index += sizeof(uint32_t);
			}
#else
			const uint8_t isSameSymbolMask = ((_mm_movemask_epi8(_mm_cmpeq_epi8(symbol, lastSymbol)) == 0xFFFF) << 7);
			lastSymbol = symbol;

			if (storedCount <= 0b01111111)
			{
				pOut[index] = (uint8_t)storedCount | isSameSymbolMask;
				index++;
			}
			else
			{
				pOut[index] = isSameSymbolMask;
				index++;
				*(uint32_t *)&(pOut[index]) = (uint32_t)storedCount;
				index += sizeof(uint32_t);
			}

			if (!isSameSymbolMask)
			{
				_mm_store_si128((symbol_t *)(&pOut[index]), symbol);
				index += symbolSize;
			}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
			*((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | 1;
			index += sizeof(uint32_t);
#else
			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = (uint32_t)range;
			index += sizeof(uint32_t);
#endif

			const size_t copySize = i - count - lastRLE;

			memcpy(pOut + index, pIn + lastRLE, copySize);
			index += copySize;

#ifndef PACKED
			_mm_store_si128((symbol_t *)(&pOut[index]), _mm_setzero_si128());
			index += symbolSize;
			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);
#else
			pOut[index] = 0b10000000;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);
#endif

			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);

			lastRLE = i;
		}
		else
		{
#ifndef PACKED
			_mm_store_si128((symbol_t *)(&pOut[index]), _mm_setzero_si128());
			index += symbolSize;
			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);
#else
			pOut[index] = 0b10000000;
			index++;
			*((uint32_t *)&pOut[index]) = 0;
			index += sizeof(uint32_t);
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
			*((uint32_t *)&pOut[index]) = (uint32_t)(range << 1) | 1;
			index += sizeof(uint32_t);
#else
			pOut[index] = 0;
			index++;
			*((uint32_t *)&pOut[index]) = (uint32_t)range;
			index += sizeof(uint32_t);
#endif

			const size_t copySize = i - lastRLE;

			memcpy(pOut + index, pIn + lastRLE, copySize);
			index += copySize;
		}
	}

	// Store compressed length.
	((uint32_t *)pOut)[1] = (uint32_t)index;

	return (uint32_t)index;
}

static void CONCAT3(rle128_, CODEC, _decompress_sse)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
	size_t offset, symbolCount;
	__m128i symbol = _mm_setzero_si128();

	typedef __m128i symbol_t;

	while (true)
	{
#ifndef PACKED
		symbol = _mm_loadu_si128((const symbol_t *)pInStart);
		pInStart += sizeof(symbol_t);
		symbolCount = (size_t)* pInStart;
		pInStart++;

		if (symbolCount == 0)
		{
			symbolCount = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);
		}
#else
		symbolCount = (size_t)*pInStart;
		pInStart++;

		const uint8_t sameSymbol = (symbolCount & 0b10000000);
		symbolCount &= 0b01111111;

		if (symbolCount == 0)
		{
			symbolCount = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);
		}

		if (!sameSymbol)
		{
			symbol = _mm_loadu_si128((const symbol_t *)pInStart);
			pInStart += sizeof(symbol_t);
		}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
		offset = (size_t)*pInStart;

		if (offset & 1)
		{
			offset = (*(uint32_t *)pInStart) >> 1;
			pInStart += sizeof(uint32_t);

			if (offset == 0)
				return;
		}
		else
		{
			pInStart++;
			offset >>= 1;
		}

		offset--;
#else
		offset = (size_t)* pInStart;
		pInStart++;

		if (offset == 0)
		{
			offset = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);

			if (offset == 0)
				return;
		}

		offset--;
#endif

		// memcpy.
		MEMCPY_SSE_MULTI;

		if (!symbolCount)
			return;

#ifndef UNBOUND
		symbolCount = (symbolCount + (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
		symbolCount = (symbolCount + RLE128_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

		// memset.
		MEMSET_SSE_MULTI;
	}
}

#ifndef _MSC_VER
__attribute__((target("avx")))
#endif
static void CONCAT3(rle128_, CODEC, _decompress_avx)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
	size_t offset, symbolCount;
	__m256i symbol = _mm256_setzero_si256();

	typedef __m128i symbol_t;

	while (true)
	{
#ifndef PACKED
		symbol = _mm256_castps_si256(_mm256_broadcast_ps((const __m128 *)pInStart));
		pInStart += sizeof(symbol_t);
		symbolCount = (size_t)* pInStart;
		pInStart++;

		if (symbolCount == 0)
		{
			symbolCount = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);
		}

		offset = (size_t)* pInStart;
		pInStart++;
#else
		symbolCount = (size_t)*pInStart;
		pInStart++;

		const uint8_t sameSymbol = (symbolCount & 0b10000000);
		symbolCount &= 0b01111111;

		if (symbolCount == 0)
		{
			symbolCount = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);
		}

		if (!sameSymbol)
		{
			symbol = _mm256_castps_si256(_mm256_broadcast_ps((const __m128 *)pInStart));
			pInStart += sizeof(symbol_t);
		}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
		offset = (size_t)*pInStart;

		if (offset & 1)
		{
			offset = (*(uint32_t *)pInStart) >> 1;
			pInStart += sizeof(uint32_t);

			if (offset == 0)
				return;
		}
		else
		{
			pInStart++;
			offset >>= 1;
		}

		offset--;
#else
		offset = (size_t)*pInStart;
		pInStart++;

		if (offset == 0)
		{
			offset = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);

			if (offset == 0)
				return;
		}

		offset--;
#endif

		// memcpy.
		MEMCPY_AVX_MULTI;

		if (!symbolCount)
			return;

#ifndef UNBOUND
		symbolCount = (symbolCount + (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
		symbolCount = (symbolCount + RLE128_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

		// memset.
		MEMSET_AVX_MULTI;
	}
}

#ifndef _MSC_VER
__attribute__((target("avx512f")))
#endif
static void CONCAT3(rle128_, CODEC, _decompress_avx512f)(IN const uint8_t *pInStart, OUT uint8_t *pOut)
{
	size_t offset, symbolCount;
	__m512i symbol = _mm512_setzero_si512();

	typedef __m128i symbol_t;

	while (true)
	{
#ifndef PACKED
		symbol = _mm512_castps_si512(_mm512_broadcast_f32x4(_mm_loadu_ps((const float *)pInStart)));
		pInStart += sizeof(symbol_t);
		symbolCount = (size_t)*pInStart;
		pInStart++;

		if (symbolCount == 0)
		{
			symbolCount = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);
		}
#else
		symbolCount = (size_t)*pInStart;
		pInStart++;

		const uint8_t sameSymbol = (symbolCount & 0b10000000);
		symbolCount &= 0b01111111;

		if (symbolCount == 0)
		{
			symbolCount = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);
		}

		if (!sameSymbol)
		{
			symbol = _mm512_castps_si512(_mm512_broadcast_f32x4(_mm_loadu_ps((const float *)pInStart)));
			pInStart += sizeof(symbol_t);
		}
#endif

#ifdef PREFER_7_BIT_OR_4_BYTE_COPY
		offset = (size_t)*pInStart;

		if (offset & 1)
		{
			offset = (*(uint32_t *)pInStart) >> 1;
			pInStart += sizeof(uint32_t);

			if (offset == 0)
				return;
		}
		else
		{
			pInStart++;
			offset >>= 1;
		}

		offset--;
#else
		offset = (size_t)*pInStart;
		pInStart++;

		if (offset == 0)
		{
			offset = *(uint32_t *)pInStart;
			pInStart += sizeof(uint32_t);

			if (offset == 0)
				return;
		}

		offset--;
#endif

		// memcpy.
		MEMCPY_AVX512_MULTI;

		if (!symbolCount)
			return;

#ifndef UNBOUND
		symbolCount = (symbolCount + (RLE128_EXTREME_MULTI_MIN_RANGE_SHORT / sizeof(symbol_t)) - 1) * sizeof(symbol_t);
#else
		symbolCount = (symbolCount + RLE128_EXTREME_MULTI_MIN_RANGE_SHORT - 1);
#endif

		// memset.
		MEMSET_AVX512_MULTI;
	}
}

uint32_t CONCAT3(rle128_, CODEC, _decompress)(IN const uint8_t *pIn, const uint32_t inSize, OUT uint8_t *pOut, const uint32_t outSize)
{
	if (pIn == NULL || pOut == NULL || inSize == 0 || outSize == 0)
		return 0;

	const size_t expectedInSize = ((uint32_t *)pIn)[1];
	const size_t expectedOutSize = ((uint32_t *)pIn)[0];

	if (expectedOutSize > outSize || expectedInSize > inSize)
		return 0;

	size_t index = sizeof(uint32_t) * 2;

	_DetectCPUFeatures();

	pIn += index;

	if (avx512FSupported)
		CONCAT3(rle128_, CODEC, _decompress_avx512f)(pIn, pOut);
	else if (avxSupported)
		CONCAT3(rle128_, CODEC, _decompress_avx)(pIn, pOut);
	else
		CONCAT3(rle128_, CODEC, _decompress_sse)(pIn, pOut);

	return (uint32_t)expectedOutSize;
}

#undef RLE128_EXTREME_MAX_COPY_RANGE
#undef RLE128_EXTRRME_FULL_COPY_SIZE

#undef RLE128_EXTREME_MULTI_MIN_RANGE_SHORT
#undef RLE128_EXTREME_MULTI_MIN_RANGE_LONG

#ifdef RLE128_EXTREME_MULTI_MIN_RANGE_MEDIUM
  #undef RLE128_EXTREME_MULTI_MIN_RANGE_MEDIUM
#endif

#undef CODEC
