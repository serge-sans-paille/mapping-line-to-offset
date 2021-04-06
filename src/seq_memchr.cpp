#include <chrono>
#include <vector>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include <cstring>

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
  if(!memchr(Buf, '\r', BufLen)) {
    while (I < BufLen) {
        if (__builtin_expect(Buf[I] == '\n', 0)) {
          LineOffsets.push_back(I + 1);
        }
      ++I;
    }
  }
  else {
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
  }

  auto stop = std::chrono::steady_clock::now();
  std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;
  for(auto LO : LineOffsets)
    std::cout << LO << std::endl;
  return 0;
}
