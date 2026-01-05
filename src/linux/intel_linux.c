#include "../gpu_info.h"
#include <stdio.h>
#include <string.h>

// Intel GPU detection on Linux is not yet implemented
// Would require i915 sysfs interface or similar

gpu_error_t intel_linux_get_gpu_count(int32_t* count) {
    if (!count) return GPU_ERROR_INVALID_PARAM;
    
    // Not yet implemented
    *count = 0;
    return GPU_SUCCESS;
}

gpu_error_t intel_linux_get_gpu_info(int32_t index, gpu_info_t* info) {
    if (!info) return GPU_ERROR_INVALID_PARAM;
    
    return GPU_ERROR_NOT_SUPPORTED;
}
