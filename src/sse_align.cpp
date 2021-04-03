#include <chrono>
#include <vector>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include <cstring>


#ifdef __SSE2__
#include <emmintrin.h>
#define VBROADCAST(v) _mm_set1_epi8(v)
#define VLOAD(v) _mm_loadu_si128((const __m128i*)(v))
#define VOR(x, y) _mm_or_si128(x, y)
#define VEQ(x, y) _mm_cmpeq_epi8(x, y)
#define VLT(x, y) _mm_cmplt_epi8(x, y)
#define VMOVEMASK(v) _mm_movemask_epi8(v)
#endif

int main(int argc, char const*argv[]) {
  std::ifstream Stream(argv[1]);
  std::string Buffer{std::istreambuf_iterator<char>(Stream),
                     std::istreambuf_iterator<char>()};

  auto start = std::chrono::steady_clock::now();
  std::vector<unsigned> LineOffsets;
  LineOffsets.push_back(0);

  const unsigned char *Buf = (const unsigned char *)Buffer.data();
  const std::size_t BufLen = Buffer.size();
  unsigned I = 0;
  constexpr char NewLineBound = std::max('\r', '\n');
  auto* End = Buf + BufLen;

  unsigned Offs = 0;
  while (true) {
    // Skip over the contents of the line.
    const unsigned char *NextBuf = (const unsigned char *)Buf;

#ifdef __SSE2__
    // Try to skip to the next newline using SSE instructions. This is very
    // performance sensitive for programs with lots of diagnostics and in -E
    // mode.
    __m128i CRs = _mm_set1_epi8('\r');
    __m128i LFs = _mm_set1_epi8('\n');

    // First fix up the alignment to 16 bytes.
    while (((uintptr_t)NextBuf & 0xF) != 0) {
      if (*NextBuf == '\n' || *NextBuf == '\r' || *NextBuf == '\0')
        goto FoundSpecialChar;
      ++NextBuf;
    }

    // Scan 16 byte chunks for '\r' and '\n'. Ignore '\0'.
    while (NextBuf+16 <= End) {
      const __m128i Chunk = *(const __m128i*)NextBuf;
      __m128i Cmp = _mm_or_si128(_mm_cmpeq_epi8(Chunk, CRs),
                                 _mm_cmpeq_epi8(Chunk, LFs));
      unsigned Mask = _mm_movemask_epi8(Cmp);

      // If we found a newline, adjust the pointer and jump to the handling code.
      if (Mask != 0) {
        NextBuf += __builtin_ctz(Mask);
        goto FoundSpecialChar;
      }
      NextBuf += 16;
    }
#endif

    while (*NextBuf != '\n' && *NextBuf != '\r' && *NextBuf != '\0')
      ++NextBuf;

#ifdef __SSE2__
FoundSpecialChar:
#endif
    Offs += NextBuf-Buf;
    Buf = NextBuf;

    if (Buf[0] == '\n' || Buf[0] == '\r') {
      // If this is \n\r or \r\n, skip both characters.
      if ((Buf[1] == '\n' || Buf[1] == '\r') && Buf[0] != Buf[1]) {
        ++Offs;
        ++Buf;
      }
      ++Offs;
      ++Buf;
      LineOffsets.push_back(Offs);
    } else {
      // Otherwise, this is a null.  If end of file, exit.
      if (Buf == End) break;
      // Otherwise, skip the null.
      ++Offs;
      ++Buf;
   }
  }


  auto stop = std::chrono::steady_clock::now();
  std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms\n";
  for(auto LO : LineOffsets)
    std::cout << LO << "\n";
  return 0;
}
