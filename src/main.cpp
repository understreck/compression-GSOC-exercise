#include <zlib.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <fstream>
#include <vector>

int my_compress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
                int level);

std::size_t file_size(std::ifstream &file);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Wrong number of arguments.\n");
    return 1;
  }

  auto inBuffer = std::vector<Byte>{};
  inBuffer.reserve(65'536); // 2^16

  auto inFile = std::ifstream{argv[1]};
  if (not inFile.is_open()) {
    printf("Could not open file \"%s\".\n", argv[1]);
  }
  inBuffer.resize(file_size(inFile));

  for (auto i = 0ul; i < inBuffer.size(); ++i) {
    inBuffer[i] = inFile.get();
  }
  inFile.close();

  auto outBuffer = std::vector<Byte>{};

  auto const now = std::chrono::system_clock::now();
  if (my_compress(inBuffer, outBuffer, 6) != Z_OK) {
    printf("Compression failed.\n");
    return 1;
  }
  auto const then = std::chrono::system_clock::now();

  auto const minutes =
      std::chrono::duration_cast<std::chrono::minutes>(then - now).count();

  auto const seconds =
      std::chrono::duration_cast<std::chrono::seconds>(then - now).count() -
      60 * minutes;

  auto const milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(then - now)
          .count() -
      1000 * seconds;

  auto outFile = std::ofstream{argv[2]};
  if (not outFile.is_open()) {
    printf("Could not open file \"%s\".\n", argv[2]);
  }
  outFile.write(reinterpret_cast<char const *>(outBuffer.data()),
                outBuffer.size());
  outFile.close();

  printf("Input was %lu bytes, output is %lu bytes.\n"
         "Output is %4.1lf%% the size of input.\n"
         "It took %ld minutes %ld.%03ld seconds\n",
         inBuffer.size(), outBuffer.size(),
         100 * (double)outBuffer.size() / inBuffer.size(), minutes, seconds,
         milliseconds);
}

int my_compress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
                int level) {
  auto const uncompressedSize = data.size();
  auto compressedSize = compressBound(uncompressedSize);
  outBuffer.resize(compressedSize);

  auto const result = compress2(outBuffer.data(), &compressedSize, data.data(),
                                uncompressedSize, level);

  outBuffer.resize(compressedSize);

  return result;
}

std::size_t file_size(std::ifstream &file) {
  auto const currentPos = file.tellg();

  file.seekg(0, std::ios_base::end);
  auto const size = file.tellg();

  file.seekg(currentPos);

  return size;
}
