#include <zlib.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <vector>

enum COMPRESSION_RESULT : int {
  OK = Z_OK,                    // OK, no errors
  OOM = Z_MEM_ERROR,            // Out of system memory
  BUFFER_OOM = Z_BUF_ERROR,     // Output buffer out of memory
  INVALID_INPUT = Z_DATA_ERROR, // Invalid or corrupted input
  INVALID_LEVEL                 // Invalid compression level
};

int my_compress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
                int level = Z_DEFAULT_COMPRESSION);

struct DecompressResult {
  COMPRESSION_RESULT compRes;
  std::size_t bytesRead;
};

DecompressResult my_decompress(std::vector<Byte> const &data,
                               std::vector<Byte> &outBuffer,
                               std::size_t buffSize);

enum OPTION {
  INVALID,
  HELP,
  INPUT,
  OUTPUT,
  FILENAME,
  STDIO,
  STRING,
  DIRECTION,
  COMPRESS,
  DECOMPRESS,
  LEVEL,
};

OPTION
parseArg(std::string const &arg);

struct Options {
  OPTION inChannel = STDIO;
  std::string inData;
  OPTION outChannel = STDIO;
  std::string outData;
  OPTION direction = COMPRESS;
  int level = Z_DEFAULT_COMPRESSION;
} option;

void print_help();

int main(int argc, char *argv[]) {
  auto args = std::vector<std::string>{};

  for (int i = 0; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  // TODO: proper range checking
  for (int i = 0; i < argc; ++i) {
    switch (parseArg(args[i])) {
    case INPUT: {
      switch (parseArg(args[i++])) {
      case STDIO: {
        option.inChannel = STDIO;
      } break;
      case FILENAME: {
        option.inChannel = FILENAME;
        option.inData = args[++i];
      } break;
      case STRING: {
        option.inChannel = STRING;
        option.inData = args[++i];
      } break;
      default: {
        print_help();
        exit(1);
      }
      }
    } break;
    case OUTPUT: {
      switch (parseArg(args[i++])) {
      case STDIO: {
        option.outChannel = STDIO;
      } break;
      case FILENAME: {
        option.outChannel = FILENAME;
        option.outData = args[++i];
      } break;
      case STRING: {
        option.outChannel = STRING;
        option.outData = args[++i];
      } break;
      case HELP:
        [[fallthrough]];
      default: {
        print_help();
        exit(1);
      }
      }
    } break;
    case DIRECTION: {
      option.level = std::stoi(args[++i]);
    } break;
    default: {
      print_help();
      exit(1);
    }
    }
  }

  auto in = std::vector<Byte>{};
  in.reserve(65'536); // 2^16

  // auto in = std::ifstream{args[0]};

  // in.seekg(0, std::ios_base::end);
  // auto const inSize = in.tellg();
  // in.seekg(0);

  while (auto c = std::getchar()) {
    in.push_back(c);
  }

  auto out = std::vector<Byte>{};

  /*  auto compResult = */ my_compress(in, out, Z_DEFAULT_COMPRESSION);

  printf("Uncompressed size: %ld\nCompressed size: %ld\n\n", in.size(),
         out.size());

  my_decompress(out, in, in.size());
  printf("\nUncompressed size: %ld\nCompressed size: %ld\n\n", in.size(),
         out.size());

  for (auto c : in) {
    putchar(c);
  }
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

OPTION
parseArg(std::string const &arg) {
  if (arg == "--input" || arg == "-i")
    return INPUT;
  if (arg == "--output" || arg == "-o")
    return OUTPUT;
  if (arg == "file")
    return FILENAME;
  if (arg == "stdio")
    return STDIO;
  if (arg == "string")
    return STRING;
  if (arg == "--direction" || arg == "-d")
    return DIRECTION;
  if (arg == "compress")
    return COMPRESS;
  if (arg == "decompress")
    return DECOMPRESS;
  if (arg == "--level" || arg == "-l")
    return LEVEL;
  if (arg == "help" || arg == "--help" || arg == "-h")
    return HELP;

  return INVALID;
}

void print_help() {}
