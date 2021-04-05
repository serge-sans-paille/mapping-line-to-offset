#include <chrono>
#include <vector>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include <cstring>

template <class T>
static constexpr inline T haslessp(T x, unsigned char n) {
  return ((x)-~0UL/255*(n))
    ;
}
template <class T>
static constexpr inline T hasless(T x, unsigned char n) {
  return (((x)-~0UL/255*(n))&~(x)&~0UL/255*128)
    ;
}
template <class T>
static constexpr inline T haszero(T x) {
  return  hasless(x, 1);
}
template <class T>
static constexpr inline T hasvalue(T x, unsigned char n) {
  // see http://graphics.stanford.edu/~seander/bithacks.html#HasBetweenInWord
  return (haszero((x) ^ (~0UL/255 * (n))));
}
template <class T>
static constexpr inline T hasvalues(T x, unsigned char n, unsigned char m) {
  return (haslessp(x ^ (~0UL/255 * n), 1) | haslessp(x ^ (~0UL/255 * m), 1)) &~(x)&~0UL/255*128;
}


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

  uint64_t Word;

  // scan sizeof(Word) bytes at a time for new lines.
  // This is much faster than scanning each byte independently.
  if (BufLen > sizeof(Word)) {
    do {
      memcpy(&Word, Buf + I, sizeof(Word));
      if(uint64_t Mask = hasvalue(Word, '\r') | hasvalue(Word, '\n'))
      {
        unsigned N = __builtin_ctzl(Mask) - 7;
        I += N / 8 + 1;
        unsigned char Byte = Word >> N;
        I += ((Byte == '\r') && (Buf[I] == '\n'))? 1 : 0;
        LineOffsets.push_back(I);
      }
      else
        I += sizeof(Word);
    }
    while (I < BufLen - sizeof(Word));
  }

  // Handle tail using a regular check.
  while (I < BufLen) {
    if (Buf[I] == '\n') {
      LineOffsets.push_back(I + 1);
    } else if (Buf[I] == '\r') {
      // If this is \r\n, skip both characters.
      if (I + 1 < BufLen && Buf[I + 1] == '\n')
        ++I;
      LineOffsets.push_back(I + 1);
    }
    ++I;
  }

  auto stop = std::chrono::steady_clock::now();
  std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms\n";
  for(auto LO : LineOffsets)
    std::cout << LO << "\n";
  return 0;
}
