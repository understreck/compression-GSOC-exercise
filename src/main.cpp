#include <zlib.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
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
parse_arg(std::string const &arg);

struct Options {
  OPTION inChannel = STDIO;
  std::string inData;
  OPTION outChannel = STDIO;
  std::string outData;
  OPTION direction = COMPRESS;
  int level = Z_DEFAULT_COMPRESSION;
};

void print_help(std::string const &arg);

Options parse_options(std::vector<std::string> const &args);

std::size_t file_size(std::ifstream &file);

std::vector<Byte> read_input(Options const &options);

void write_output(Options const &options, std::size_t ogSize,
                  std::vector<Byte> const &data);

int main(int argc, char *argv[]) {
  auto args = std::vector<std::string>{};

  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  auto const options = parse_options(args);

  auto in = read_input(options);

  auto out = std::vector<Byte>{};

  if (options.direction == COMPRESS) {
    my_compress(in, out, options.level);
  } else {
    my_decompress(in, out, in.size() * 4);
  }

  write_output(options, in.size(), out);
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

  auto result = (COMPRESSION_RESULT)(uncompress2(outBuffer.data(), &buffSize,
                                                 data.data(), &compressedSize));

  while (result == COMPRESSION_RESULT::BUFFER_OOM) {
    outBuffer.clear();
    buffSize *= 4;
    compressedSize = data.size();
    outBuffer.resize(buffSize);

    result = (COMPRESSION_RESULT)(uncompress2(outBuffer.data(), &buffSize,
                                              data.data(), &compressedSize));
  }

  outBuffer.resize(buffSize);

  return {result, compressedSize};
}

OPTION
parse_arg(std::string const &arg) {
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

void print_help(std::string const &arg) {
  printf("HELP TEXT: {%s}\n", arg.data());
}

Options parse_options(std::vector<std::string> const &args) {
  auto option = Options{};

  // TODO: proper range checking
  for (auto i = 0ul; i < args.size(); ++i) {
    switch (parse_arg(args[i])) {
    case INPUT: {
      switch (parse_arg(args[++i])) {
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
        print_help(args[i]);
        exit(1);
      }
      }
    } break;

    case OUTPUT: {
      switch (parse_arg(args[++i])) {
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
        print_help(args[i]);
        exit(1);
      }
      }
    } break;

    case DIRECTION: {
      option.direction = parse_arg(args[++i]);
    } break;

    case LEVEL: {
      option.level = std::stoi(args[++i]);
    } break;

    default: {
      print_help(args[i]);
      exit(1);
    }
    }
  }

  return option;
}

std::size_t file_size(std::ifstream &file) {
  auto const currentPos = file.tellg();

  file.seekg(0, std::ios_base::end);
  auto const size = file.tellg();

  file.seekg(currentPos);

  return size;
}

std::vector<Byte> read_input(Options const &options) {
  auto buffer = std::vector<Byte>{};
  buffer.reserve(65'536); // 2^16

  switch (options.inChannel) {
  case STDIO: {
    while (auto c = std::getchar()) {
      buffer.push_back(c);
    }
  } break;
  case FILENAME: {
    auto file = std::ifstream{options.inData};
    buffer.resize(file_size(file));

    for (auto i = 0ul; i < buffer.size(); ++i) {
      buffer[i] = file.get();
    }
    file.close();
  } break;
  case STRING: {
    buffer.resize(options.inData.size());
    std::copy(options.inData.begin(), options.inData.end(), buffer.begin());
  } break;

  // TODO: Add error message
  default: {
    exit(1);
  }
  }

  return buffer;
}

void write_output(Options const &options, std::size_t ogSize,
                  std::vector<Byte> const &data) {
  switch (options.outChannel) {
  case STDIO: {
    printf("Input was %lu bytes, output is %lu bytes.\n"
           "Output is %4.1lf%% the size of input.\n"
           "Resulting data:\n%*s",
           ogSize, data.size(), 100 * (double)data.size() / ogSize,
           (int)data.size(), data.data());
  } break;
  case FILENAME: {
    printf("Input was %lu bytes, output is %lu bytes.\n"
           "Output is %4.1lf%% the size of input.\n",
           ogSize, data.size(), 100 * (double)data.size() / ogSize);
    auto outFile = std::ofstream{options.outData};
    outFile.write(reinterpret_cast<char const *>(data.data()), data.size());
    outFile.close();
  } break;
  default: {
    exit(1);
  }
  }
}
