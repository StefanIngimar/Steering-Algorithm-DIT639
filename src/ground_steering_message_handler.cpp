#include "ground_steering_message_handler.hpp"

GroundSteeringMessageHandler::GroundSteeringMessageHandler()
    : m_handler_mutex(), m_ground_steering_request() {}

float GroundSteeringMessageHandler::get_actual_steering_angle() {
  std::lock_guard<std::mutex> lock(m_handler_mutex);
  return m_ground_steering_request.groundSteering();
}

void GroundSteeringMessageHandler::setup(cluon::OD4Session &od4) {
  auto on_ground_steering_request = [this](cluon::data::Envelope &&env) {
    std::lock_guard<std::mutex> lock(m_handler_mutex);
    m_ground_steering_request =
        cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(
            std::move(env));
  };
  od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(),
                  on_ground_steering_request);
}
