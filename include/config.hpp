#pragma once

#include "cluon-complete.hpp"

struct Config {
  int32_t cid;
  uint32_t width;
  uint32_t height;
  std::string shared_memory_name;
  bool is_verbose;
  bool should_generate_plot;
  bool should_analyze;

  static bool are_arguments_valid(
      const std::map<std::string, std::string>& args) {
    if (args.count("cid") == 0 || args.count("name") == 0 ||
        args.count("width") == 0 || args.count("height") == 0) {
      return false;
    }

    try {
      int cid = std::stoi(args.at("cid"));
      if (cid <= 0) {
        return false;
      }

      int width = std::stoi(args.at("width"));
      if (width <= 0) {
        return false;
      }

      int height = std::stoi(args.at("height"));
      if (height <= 0) {
        return false;
      }

      if (args.at("name").empty()) {
        return false;
      }

      return true;
    } catch (const std::invalid_argument&) {
      return false;
    } catch (const std::out_of_range&) {
      return false;
    }
  }

  static Config parse_config(int argc, char** argv) {
    auto args = cluon::getCommandlineArguments(argc, argv);
    if (!are_arguments_valid(args)) {
      throw std::invalid_argument("Usage: " + std::string(argv[0]) +
                                  " --cid=<OD4 session> --name=<shared memory "
                                  "name> --width=<frame width> "
                                  "--height=<frame height> [--verbose]\n"
                                  "Example: " +
                                  std::string(argv[0]) +
                                  " --cid=253 --name=img --width=640 "
                                  "--height=480 --verbose --generate_plot");
    }

    Config config;
    config.cid = std::stoi(args["cid"]);
    config.shared_memory_name = args["name"];
    config.width = static_cast<uint32_t>(std::stoi(args["width"]));
    config.height = static_cast<uint32_t>(std::stoi(args["height"]));
    config.is_verbose = args.count("verbose") != 0;
    config.should_generate_plot = args.count("generate_plot") != 0;
    config.should_analyze = args.count("analyze") != 0;

    return config;
  };
};
