#include <algorithm>
#include <cstddef>
#include <iostream>
#include <zlib.h>

#include <cstdio>
#include <vector>

enum COMPRESSION_RESULT : int {
  OK = Z_OK,                    // OK, no errors
  OOM = Z_MEM_ERROR,            // Out of system memory
  BUFFER_OOM = Z_BUF_ERROR,     // Output buffer out of memory
  INVALID_INPUT = Z_DATA_ERROR, // Invalid or corrupted input
  INVALID_LEVEL                 // Invalid compression level
};

enum class COMPRESSION_LEVEL : int {
  NONE = Z_NO_COMPRESSION,
  FASTEST = 1,
  ONE = FASTEST,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  SIX = 6,
  SEVEN = 7,
  EIGHT = 8,
  NINE = 9,
  BEST = NINE,
  DEFAULT = Z_DEFAULT_COMPRESSION
};

COMPRESSION_RESULT
my_compress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
            COMPRESSION_LEVEL level = COMPRESSION_LEVEL::DEFAULT);

struct DecompressResult {
  COMPRESSION_RESULT compRes;
  std::size_t bytesRead;
};

DecompressResult my_decompress(std::vector<Byte> const &data,
                               std::vector<Byte> &outBuffer,
                               std::size_t maxBuffSize);

int main() {
  auto in = std::vector<Byte>{};
  in.reserve(65'536); // 2^16

  while (auto c = std::getchar()) {
    in.push_back(c);
  }

  auto out = std::vector<Byte>{};

  /*  auto compResult = */ my_compress(in, out, COMPRESSION_LEVEL::FASTEST);

  printf("Uncompressed size: %ld\nCompressed size: %ld\n\n", in.size(),
         out.size());

  my_decompress(out, in, in.size());
  printf("\nUncompressed size: %ld\nCompressed size: %ld\n\n", in.size(),
         out.size());

  for (auto c : in) {
    putchar(c);
  }
}

COMPRESSION_RESULT
my_compress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
            COMPRESSION_LEVEL level) {
  auto const uncompressedSize = data.size();
  auto compressedSize = compressBound(uncompressedSize);
  outBuffer.resize(compressedSize);

  auto const result =
      (COMPRESSION_RESULT)compress2(outBuffer.data(), &compressedSize,
                                    data.data(), uncompressedSize, (int)level);

  outBuffer.resize(compressedSize);

  return result;
}

DecompressResult my_decompress(std::vector<Byte> const &data,
                               std::vector<Byte> &outBuffer,
                               std::size_t buffSize) {
  auto compressedSize = data.size();
  outBuffer.resize(buffSize);

  auto const result = (COMPRESSION_RESULT)(uncompress2(
      outBuffer.data(), &buffSize, data.data(), &compressedSize));

  outBuffer.resize(buffSize);

  return {result, compressedSize};
}
