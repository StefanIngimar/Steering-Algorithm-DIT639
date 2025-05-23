#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(const uint8_t *buf, size_t bufLen) {
  std::string ret;
  int i = 0;
  uint32_t octet_a, octet_b, octet_c, triple;

  while (i < static_cast<int>(bufLen)) {
    octet_a = buf[i++];
    octet_b = (i < static_cast<int>(bufLen)) ? buf[i++] : 0;
    octet_c = (i < static_cast<int>(bufLen)) ? buf[i++] : 0;

    triple = (octet_a << 16) | (octet_b << 8) | octet_c;

    ret.push_back(base64_chars[(triple >> 18) & 0x3F]);
    ret.push_back(base64_chars[(triple >> 12) & 0x3F]);
    ret.push_back((i > static_cast<int>(bufLen) + 1)
                      ? '='
                      : base64_chars[(triple >> 6) & 0x3F]);
    ret.push_back((i > static_cast<int>(bufLen)) ? '='
                                                 : base64_chars[triple & 0x3F]);
  }

  return ret;
}

int32_t main(int32_t argc, char **argv) {
  auto args = cluon::getCommandlineArguments(argc, argv);
  if (args.count("rec") == 0 || args.count("output") == 0) {
    std::cerr << "Usage: " << argv[0]
              << " --rec=<file.rec> --output=<file.txt>\n";
    return 1;
  }

  std::ofstream fout(args["output"]);
  if (!fout.is_open()) {
    std::cerr << "Could not open output file: " << args["output"] << "\n";
    return 1;
  }

  cluon::Player player(args["rec"], false, false);
  size_t imageCount = 0;
  size_t totalMessages = 0;

  while (player.hasMoreData()) {
    auto [ok, env] = player.getNextEnvelopeToBeReplayed();
    if (!ok) continue;
    totalMessages++;

    if (env.dataType() == opendlv::proxy::ImageReading::ID()) {
      auto img =
          cluon::extractMessage<opendlv::proxy::ImageReading>(std::move(env));
      const std::string &raw = img.data();

      std::string encoded = base64_encode(
          reinterpret_cast<const uint8_t *>(raw.data()), raw.size());

      fout << env.sampleTimeStamp().seconds() << "." << std::setfill('0')
           << std::setw(6) << env.sampleTimeStamp().microseconds()
           << " TYPE=" << env.dataType() << " SENDER=" << env.senderStamp()
           << " WIDTH=" << img.width() << " HEIGHT=" << img.height()
           << " data: (base64) " << encoded << "\n";

      std::cerr << "Extracted ImageReading #" << imageCount << " ["
                << img.width() << "x" << img.height() << ", " << raw.size()
                << " bytes]\n";

      imageCount++;
    }
  }

  fout.close();
  std::cerr << "Total messages parsed: " << totalMessages << "\n";
  std::cerr << "ImageReading messages extracted: " << imageCount << "\n";

  if (imageCount == 0) {
    std::cerr << "Warning: No ImageReading messages found in this .rec file.\n";
  }

  return 0;
}
