#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/shm.h>

#include <cluon-complete.hpp>
#include <opendlv-standard-message-set.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

/*
 * Display frame received from python producer.
 *
 * Return -1 when shit goes wrong, 0 otherwise.
 * */
int display_frame(uint32_t content_length, std::vector<uint8_t>& frame_data_buffer) {
    // NOTE: one of the ways to display the received frame is by saving
    // it to a file and passing that file to video capture. That solution
    // seems simpler than streaming or ffmpeg directly.
    //
    // Passing frame data directly to video capture does not work...
    std::string temp_file = "/tmp/frame.h264";
    std::ofstream out(temp_file, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to create temporary file " << temp_file << std::endl;
        return -1;
    }
    out.write(reinterpret_cast<char*>(frame_data_buffer.data()), content_length);
    out.close();

    cv::Mat frame;
    try {
        cv::VideoCapture decoder;
        if (!decoder.open(temp_file, cv::CAP_FFMPEG)) {
            std::cerr << "Failed to open decoder for temporary file " << temp_file << std::endl;
            return -1;
        }

        if (!decoder.read(frame)) {
            std::cerr << "Failed reading frame from decoder" << std::endl;
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error decoding frame: " << e.what() << std::endl;
        return -1;
    }

    if (!frame.empty()) {
        cv::imshow("pywindow", frame);
    } else {
        std::cerr << "Decoded frame is empty" << std::endl;
        return -1;
    }

    std::remove(temp_file.c_str());

    return 0;
}

int main(){
    // those values have to match values used by the python producer
    const key_t SHM_KEY = 0x696d67;
    const int HEADER_SIZE = 6;
    const int MAX_H264_FRAME_SIZE = 200 * 1024;
    const int SHARED_MEMORY_SIZE = HEADER_SIZE + MAX_H264_FRAME_SIZE;
    
    // Python shared memory setup
    int shm_id = shmget(SHM_KEY, SHARED_MEMORY_SIZE, 0666);
    if (shm_id == -1) {
        std::cerr << "Error: Failed to get shared memory with key 0x" << std::hex << SHM_KEY << ": " << std::strerror(errno) << std::endl;
        return 1;
    }

    void* shm_ptr= shmat(shm_id, nullptr, 0);
    if (shm_ptr == (void*)-1) {
        std::cerr << "Error: Failed to attach to shared memory: " << std::strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Attached to shared memory with key 0x" << std::hex << SHM_KEY << std::dec << std::endl;

    // Cluon shared memory setup 
    const int cluon_cid = 253;
    cluon::OD4Session od4(cluon_cid, [](cluon::data::Envelope &&envelope) noexcept {});
    if (!od4.isRunning()) {
        std::cerr << "Failed to start cluon OD4 session with CID " << cluon_cid << std::endl;
        shmdt(shm_ptr);
        return 1;
    }
    std::cout << "Started cluon OD4 session with CID " << cluon_cid << std::endl;

    // cv::namedWindow("pywindow", cv::WINDOW_AUTOSIZE);

    std::vector<uint8_t> frame_data_buffer(MAX_H264_FRAME_SIZE);
    uint32_t frame_count = 0;

    while (true) {
        uint8_t* data = reinterpret_cast<uint8_t*>(shm_ptr);
        
        uint8_t is_frame_available = data[0];
        uint8_t should_terminate = data[1];

        if (is_frame_available == 0 && should_terminate == 1) {
            std::cout << "Termination signal received from python. Exiting the main loop." << std::endl;
            break;
        } 
        if (is_frame_available != 1) {
            usleep(1000);
            continue;
        }
        
        // content length is 4 bytes (big-endian)
        uint32_t content_length = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];
        if (content_length > MAX_H264_FRAME_SIZE) {
            std::cerr << "Received frame is too big: " << content_length << " bytes" << std::endl;
            data[0] = 0;
            continue;
        }

        frame_data_buffer.resize(content_length);
        std::memcpy(frame_data_buffer.data(), data + HEADER_SIZE, content_length);
        
        // indicate that the frame was read
        data[0] = 0;
        
        // this will display frames received from the python producer
        // display_frame(content_length, frame_data_buffer);

        opendlv::proxy::ImageReading msg;
        msg.data(std::string(frame_data_buffer.begin(), frame_data_buffer.end())).fourcc("h264").width(640).height(480);
        od4.send(msg, cluon::time::now(), 0);

        frame_count += 1;
        if (frame_count % 100 == 0) {
            std::cout << "Received "<< std::dec << frame_count << " frames" << std::endl;
        }
    }
    
    // cv::destroyWindow("pywindow");
    shmdt(shm_ptr);
    std::cout << "Done. Received " << frame_count << " frames " << std::endl;

    return 0;
}

