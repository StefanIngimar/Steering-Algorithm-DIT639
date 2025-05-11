#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>

#include "onnx_runtime.hpp"
#include "logger.hpp"

OnnxRuntime::OnnxRuntime(const std::string& model_path, int trained_frame_width, int trained_frame_height)
    : m_model_path(model_path),
      m_trained_frame_width(trained_frame_width), 
      m_trained_frame_height(trained_frame_height),
      m_env(ORT_LOGGING_LEVEL_WARNING, "OnnxRuntime"),
      m_session(nullptr) {
}

void OnnxRuntime::load() {
    auto logger = Logger::get_instance().get_logger();
    
    try {
        logger->debug("Loading ONNX model: {}", m_model_path);
        // configure how the model is executed 
        Ort::SessionOptions session_options;
        m_session = Ort::Session(m_env, m_model_path.c_str(), session_options);

        // get all model input fields for specifying inputs later
        Ort::AllocatorWithDefaultOptions allocator;
        size_t num_inputs = m_session.GetInputCount();
        m_input_names.resize(num_inputs);
        for (size_t i = 0; i < num_inputs; i++) {
            m_input_names[i] = m_session.GetInputNameAllocated(i, allocator).get();
        }

        // get all model output fields for fetching results later
        size_t num_outputs = m_session.GetOutputCount();
        m_output_names.resize(num_outputs);
        for (size_t i = 0; i < num_outputs; i++) {
            m_output_names[i] = m_session.GetOutputNameAllocated(i, allocator).get();
        }

        // get the expected tensor shape
        auto input_type_info = m_session.GetInputTypeInfo(0);
        auto tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
        m_input_shape = tensor_info.GetShape();

        logger->info("Loaded ONNX model: {}, Input shape: {} {} {} {}", m_model_path, m_input_shape[0], m_input_shape[1], m_input_shape[2], m_input_shape[3]);
    } catch (const Ort::Exception& e) {
        logger->error("Failed to load ONNX model: {}. Error: {}", m_model_path, e.what());
        throw;
    } catch (const std::exception& e) {
        logger->error("Unexpected error loading ONNX model: {}. Error: {}", m_model_path, e.what());
        throw;
    }
}

std::vector<cv::Mat> OnnxRuntime::predict(const cv::Mat& image) {
    auto logger = Logger::get_instance().get_logger();
    
    try {
        if (image.empty()) {
            logger->warn("Runtime predict: Input image is empty");
            return {};
        }

        // resize the input image to the models expected size (to the size specified during training)
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(m_trained_frame_width, m_trained_frame_height));

        cv::Mat blob = cv::dnn::blobFromImage(resized, 1.0 / 255.0, cv::Size(m_trained_frame_width, m_trained_frame_height), cv::Scalar(0, 0, 0), true, false);
        
        // specify the memory allocator for input/output tensors and copy data into a vector for tensor creation
        Ort::MemoryInfo memory_info("Cpu", OrtArenaAllocator, 0, OrtMemTypeDefault);
        std::vector<float> input_data(blob.ptr<float>(), blob.ptr<float>() + blob.total());
        
        // create input tensor
        std::vector<Ort::Value> input_tensors;
        input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, input_data.data(), input_data.size(), m_input_shape.data(), m_input_shape.size()));
        
        // run inference/run the model and product the output tensors
        std::vector<const char*> input_names_cstr;
        for (const auto& name : m_input_names) {
            input_names_cstr.push_back(name.c_str());
        }
        std::vector<const char*> output_names_cstr;
        for (const auto& name : m_output_names) {
            output_names_cstr.push_back(name.c_str());
        }
        auto outputs = m_session.Run(
            Ort::RunOptions{nullptr}, 
            input_names_cstr.data(), 
            input_tensors.data(), 
            input_names_cstr.size(),
            output_names_cstr.data(), 
            output_names_cstr.size()
        );

        if (outputs.empty()) {
            logger->warn("Runtime predict: No outputs from inference");
            return {};
        }
        
        // reshape to match CvDnnRuntime: [1, num_detections, num_attributes]
        // without reshaping the program silently crashes
        auto output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
        int num_detections = output_shape[output_shape.size() - 2];
        int num_attributes = output_shape[output_shape.size() - 1];
        if (num_detections <= 0 || num_attributes <= 0) {
            logger->error("Runtime predict: Invalid output dimensions: num_detections={}, num_attributes={}", num_detections, num_attributes);
            return {};
        }

        float* tensor_data = outputs[0].GetTensorMutableData<float>();
        if (!tensor_data) {
            logger->error("Runtime predict: Output tensor data is null");
            return {};
        }

        // create cv::Mat with shape [1, num_detections, num_attributes]
        cv::Mat output_mat(cv::Size(num_attributes, num_detections), CV_32F, tensor_data, cv::Mat::AUTO_STEP);
        logger->debug("Runtime predict: Output Mat created, size={}x{}, type={}", output_mat.cols, output_mat.rows, output_mat.type());
        if (output_mat.empty() || !output_mat.isContinuous()) {
            logger->error("Runtime predict: Output Mat is empty or not continuous");
            return {};
        }

        // reshape to [1, num_detections, num_attributes] for compatibility
        output_mat = output_mat.reshape(1, {1, num_detections, num_attributes});
        logger->debug("Runtime predict: Reshaped Mat to [1, {}, {}]", num_detections, num_attributes);

        return {output_mat};
    } catch (const Ort::Exception& e) {
        logger->error("Inference failed: {}", e.what());
        return {};
    } catch (const cv::Exception& e) {
        logger->error("OpenCV error in predict: {}", e.what());
        return {};
    } catch (const std::exception& e) {
        logger->error("Unexpected error in predict: {}", e.what());
        return {};
    } catch (...) {
        logger->error("Unknown error in predict");
        return {};
    }
}

int OnnxRuntime::get_trained_frame_width() const {
    return m_trained_frame_width;
}

int OnnxRuntime::get_trained_frame_height() const {
    return m_trained_frame_height;
}
