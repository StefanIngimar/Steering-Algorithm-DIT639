#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <cluon-complete.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <onnxruntime_cxx_api.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <sys/ioctl.h>
#include <vector>
#include <chrono>
#include <numeric>
/*
 * This video-hsv-inspector detects cones with the help of a ML model. The model has been converted from pytorch (.pt) format to ONNX (.onnx) format. 
 * Information on ONNX: https://medium.com/@shivprataprai11/understanding-onnx-an-open-standard-for-deep-learning-models-350a72714660
 * Information on pytorch: https://pytorch.org/tutorials/beginner/saving_loading_models.html
 * This approach uses ONNX runtime framework for C++ to execute the model in real time.
 * Input is the frames from the shared memory, and output is the bounding boxes around the cones.
 */
int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    auto args = cluon::getCommandlineArguments(argc, argv);
    if (args.count("name") == 0 || args.count("width") == 0 || args.count("height") == 0) {
        std::cerr << argv[0] << " --name=<shared mem name> --width=<W> --height=<H>" << std::endl;
        return retCode;
    }

    const std::string NAME = args["name"];
    const uint32_t WIDTH = std::stoi(args["width"]);
    const uint32_t HEIGHT = std::stoi(args["height"]);

    std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
    if (sharedMemory && sharedMemory->valid()) {
        std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << "' (" << sharedMemory->size() << " bytes)." << std::endl;
        // model is initialized here
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "yolo");
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        Ort::Session session(env, "/usr/share/models/color_cones_yolov5n.onnx", session_options); // path in docker env

        cv::namedWindow("ONNX Runtime Output", cv::WINDOW_AUTOSIZE);
        cv::Mat img(HEIGHT, WIDTH, CV_8UC4);

        std::deque<double> fps_history;
        const size_t max_history = 30;
        auto last_time = std::chrono::steady_clock::now();


        while (cv::waitKey(10) != 27) {
            sharedMemory->lock();
            std::memcpy(img.data, sharedMemory->data(), WIDTH * HEIGHT * 4);
            sharedMemory->unlock();
            // fps counter. current average fps on my machine is 11. will revisit this and check if it can be improved.
            auto current_time = std::chrono::steady_clock::now();
            double frame_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
            last_time = current_time;

            double fps = frame_time_ms > 0.0 ? 1000.0 / frame_time_ms : 0.0;
            fps_history.push_back(fps);
            if(fps_history.size() > max_history){
              fps_history.pop_front();
            }

            // the boxes for the cones are below. should be fixed in the future
            // TODO separate the classes and output the color of the cones. this solution was just to create an MVP
            cv::Mat bgr;
            cv::cvtColor(img, bgr, cv::COLOR_BGRA2BGR);
            cv::Mat resized;
            cv::resize(bgr, resized, cv::Size(640, 640)); // expected format for yolo
            resized.convertTo(resized, CV_32F, 1.0f / 255.0f);
            // tensor input. more info: https://onnx.ai/onnx/intro/concepts.html
            std::array<int64_t, 4> input_shape{1, 3, 640, 640};
            std::vector<float> input_tensor_values(1 * 3 * 640 * 640);
            std::vector<const char*> input_names = {"images"};
            std::vector<const char*> output_names = {"output"};
            // rgb color.
            std::vector<cv::Mat> channels(3);
            cv::split(resized, channels);
            std::memcpy(input_tensor_values.data(), channels[0].data, 640 * 640 * sizeof(float));
            std::memcpy(input_tensor_values.data() + 640 * 640, channels[1].data, 640 * 640 * sizeof(float));
            std::memcpy(input_tensor_values.data() + 2 * 640 * 640, channels[2].data, 640 * 640 * sizeof(float));

            Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_values.size(), input_shape.data(), input_shape.size());

            auto output_tensors = session.Run(Ort::RunOptions{nullptr}, input_names.data(), &input_tensor, 1, output_names.data(), 1);

            float* output_data = output_tensors[0].GetTensorMutableData<float>();
            //int64_t* shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape().data();
            std::vector<int64_t> shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
            int num_detections = shape[1];
            int num_attrs = shape[2];

            for (int i = 0; i < num_detections; ++i) {
                float conf = output_data[i * num_attrs + 4];
                if (conf > 0.4f) { // 40% confidence for the detections
                    float x_center = output_data[i * num_attrs + 0] * WIDTH / 640.0f;
                    float y_center = output_data[i * num_attrs + 1] * HEIGHT / 640.0f;
                    float w = output_data[i * num_attrs + 2] * WIDTH / 640.0f;
                    float h = output_data[i * num_attrs + 3] * HEIGHT / 640.0f;
                    int left = static_cast<int>(x_center - w / 2);
                    int top = static_cast<int>(y_center - h / 2);
                    int width = static_cast<int>(w);
                    int height = static_cast<int>(h);
                    cv::rectangle(bgr, cv::Rect(left, top, width, height), cv::Scalar(0, 255, 0), 2);
                }
            }

            cv::imshow("ONNX Runtime Output", bgr);
        }
    }

    return 0;
}
