#ifndef GPU_INFO_H
#define GPU_INFO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// GPU vendor types
typedef enum {
    GPU_VENDOR_UNKNOWN = 0,
    GPU_VENDOR_NVIDIA,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL
} gpu_vendor_t;

// GPU information structure
typedef struct {
    int32_t index;
    gpu_vendor_t vendor;
    char name[256];
    char uuid[64];
    char pci_bus_id[32];
    
    // Memory information (in MB)
    uint64_t memory_total;
    uint64_t memory_used;
    uint64_t memory_free;
    
    // Utilization (0-100)
    float gpu_utilization;
    float memory_utilization;
    
    // Temperature in Celsius
    float temperature;
    
    // Power in Watts
    float power_usage;
    
    // Clocks in MHz
    uint32_t core_clock;
    uint32_t memory_clock;
    
    // Fan speed in percentage (0-100)
    float fan_speed;
} gpu_info_t;

// Error codes
typedef enum {
    GPU_SUCCESS = 0,
    GPU_ERROR_NOT_SUPPORTED = -1,
    GPU_ERROR_NO_GPU = -2,
    GPU_ERROR_ACCESS_DENIED = -3,
    GPU_ERROR_INVALID_PARAM = -4,
    GPU_ERROR_API_FAILED = -5
} gpu_error_t;

// Initialization and cleanup
gpu_error_t gpu_info_init(void);
gpu_error_t gpu_info_cleanup(void);

// GPU discovery
gpu_error_t gpu_get_count(int32_t* count);
gpu_error_t gpu_get_info(int32_t index, gpu_info_t* info);

// Platform-specific implementations
gpu_error_t nvidia_get_gpu_count(int32_t* count);
gpu_error_t nvidia_get_gpu_info(int32_t index, gpu_info_t* info);

gpu_error_t amd_get_gpu_count(int32_t* count);
gpu_error_t amd_get_gpu_info(int32_t index, gpu_info_t* info);

gpu_error_t intel_get_gpu_count(int32_t* count);
gpu_error_t intel_get_gpu_info(int32_t index, gpu_info_t* info);

// Utility functions
const char* gpu_error_string(gpu_error_t error);
bool gpu_vendor_supported(gpu_vendor_t vendor);

#ifdef __cplusplus
}
#endif

#endif // GPU_INFO_H