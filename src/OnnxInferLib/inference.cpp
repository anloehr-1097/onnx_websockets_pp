#include <cassert>
#include <cstddef>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include "include/inference.h"

Ort::MemoryInfo get_mem_info(std::string memtype){
    if (memtype == "cpu"){
        return Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    }
    else {
        throw "GPU memory info not implemented yet.";
    };
}



