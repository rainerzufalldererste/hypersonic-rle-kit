#ifndef rle8_ocl_kernel_h__
#define rle8_ocl_kernel_h__

#define STRINGIFY(...) #__VA_ARGS__ "\n"

const char *kernelsource = STRINGIFY(
  
  __kernel
  void rle8_decompress(global uchar *pOut, global uchar *pIn, global uchar *pRle, global uchar *pSymbolToCount, global uint *pStartOffsets, uint subSectionSize)
  {
    uint subSectionIndex = get_group_id(0);
    uint subSections = get_global_size(0);
  
    global uchar *pStart = pIn + pStartOffsets[subSectionIndex];
    global uchar *pEnd = pIn + pStartOffsets[subSectionIndex + 1];
  
    bool rle[256];
    uchar symbolToCount[256];
    pOut += subSectionIndex * subSectionSize;
  
    for (uint i = 0; i < 256; i++)
    {
      rle[i] = pRle[i];
      symbolToCount[i] = pSymbolToCount[i];
    }
  
    while (pStart < pEnd)
    {
      uchar b = *pStart;
      *pEnd = b;
      pStart++;
      pEnd++;
  
      if (rle[b])
      {
        uchar count = symbolToCount[*pStart];
        pStart++;
  
        for (uchar i = 0; i < count; i++)
        {
          *pEnd = b;
          pEnd++;
        }
      }
    }
  }

  __kernel
  void rle8_decompress_single(global uchar *pOut, global uchar *pIn, uchar symbol, global uchar *pSymbolToCount, global uint *pStartOffsets, uint subSectionSize)
  {
    uint subSectionIndex = get_group_id(0);
    uint subSections = get_global_size(0);
  
    global uchar *pStart = pIn + pStartOffsets[subSectionIndex];
    global uchar *pEnd = pIn + pStartOffsets[subSectionIndex + 1];
    global uchar *pPreEnd = pEnd - 256;
  
    uchar symbolToCount[256];
    pOut += subSectionIndex * subSectionSize;
  
    for (uint i = 0; i < 256; i++)
      symbolToCount[i] = pSymbolToCount[i];
  
    const uint sym = symbol | (symbol << 8) | (symbol << 16) | (symbol << 24);
  
    while (pStart < pPreEnd)
    {
      uint b = *(global uint *)pStart;
      *(global uint *)pEnd = b;
  
      uint mask = b ^ sym;
      uint contained = ((uint)((mask)-(uint)0x01010101) & ~(mask) & (uint)0x80808080);
  
      if (contained == 0)
      {
        pStart += sizeof(uint);
        pOut += sizeof(uint);
      }
      else
      {
        for (uint i = 0; i < sizeof(uint); i++)
        {
          uchar s = (b & 0xFF);
  
          if (s == symbol)
          {
            pStart += i + 1;
            pOut += i + 1;
  
            uchar count = symbolToCount[*pStart];
            pStart++;
  
            for (uchar i = 0; i < count; i += sizeof(uint))
            {
              *(global uint *)pEnd = sym;
              pEnd += sizeof(uint);
            }
  
            break;
          }
  
          b >>= 8;
        }
      }
    }
  
    while (pStart < pEnd)
    {
      uchar b = *pStart;
      *pEnd = b;
      pStart++;
      pEnd++;
  
      if (b == symbol)
      {
        uchar count = symbolToCount[*pStart];
        pStart++;
  
        for (uchar i = 0; i < count; i++)
        {
          *pEnd = b;
          pEnd++;
        }
      }
    }
  }
);

#endif // rle8_ocl_kernel_h__
