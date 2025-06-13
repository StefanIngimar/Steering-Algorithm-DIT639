#pragma once

#include <mutex>

#include "config.hpp"
#include "opendlv-standard-message-set.hpp"

#include "message_handler.hpp"

enum ProcessStatus {
    UNKNOWN = 0,
    DONE = 1,
};

class ProcessingStatusMessageHandler : public MessageHandler {
public:
  ProcessingStatusMessageHandler();

  void setup(cluon::OD4Session &od4) override;
  ProcessStatus get_status();
private:
  std::mutex m_handler_mutex;
  opendlv::sim::ProcessingStatus m_msg; 
};
