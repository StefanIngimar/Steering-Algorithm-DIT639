#include "processing_status_message_handler.hpp"


ProcessingStatusMessageHandler::ProcessingStatusMessageHandler()
    : m_handler_mutex(), m_msg() {}


void ProcessingStatusMessageHandler::setup(cluon::OD4Session &od4) {
  auto processing_status_request = [this](cluon::data::Envelope &&env) {
    std::lock_guard<std::mutex> lock(m_handler_mutex);
    m_msg = cluon::extractMessage<opendlv::sim::ProcessingStatus>(std::move(env));
  };
  od4.dataTrigger(opendlv::sim::ProcessingStatus::ID(), processing_status_request);
}

ProcessStatus ProcessingStatusMessageHandler::get_status() {
    std::lock_guard<std::mutex> lock(m_handler_mutex);
    const std::string process_status = m_msg.status();
    if (process_status == "done") return ProcessStatus::DONE;
    return ProcessStatus::UNKNOWN;
}

