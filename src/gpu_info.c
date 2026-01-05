#include "gpu_info.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Forward declarations for vendor functions
gpu_error_t nvidia_get_gpu_count(int32_t* count);
gpu_error_t nvidia_get_gpu_info(int32_t index, gpu_info_t* info);
gpu_error_t amd_get_gpu_count(int32_t* count);
gpu_error_t amd_get_gpu_info(int32_t index, gpu_info_t* info);
gpu_error_t intel_get_gpu_count(int32_t* count);
gpu_error_t intel_get_gpu_info(int32_t index, gpu_info_t* info);

static bool g_initialized = false;

gpu_error_t gpu_info_init(void) {
    if (g_initialized) {
        return GPU_SUCCESS;
    }
    
    // Platform-specific initialization
#ifdef _WIN32
    // Windows-specific init
#else
    // Linux-specific init
#endif
    
    g_initialized = true;
    return GPU_SUCCESS;
}

gpu_error_t gpu_info_cleanup(void) {
    if (!g_initialized) {
        return GPU_SUCCESS;
    }
    
    // Platform-specific cleanup
    
    g_initialized = false;
    return GPU_SUCCESS;
}

gpu_error_t gpu_get_count(int32_t* count) {
    if (!g_initialized) {
        return GPU_ERROR_API_FAILED;
    }
    
    if (!count) {
        return GPU_ERROR_INVALID_PARAM;
    }
    
    int32_t total_count = 0;
    int32_t nvidia_count = 0;
    int32_t amd_count = 0;
    int32_t intel_count = 0;
    
    // Get counts from all vendors
    if (nvidia_get_gpu_count(&nvidia_count) == GPU_SUCCESS) {
        total_count += nvidia_count;
    }
    
    if (amd_get_gpu_count(&amd_count) == GPU_SUCCESS) {
        total_count += amd_count;
    }
    
    if (intel_get_gpu_count(&intel_count) == GPU_SUCCESS) {
        total_count += intel_count;
    }
    
    *count = total_count;
    return total_count > 0 ? GPU_SUCCESS : GPU_ERROR_NO_GPU;
}

gpu_error_t gpu_get_info(int32_t index, gpu_info_t* info) {
    if (!g_initialized || !info) {
        return GPU_ERROR_INVALID_PARAM;
    }
    
    int32_t nvidia_count = 0;
    int32_t amd_count = 0;
    int32_t intel_count = 0;
    
    // Get vendor counts
    nvidia_get_gpu_count(&nvidia_count);
    amd_get_gpu_count(&amd_count);
    intel_get_gpu_count(&intel_count);
    
    if (index < nvidia_count) {
        // NVIDIA GPU
        return nvidia_get_gpu_info(index, info);
    } else if (index < nvidia_count + amd_count) {
        // AMD GPU
        int32_t amd_index = index - nvidia_count;
        return amd_get_gpu_info(amd_index, info);
    } else if (index < nvidia_count + amd_count + intel_count) {
        // Intel GPU
        int32_t intel_index = index - nvidia_count - amd_count;
        return intel_get_gpu_info(intel_index, info);
    }
    
    return GPU_ERROR_INVALID_PARAM;
}

const char* gpu_error_string(gpu_error_t error) {
    switch (error) {
        case GPU_SUCCESS: return "Success";
        case GPU_ERROR_NOT_SUPPORTED: return "Operation not supported";
        case GPU_ERROR_NO_GPU: return "No GPU found";
        case GPU_ERROR_ACCESS_DENIED: return "Access denied";
        case GPU_ERROR_INVALID_PARAM: return "Invalid parameter";
        case GPU_ERROR_API_FAILED: return "API call failed";
        default: return "Unknown error";
    }
}

bool gpu_vendor_supported(gpu_vendor_t vendor) {
    switch (vendor) {
        case GPU_VENDOR_NVIDIA:
        case GPU_VENDOR_AMD:
        case GPU_VENDOR_INTEL:
            return true;
        default:
            return false;
    }
}