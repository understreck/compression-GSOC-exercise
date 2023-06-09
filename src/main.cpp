#include <zlib.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <fstream>
#include <vector>

enum COMPRESSION_RESULT : int {
  OK = Z_OK,                    // OK, no errors
  OOM = Z_MEM_ERROR,            // Out of system memory
  BUFFER_OOM = Z_BUF_ERROR,     // Output buffer out of memory
  INVALID_INPUT = Z_DATA_ERROR, // Invalid or corrupted input
  INVALID_LEVEL                 // Invalid compression level
};

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

Options parse_options(std::vector<std::string> const &args);

std::size_t file_size(std::ifstream &file);

std::vector<Byte> read_input(Options const &options);

void write_output(Options const &options, std::size_t ogSize,
                  std::vector<Byte> const &data);

void print_help();

int my_compress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
                int level = Z_DEFAULT_COMPRESSION);

int my_decompress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
                  std::size_t buffSize);

int main(int argc, char *argv[]) {
  if(argc == 1) {
    print_help();
    return 0;
  }
    
  auto args = std::vector<std::string>{};

  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  auto const options = parse_options(args);

  auto in = read_input(options);

  auto out = std::vector<Byte>{};

  auto now = std::chrono::system_clock::now();
  if (options.direction == COMPRESS) {
    auto result = my_compress(in, out, options.level);

    if (result != COMPRESSION_RESULT::OK) {
      printf("Compression failed.\n");
      return 1;
    }
  } else {
    auto result = my_decompress(in, out, in.size() * 4);

    if (result != COMPRESSION_RESULT::OK) {
      printf("Decompression failed.\n");
      return 1;
    }
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

  printf("It took %ld minutes %ld.%03ld seconds.\n", minutes, seconds,
         milliseconds);

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

int my_decompress(std::vector<Byte> const &data, std::vector<Byte> &outBuffer,
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

  return result;
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

void print_help() {
  printf(
      "compression-exercise   [ [-i | --input]   [file <path> | stdio | string <string>] ]\n"
      "                       [ [-o | --output]  [file <path> | stdio] ]\n"
      "                       [ [-l | --level] <0-9>]\n"
      "                       [ [-d | --direction]   [compress | decompress] ]\n"
      "                       [-h | --help | help]\n\n"

      "-i, --input:\n"
      "  Set input channel. Default is stdio. Examples:\n"
      "    compression-exercise -i file ./file/path.txt\n"
      "    echo \"Compress this\\0\" | compression-exercise -i stdio\n"
      "    compression-exercise -i string \"Compress this\"\n"
      "-o, --output:\n"
      "  Set output channel. Default is stdio. Examples:\n"
      "    echo \"Compress this\\0\" | compression-exercise -o file "
      "./file/path.txt\n"
      "    echo \"Compress this\\0\" | compression-exercise -o stdio\n"
      "-l, --level:\n"
      "  Set compression level. 0 is none, 1 is fastest, 9 is best. Default is 6.\n"
      "  Is ignored when decompressing. Example:\n"
      "    compression-exercise -i string \"Compress this\" -l 9\n"
      "-d, --direction:\n"
      "  Set if program should compress or decompress input. Example:\n"
      "    compression-exercise -i file compressed-input.zlib -d decompress\n");
}

Options parse_options(std::vector<std::string> const &args) {
  auto option = Options{};

  for (auto i = 0ul; i < args.size(); ++i) {
    switch (parse_arg(args[i])) {
    case INPUT: {
      if ((i + 1) >= args.size()) {
        print_help();
        exit(1);
      }

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
        print_help();
        exit(1);
      }
      }
    } break;

    case OUTPUT: {
      if ((i + 1) >= args.size()) {
        print_help();
        exit(1);
      }

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
        print_help();
        exit(1);
      }
      }
    } break;

    case DIRECTION: {
      if ((i + 1) >= args.size()) {
        print_help();
        exit(1);
      }

      option.direction = parse_arg(args[++i]);
    } break;

    case LEVEL: {
      if ((i + 1) >= args.size()) {
        print_help();
        exit(1);
      }

      try {
        option.level = std::stoi(args[++i]);
      } catch (std::logic_error const &) {
        print_help();
        exit(1);
      }
    } break;

    default: {
      print_help();
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
    if (not file.is_open()) {
      printf("Could not open file \"%s\".\n", options.inData.data());
    }
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
    if (not outFile.is_open()) {
      printf("Could not open file \"%s\".\n", options.outData.data());
    }
    outFile.write(reinterpret_cast<char const *>(data.data()), data.size());
    outFile.close();
  } break;
  default: {
    exit(1);
  }
  }
}
