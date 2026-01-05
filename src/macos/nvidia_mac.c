#include "../gpu_info.h"
#include <stdio.h>
#include <string.h>

// macOS NVIDIA support is extremely limited
// Apple dropped NVIDIA support after macOS High Sierra (10.13)

gpu_error_t nvidia_macos_get_gpu_count(int32_t* count) {
    if (!count) return GPU_ERROR_INVALID_PARAM;
    
    // NVIDIA GPUs are not supported on modern macOS
    *count = 0;
    return GPU_SUCCESS;
}

gpu_error_t nvidia_macos_get_gpu_info(int32_t index, gpu_info_t* info) {
    if (!info) return GPU_ERROR_INVALID_PARAM;
    
    // NVIDIA GPUs are not supported on modern macOS
    return GPU_ERROR_NOT_SUPPORTED;
}
