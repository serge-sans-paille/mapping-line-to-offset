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

#ifdef __SSE2__

  const auto LFs = VBROADCAST('\n');
  const auto CRs = VBROADCAST('\r');

  while (I + sizeof(LFs) + 1 < BufLen) {
    auto Chunk1 = VLOAD(Buf + I);
    auto Cmp1 = VOR(VEQ(Chunk1, LFs), VEQ(Chunk1, CRs));
    unsigned Mask = VMOVEMASK(Cmp1) ;

    if(Mask) {
      unsigned N = __builtin_ctz(Mask);
      I += N;
      Mask >>= N;
      I += ((Buf[I] == '\r') && (Buf[I + 1] == '\n'))? 2 : 1;
      LineOffsets.push_back(I);
    }
    else
      I += sizeof(LFs);
  }
#endif

  // Handle tail using a regular check.
  while (I < BufLen) {
    // Use a fast check to catch both newlines
    if (__builtin_expect((Buf[I] - '\n') <= ('\r' - '\n'), 0)) {
      if (Buf[I] == '\n') {
        LineOffsets.push_back(I + 1);
      } else if (Buf[I] == '\r') {
        // If this is \r\n, skip both characters.
        if (I + 1 < BufLen && Buf[I + 1] == '\n')
          ++I;
        LineOffsets.push_back(I + 1);
      }
    }
    ++I;
  }

  auto stop = std::chrono::steady_clock::now();
  std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms\n";
  for(auto LO : LineOffsets)
    std::cout << LO << "\n";
  return 0;
}
