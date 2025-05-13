#pragma once

#include "cluon-complete.hpp"

struct Config {
    int cid;
    int width;
    int height;
    std::string shared_memory_name;
    bool is_verbose;

    static bool are_arguments_valid(std::map<std::string, std::string>& args) {
        return args.count("cid") == 0 || args.count("name") == 0 || args.count("width") || args.count("height") == 0;
    }

    static Config parse_config(int argc, char** argv) {
        auto args = cluon::getCommandlineArguments(argc, argv);
        if (!are_arguments_valid(args)) {
            throw std::invalid_argument(
                "Usage: " + std::string(argv[0]) + " --cid=<OD4 session> --name=<shared memory name> --width=<frame width> " "--height=<frame height> [--verbose]\n"
                "Example: " + std::string(argv[0]) + " --cid=253 --name=img --width=640 --height=480 --verbose"
            );
        }
        
        Config config;
        config.cid = static_cast<int>(std::stoi(args["cid"]));
        config.shared_memory_name = args["name"];
        config.width = static_cast<uint32_t>(std::stoi(args["width"]));
        config.height = static_cast<uint32_t>(std::stoi(args["height"]));
        config.is_verbose = args.count("verbose") != 0;

        return config;
    };
};

