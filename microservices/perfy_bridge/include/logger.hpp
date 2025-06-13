#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/*
 *
 * Logger based on spdlog.
 *
 * Could technically write a logger wrapper that would decouple the interface from implementation
 * and make it easy to change the logger in the future --- an overkill for now though
 * */
class Logger {
public:
    static Logger& get_instance() {
        static Logger instance;
        return instance;
    }

    std::shared_ptr<spdlog::logger> get_logger() const {
        return m_logger;
    }

private:
    Logger() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        auto logger = std::make_shared<spdlog::logger>("nutmeg", console_sink);
        logger->set_level(spdlog::level::debug);
        spdlog::register_logger(logger);

        logger->set_pattern("[BRIDGE] [%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        m_logger = logger;
    }

    std::shared_ptr<spdlog::logger> m_logger;
};
