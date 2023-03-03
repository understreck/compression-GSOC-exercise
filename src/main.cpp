#include <zlib.h>

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

int
main(int argc, char *argv[]) {
    auto args = std::vector<std::string>{};

    for(int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    auto in = std::ifstream{args[0]};

    in.seekg(0, std::ios_base::end);
    auto const inSize = in.tellg();
    in.seekg(0);

    auto outBuf = std::vector<char>{};
    outBuf.resize(compressBound(inSize));
}
