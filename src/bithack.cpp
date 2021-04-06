#include <chrono>
#include <vector>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include <cstring>

template <class T>
static constexpr inline T likelyhasbetween(T x, unsigned char m,
                                              unsigned char n) {
  // see http://graphics.stanford.edu/~seander/bithacks.html#HasBetweenInWord
  return (((x) - ~0UL / 255 * (n)) & ~(x) &
          ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (m))) &
         ~0UL / 255 * 128;
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
  constexpr char NewLineBound = std::max('\r', '\n');

  uint64_t Word;

  // scan sizeof(Word) bytes at a time for new lines.
  // This is much faster than scanning each byte independently.
  if (BufLen > 1 + sizeof(Word)) {
    do {
      memcpy(&Word, Buf + I, sizeof(Word));
      // no new line => jump over sizeof(Word) bytes.
      auto Mask = likelyhasbetween(Word, '\n' - 1, '\r'+1 );
      if (!Mask) {
        I += sizeof(Word);
        continue;
      }

      // Otherwise scan for the next newline - it's very likely there's one.
      // Note that according to
      // http://graphics.stanford.edu/~seander/bithacks.html#HasBetweenInWord,
      // likelyhasbetween may have false positive for the upper bound.
      unsigned N = __builtin_ctzl(Mask) - 7;
      Word >>= N;
      I += N / 8 + 1;
      unsigned char Byte = Word;
      if (Byte == '\n') {
        LineOffsets.push_back(I);
      } else if (Byte == '\r') {
        // If this is \r\n, skip both characters.
        if (Buf[I] == '\n')
          ++I;
        LineOffsets.push_back(I);
      }
    }
    while (I < BufLen - sizeof(Word) - 1);
  }

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
