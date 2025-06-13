#include <cstdint>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/shm.h>

#include <cluon-complete.hpp>
#include <opendlv-standard-message-set.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "logger.hpp"

enum HeaderParseStatus {
    TERMINATION_SIGNAL = 0,
    FRAME_NOT_AVAILABLE = 1,
    FRAME_TOO_BIG = 2,
    OK = 3,
};

struct ParsedHeader {
    HeaderParseStatus status;
    uint64_t frame_ts;
    double latest_steering;
    std::vector<uint8_t> frame_data;
};

uint32_t read_uint32(const uint8_t* data) {
    return (uint32_t(data[0]) << 24) | (uint32_t(data[1]) << 16) |
           (uint32_t(data[2]) << 8) | uint32_t(data[3]);
}

uint64_t read_uint64(const uint8_t* data) {
    return (uint64_t(data[0]) << 56) | (uint64_t(data[1]) << 48) |
           (uint64_t(data[2]) << 40) | (uint64_t(data[3]) << 32) |
           (uint64_t(data[4]) << 24) | (uint64_t(data[5]) << 16) |
           (uint64_t(data[6]) << 8)  | uint64_t(data[7]);
}

double read_double(const uint8_t* data) {
    uint64_t as_int = read_uint64(data);
    double result;
    std::memcpy(&result, &as_int, sizeof(result));
    return result;
}

ParsedHeader parse_header(
    uint8_t* data,
    const size_t max_frame_size
) {
    auto logger = Logger::get_instance().get_logger();

    uint8_t is_frame_available = data[0];
    uint8_t should_terminate = data[1];

    if (is_frame_available == 0 && should_terminate == 1) {
        logger->info("Termination signal received from python. Exiting the main loop.");
        return ParsedHeader{
            TERMINATION_SIGNAL,
            0,
            0.0,
            {},
        };
    }
    if (is_frame_available != 1) {
        usleep(1000);
        return ParsedHeader{
            FRAME_NOT_AVAILABLE,
            0,
            0.0,
            {},
        };
    }

    const size_t offset_encoded_size = 2;
    const size_t offset_timestamp = 6;
    const size_t offset_steering = 14;
    const size_t header_size = 22;

    uint32_t content_length = read_uint32(data + offset_encoded_size);
    if (content_length > max_frame_size) {
        logger->error("Received frame is too big: {} bytes", content_length);
        data[0] = 0;
        return ParsedHeader{
            FRAME_TOO_BIG,
            0,
            0.0,
            {},
        };
    }

    uint64_t frame_ts = read_uint64(data + offset_timestamp);
    double latest_steering = read_double(data + offset_steering);

    std::vector<uint8_t> frame_data(content_length);
    std::memcpy(frame_data.data(), data + header_size, content_length);

    data[0] = 0;

    return ParsedHeader{
        OK,
        frame_ts,
        latest_steering,
        std::move(frame_data)
    };
}

int main(){
    auto logger = Logger::get_instance().get_logger();

    // those values have to match values used by the python producer
    const key_t SHM_KEY = 0x696d67;
    const int HEADER_SIZE = 22;
    const int MAX_H264_FRAME_SIZE = 200 * 1024;
    const int SHARED_MEMORY_SIZE = HEADER_SIZE + MAX_H264_FRAME_SIZE;

    std::stringstream ss;
    ss << std::hex << SHM_KEY;

    // Python shared memory setup
    int shm_id = shmget(SHM_KEY, SHARED_MEMORY_SIZE, 0666);
    if (shm_id == -1) {
        logger->error("Failed to get shared memory with key 0x{}: {}", ss.str(), std::strerror(errno));
        return 1;
    }

    void* shm_ptr= shmat(shm_id, nullptr, 0);
    if (shm_ptr == (void*)-1) {
        logger->error("Failed to attach to shared memory: {}", std::strerror(errno));
        return 1;
    }
    logger->info("Attached to shared memory with key 0x{}", ss.str());

    // Cluon shared memory setup
    const int cluon_cid = 253;
    cluon::OD4Session od4(cluon_cid, [](cluon::data::Envelope &&envelope) noexcept {});
    if (!od4.isRunning()) {
        logger->error("Failed to start cluon OD4 session with CID '{}'", cluon_cid);
        shmdt(shm_ptr);
        return 1;
    }
    logger->info("Started cluon OD4 session with CID '{}'", cluon_cid);

    uint32_t frame_count = 0;
    ParsedHeader result;
    while (true) {
        uint8_t* data = reinterpret_cast<uint8_t*>(shm_ptr);

        result = parse_header(data, MAX_H264_FRAME_SIZE);

        if (result.status == TERMINATION_SIGNAL) {
            break;
        } else if (result.status == FRAME_NOT_AVAILABLE || result.status == FRAME_TOO_BIG) {
            continue;
        }

        cluon::data::TimeStamp cluon_ts;
        cluon_ts.seconds(static_cast<int32_t>(result.frame_ts / 1000000));
        cluon_ts.microseconds(static_cast<int32_t>(result.frame_ts % 1000000));

        opendlv::proxy::ImageReading msg;
        msg.data(std::string(result.frame_data.begin(), result.frame_data.end())).fourcc("h264").width(640).height(480);
        od4.send(msg, cluon_ts, 0);

        opendlv::proxy::GroundSteeringRequest ground_steering_req_msg;
        ground_steering_req_msg.groundSteering(static_cast<float>(result.latest_steering));
        od4.send(ground_steering_req_msg, cluon_ts, 0);

        frame_count += 1;
        if (frame_count % 100 == 0) {
            logger->info("Received {} frames", frame_count);
        }
    }
    logger->info("Done. Received {} frames", frame_count);

    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    int64_t now_us = duration.count();

    cluon::data::TimeStamp cluon_ts;
    cluon_ts.seconds(static_cast<int32_t>(now_us / 1000000));
    cluon_ts.microseconds(static_cast<int32_t>(now_us % 1000000));

    opendlv::sim::ProcessingStatus status_msg;
    status_msg.status("done");
    od4.send(status_msg, cluon_ts, 0);
    
    // NOTE(sw): that is basically a hack... not super proud... after the 'done' processing status is sent, we still need
    // to send one more frame to nutmeg. This prevents the situation when the shared memory is waiting indefinitely - since all frames were
    // already sent there will be no more data coming to the shared memory creating an infinite loop.
    opendlv::proxy::ImageReading msg;
    msg.data(std::string(result.frame_data.begin(), result.frame_data.end())).fourcc("h264").width(640).height(480);
    od4.send(msg, cluon_ts, 0);

    logger->info("Detaching from shared memory with key {}", ss.str());
    shmdt(shm_ptr);

    return 0;
}
