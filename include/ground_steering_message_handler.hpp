#pragma once

#include <mutex>

#include "config.hpp"
#include "opendlv-standard-message-set.hpp"

#include "message_handler.hpp"

class GroundSteeringMessageHandler : public MessageHandler {
public:
  GroundSteeringMessageHandler();

  void setup(cluon::OD4Session &od4) override;
  float get_actual_steering_angle(); // expose getter for the steering
private:
  std::mutex m_handler_mutex;
  opendlv::proxy::GroundSteeringRequest m_ground_steering_request;
};
