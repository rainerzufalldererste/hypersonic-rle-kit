#ifndef rle8_ocl_kernel_h__
#define rle8_ocl_kernel_h__

#define STRINGIFY(...) #__VA_ARGS__

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

);

#endif // rle8_ocl_kernel_h__
