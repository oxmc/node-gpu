#include <napi.h>
extern "C" {
#include "gpu_info.h"
}
#include <string>

namespace gpu {

/**
 * Convert gpu_info_t struct to JavaScript object
 */
Napi::Object GpuInfoToObject(Napi::Env env, const gpu_info_t& info) {
    Napi::Object obj = Napi::Object::New(env);
    
    obj.Set("index", Napi::Number::New(env, info.index));
    
    // Vendor
    std::string vendor_str;
    switch (info.vendor) {
        case GPU_VENDOR_NVIDIA: vendor_str = "NVIDIA"; break;
        case GPU_VENDOR_AMD: vendor_str = "AMD"; break;
        case GPU_VENDOR_INTEL: vendor_str = "Intel"; break;
        default: vendor_str = "Unknown"; break;
    }
    obj.Set("vendor", Napi::String::New(env, vendor_str));
    
    obj.Set("name", Napi::String::New(env, info.name));
    obj.Set("uuid", Napi::String::New(env, info.uuid));
    obj.Set("pciBusId", Napi::String::New(env, info.pci_bus_id));
    
    // Memory (in MB)
    obj.Set("memoryTotal", Napi::Number::New(env, info.memory_total));
    obj.Set("memoryUsed", Napi::Number::New(env, info.memory_used));
    obj.Set("memoryFree", Napi::Number::New(env, info.memory_free));
    
    // Utilization (percentage)
    obj.Set("gpuUtilization", Napi::Number::New(env, info.gpu_utilization));
    obj.Set("memoryUtilization", Napi::Number::New(env, info.memory_utilization));
    
    // Temperature (Celsius)
    obj.Set("temperature", Napi::Number::New(env, info.temperature));
    
    // Power (Watts)
    obj.Set("powerUsage", Napi::Number::New(env, info.power_usage));
    
    // Clocks (MHz)
    obj.Set("coreClock", Napi::Number::New(env, info.core_clock));
    obj.Set("memoryClock", Napi::Number::New(env, info.memory_clock));
    
    // Fan speed (percentage)
    obj.Set("fanSpeed", Napi::Number::New(env, info.fan_speed));
    
    return obj;
}

/**
 * Node.js binding: initialize()
 * Initialize the GPU information library
 */
Napi::Value Initialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    gpu_error_t result = gpu_info_init();
    
    if (result != GPU_SUCCESS) {
        Napi::Error::New(env, "Failed to initialize GPU library")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return Napi::Boolean::New(env, true);
}

/**
 * Node.js binding: cleanup()
 * Cleanup the GPU information library
 */
Napi::Value Cleanup(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    gpu_error_t result = gpu_info_cleanup();
    
    if (result != GPU_SUCCESS) {
        Napi::Error::New(env, "Failed to cleanup GPU library")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return Napi::Boolean::New(env, true);
}

/**
 * Node.js binding: getGpuCount()
 * Get the number of GPUs in the system
 */
Napi::Value GetGpuCount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    int32_t count = 0;
    gpu_error_t result = gpu_get_count(&count);
    
    if (result != GPU_SUCCESS) {
        if (result == GPU_ERROR_NO_GPU) {
            return Napi::Number::New(env, 0);
        }
        Napi::Error::New(env, "Failed to get GPU count")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return Napi::Number::New(env, count);
}

/**
 * Node.js binding: getGpuInfo(index)
 * Get information about a specific GPU by index
 */
Napi::Value GetGpuInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // Check arguments
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected GPU index as number")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int32_t index = info[0].As<Napi::Number>().Int32Value();
    
    gpu_info_t gpu_info;
    gpu_error_t result = gpu_get_info(index, &gpu_info);
    
    if (result != GPU_SUCCESS) {
        std::string error_msg = "Failed to get GPU info for index " + std::to_string(index);
        Napi::Error::New(env, error_msg)
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return GpuInfoToObject(env, gpu_info);
}

/**
 * Node.js binding: getAllGpuInfo()
 * Get information about all GPUs in the system
 */
Napi::Value GetAllGpuInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // Get GPU count
    int32_t count = 0;
    gpu_error_t result = gpu_get_count(&count);
    
    if (result != GPU_SUCCESS || count == 0) {
        return Napi::Array::New(env, 0);
    }
    
    // Create array to hold all GPU info
    Napi::Array gpuArray = Napi::Array::New(env, count);
    
    // Get info for each GPU
    for (int32_t i = 0; i < count; i++) {
        gpu_info_t gpu_info;
        result = gpu_get_info(i, &gpu_info);

        if (result == GPU_SUCCESS) {
            gpuArray.Set(static_cast<uint32_t>(i), GpuInfoToObject(env, gpu_info));
        } else {
            gpuArray.Set(static_cast<uint32_t>(i), env.Null());
        }
    }
    
    return gpuArray;
}

/**
 * Initialize the Node.js addon
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Auto-initialize on module load
    gpu_info_init();
    
    // Export functions
    exports.Set("initialize", Napi::Function::New(env, Initialize));
    exports.Set("cleanup", Napi::Function::New(env, Cleanup));
    exports.Set("getGpuCount", Napi::Function::New(env, GetGpuCount));
    exports.Set("getGpuInfo", Napi::Function::New(env, GetGpuInfo));
    exports.Set("getAllGpuInfo", Napi::Function::New(env, GetAllGpuInfo));
    
    return exports;
}

} // namespace gpu

// Register the addon (must be outside namespace)
Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    return gpu::Init(env, exports);
}

NODE_API_MODULE(gpu, InitModule)
