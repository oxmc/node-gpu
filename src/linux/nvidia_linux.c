#include "../gpu_info.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void* nvml_library = NULL;
static int nvml_initialized = 0;

// NVML structures (same as Windows)
typedef struct {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
} nvmlMemory_t;

typedef struct {
    unsigned int gpu;
    unsigned int memory;
} nvmlUtilization_t;

// PCI info structure (same as Windows)
typedef struct {
    char busId[16];  // PCI bus ID in format: "0000:00:00.0"
    unsigned int domain;
    unsigned int bus;
    unsigned int device;
    unsigned int pciDeviceId;
    unsigned int pciSubSystemId;
} nvmlPciInfo_t;

// Function pointers (same as Windows)
static int (*nvmlInit_v2)(void) = NULL;
static int (*nvmlShutdown)(void) = NULL;
static int (*nvmlDeviceGetCount_v2)(unsigned int*) = NULL;
static int (*nvmlDeviceGetHandleByIndex)(unsigned int, void**) = NULL;
static int (*nvmlDeviceGetName)(void*, char*, unsigned int) = NULL;
static int (*nvmlDeviceGetUUID)(void*, char*, unsigned int) = NULL;
static int (*nvmlDeviceGetMemoryInfo)(void*, nvmlMemory_t*) = NULL;
static int (*nvmlDeviceGetUtilizationRates)(void*, nvmlUtilization_t*) = NULL;
static int (*nvmlDeviceGetTemperature)(void*, int, unsigned int*) = NULL;
static int (*nvmlDeviceGetPowerUsage)(void*, unsigned int*) = NULL;
static int (*nvmlDeviceGetClockInfo)(void*, int, unsigned int*) = NULL;
static int (*nvmlDeviceGetFanSpeed)(void*, unsigned int*) = NULL;
static int (*nvmlDeviceGetPciInfo)(void*, nvmlPciInfo_t*) = NULL;

static gpu_error_t load_nvml_linux(void) {
    if (nvml_initialized) return GPU_SUCCESS;
    
    // Try to load NVML library from common locations
    nvml_library = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!nvml_library) {
        nvml_library = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    if (!nvml_library) {
        return GPU_ERROR_NOT_SUPPORTED;
    }
    
    // Load all NVML functions
    nvmlInit_v2 = (int(*)(void))dlsym(nvml_library, "nvmlInit_v2");
    nvmlShutdown = (int(*)(void))dlsym(nvml_library, "nvmlShutdown");
    nvmlDeviceGetCount_v2 = (int(*)(unsigned int*))dlsym(nvml_library, "nvmlDeviceGetCount_v2");
    nvmlDeviceGetHandleByIndex = (int(*)(unsigned int, void**))dlsym(nvml_library, "nvmlDeviceGetHandleByIndex");
    nvmlDeviceGetName = (int(*)(void*, char*, unsigned int))dlsym(nvml_library, "nvmlDeviceGetName");
    nvmlDeviceGetUUID = (int(*)(void*, char*, unsigned int))dlsym(nvml_library, "nvmlDeviceGetUUID");
    nvmlDeviceGetMemoryInfo = (int(*)(void*, nvmlMemory_t*))dlsym(nvml_library, "nvmlDeviceGetMemoryInfo");
    nvmlDeviceGetUtilizationRates = (int(*)(void*, nvmlUtilization_t*))dlsym(nvml_library, "nvmlDeviceGetUtilizationRates");
    nvmlDeviceGetTemperature = (int(*)(void*, int, unsigned int*))dlsym(nvml_library, "nvmlDeviceGetTemperature");
    nvmlDeviceGetPowerUsage = (int(*)(void*, unsigned int*))dlsym(nvml_library, "nvmlDeviceGetPowerUsage");
    nvmlDeviceGetClockInfo = (int(*)(void*, int, unsigned int*))dlsym(nvml_library, "nvmlDeviceGetClockInfo");
    nvmlDeviceGetFanSpeed = (int(*)(void*, unsigned int*))dlsym(nvml_library, "nvmlDeviceGetFanSpeed");
    nvmlDeviceGetPciInfo = (int(*)(void*, nvmlPciInfo_t*))dlsym(nvml_library, "nvmlDeviceGetPciInfo");
    
    // Check for essential functions
    if (!nvmlInit_v2 || !nvmlDeviceGetCount_v2 || !nvmlDeviceGetHandleByIndex) {
        dlclose(nvml_library);
        nvml_library = NULL;
        return GPU_ERROR_NOT_SUPPORTED;
    }
    
    if (nvmlInit_v2() != 0) {
        dlclose(nvml_library);
        nvml_library = NULL;
        return GPU_ERROR_NOT_SUPPORTED;
    }
    
    nvml_initialized = 1;
    return GPU_SUCCESS;
}

gpu_error_t nvidia_linux_get_gpu_count(int32_t* count) {
    if (!count) return GPU_ERROR_INVALID_PARAM;
    
    gpu_error_t result = load_nvml_linux();
    if (result != GPU_SUCCESS) {
        *count = 0;
        return GPU_SUCCESS;
    }
    
    unsigned int nv_count = 0;
    if (nvmlDeviceGetCount_v2(&nv_count) != 0) {
        *count = 0;
        return GPU_ERROR_API_FAILED;
    }
    
    *count = (int32_t)nv_count;
    return GPU_SUCCESS;
}

