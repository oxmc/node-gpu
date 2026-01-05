#include "../gpu_info.h"

// Forward declarations for platform-specific implementations
#ifdef _WIN32
gpu_error_t intel_windows_get_gpu_count(int32_t* count);
gpu_error_t intel_windows_get_gpu_info(int32_t index, gpu_info_t* info);
#elif defined(__APPLE__)
gpu_error_t intel_macos_get_gpu_count(int32_t* count);
gpu_error_t intel_macos_get_gpu_info(int32_t index, gpu_info_t* info);
#else
gpu_error_t intel_linux_get_gpu_count(int32_t* count);
gpu_error_t intel_linux_get_gpu_info(int32_t index, gpu_info_t* info);
#endif

gpu_error_t intel_get_gpu_count(int32_t* count) {
    if (!count) return GPU_ERROR_INVALID_PARAM;
    
#ifdef _WIN32
    return intel_windows_get_gpu_count(count);
#elif defined(__APPLE__)
    return intel_macos_get_gpu_count(count);
#else
    return intel_linux_get_gpu_count(count);
#endif
}

gpu_error_t intel_get_gpu_info(int32_t index, gpu_info_t* info) {
    if (!info) return GPU_ERROR_INVALID_PARAM;
    
#ifdef _WIN32
    return intel_windows_get_gpu_info(index, info);
#elif defined(__APPLE__)
    return intel_macos_get_gpu_info(index, info);
#else
    return intel_linux_get_gpu_info(index, info);
#endif
}