gpu_error_t nvidia_linux_get_gpu_info(int32_t index, gpu_info_t* info) {
    if (!info) return GPU_ERROR_INVALID_PARAM;
    
    gpu_error_t result = load_nvml_linux();
    if (result != GPU_SUCCESS) {
        return result;
    }
    
    void* device;
    if (nvmlDeviceGetHandleByIndex((unsigned int)index, &device) != 0) {
        return GPU_ERROR_API_FAILED;
    }
    
    memset(info, 0, sizeof(gpu_info_t));
    info->index = index;
    info->vendor = GPU_VENDOR_NVIDIA;
    
    // Get GPU name
    char name[256];
    if (nvmlDeviceGetName && nvmlDeviceGetName(device, name, sizeof(name)) == 0) {
        strncpy(info->name, name, sizeof(info->name) - 1);
    } else {
        strncpy(info->name, "NVIDIA GPU", sizeof(info->name) - 1);
    }
    
    // Get UUID
    char uuid[64];
    if (nvmlDeviceGetUUID && nvmlDeviceGetUUID(device, uuid, sizeof(uuid)) == 0) {
        strncpy(info->uuid, uuid, sizeof(info->uuid) - 1);
    } else {
        snprintf(info->uuid, sizeof(info->uuid), "NVIDIA-%d", index);
    }
    
    // Get PCI bus information
    if (nvmlDeviceGetPciInfo) {
        nvmlPciInfo_t pciInfo;
        if (nvmlDeviceGetPciInfo(device, &pciInfo) == 0) {
            // Use the actual PCI bus ID from NVML
            strncpy(info->pci_bus_id, pciInfo.busId, sizeof(info->pci_bus_id) - 1);
            
            // Debug output to see all PCI information (optional)
            /*printf("GPU %d PCI Info:\n", index);
            printf("  Bus ID: %s\n", pciInfo.busId);
            printf("  Domain: 0x%04X\n", pciInfo.domain);
            printf("  Bus: 0x%02X\n", pciInfo.bus);
            printf("  Device: 0x%02X\n", pciInfo.device);
            printf("  Device ID: 0x%04X\n", pciInfo.pciDeviceId);
            printf("  Subsystem ID: 0x%04X\n", pciInfo.pciSubSystemId);*/
        } else {
            // Fallback: generate from index
            snprintf(info->pci_bus_id, sizeof(info->pci_bus_id), "PCI:%d", index);
        }
    } else {
        // Fallback: generate from index
        snprintf(info->pci_bus_id, sizeof(info->pci_bus_id), "PCI:%d", index);
    }
    
    // Get memory info
    nvmlMemory_t memory;
    if (nvmlDeviceGetMemoryInfo && nvmlDeviceGetMemoryInfo(device, &memory) == 0) {
        info->memory_total = memory.total / (1024 * 1024); // Convert to MB
        info->memory_used = memory.used / (1024 * 1024);
        info->memory_free = memory.free / (1024 * 1024);
        
        // Calculate memory capacity utilization (for reference)
        float capacity_utilization = 0.0f;
        if (memory.total > 0) {
            capacity_utilization = (float)memory.used / memory.total * 100.0f;
        }
        
        // Debug output (optional)
        /*printf("GPU %d Memory: %llu/%llu MB (%.1f%% capacity)\n", 
               index, memory.used / (1024 * 1024), memory.total / (1024 * 1024),
               capacity_utilization);*/
    } else {
        // Fallback values
        info->memory_total = 8 * 1024;
        info->memory_used = 0;
        info->memory_free = 8 * 1024;
    }
    
    // Get utilization rates
    nvmlUtilization_t utilization;
    if (nvmlDeviceGetUtilizationRates && nvmlDeviceGetUtilizationRates(device, &utilization) == 0) {
        info->gpu_utilization = (float)utilization.gpu;
        info->memory_utilization = (float)utilization.memory; // Memory bandwidth utilization
        
        // Debug output (optional)
        /*printf("GPU %d Utilization: GPU=%u%%, Memory=%u%%\n", 
               index, utilization.gpu, utilization.memory);*/
    } else {
        info->gpu_utilization = 0.0f;
        info->memory_utilization = 0.0f;
    }
    
    // Get temperature (GPU core)
    unsigned int temperature;
    if (nvmlDeviceGetTemperature && nvmlDeviceGetTemperature(device, 0, &temperature) == 0) {
        info->temperature = (float)temperature;
    } else {
        info->temperature = 0.0f;
    }
    
    // Get power usage (in milliwatts)
    unsigned int power;
    if (nvmlDeviceGetPowerUsage && nvmlDeviceGetPowerUsage(device, &power) == 0) {
        info->power_usage = (float)power / 1000.0f; // Convert to watts
    } else {
        info->power_usage = 0.0f;
    }
    
    // Get core clock (graphics clock)
    unsigned int clock;
    if (nvmlDeviceGetClockInfo && nvmlDeviceGetClockInfo(device, 0, &clock) == 0) {
        info->core_clock = clock;
    } else {
        info->core_clock = 0;
    }
    
    // Get memory clock
    if (nvmlDeviceGetClockInfo && nvmlDeviceGetClockInfo(device, 1, &clock) == 0) {
        info->memory_clock = clock;
    } else {
        info->memory_clock = 0;
    }
    
    // Get fan speed
    unsigned int fan_speed;
    if (nvmlDeviceGetFanSpeed && nvmlDeviceGetFanSpeed(device, &fan_speed) == 0) {
        info->fan_speed = (float)fan_speed;
    } else {
        info->fan_speed = 0.0f;
    }
    
    return GPU_SUCCESS;
}

// Cleanup function
void nvidia_linux_cleanup(void) {
    if (nvml_initialized && nvmlShutdown) {
        nvmlShutdown();
    }
    if (nvml_library) {
        dlclose(nvml_library);
        nvml_library = NULL;
    }
    nvml_initialized = 0;
}