#include "../gpu_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

static int is_amd_device(const char* card_path) {
    char vendor_path[512];
    snprintf(vendor_path, sizeof(vendor_path), "%s/device/vendor", card_path);
    
    FILE* f = fopen(vendor_path, "r");
    if (!f) return 0;
    
    unsigned int vendor;
    int result = fscanf(f, "%x", &vendor);
    fclose(f);
    
    // AMD vendor ID is 0x1002
    return (result == 1 && vendor == 0x1002);
}

gpu_error_t amd_linux_get_gpu_count(int32_t* count) {
    if (!count) return GPU_ERROR_INVALID_PARAM;
    
    DIR* dir;
    struct dirent* entry;
    int amd_count = 0;
    
    dir = opendir("/sys/class/drm");
    if (!dir) {
        *count = 0;
        return GPU_ERROR_API_FAILED;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // Look for card* directories
        if (strncmp(entry->d_name, "card", 4) == 0 && 
            strlen(entry->d_name) == 5 && 
            entry->d_name[4] >= '0' && entry->d_name[4] <= '9') {
            
            char card_path[512];
            snprintf(card_path, sizeof(card_path), "/sys/class/drm/%s", entry->d_name);
            
            if (is_amd_device(card_path)) {
                amd_count++;
            }
        }
    }
    
    closedir(dir);
    *count = amd_count;
    return GPU_SUCCESS;
}

static int read_sysfs_string(const char* path, char* buffer, size_t size) {
    FILE* f = fopen(path, "r");
    if (!f) return 0;
    
    if (fgets(buffer, size, f)) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        fclose(f);
        return 1;
    }
    
    fclose(f);
    return 0;
}

static long read_sysfs_long(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    
    long value;
    int result = fscanf(f, "%ld", &value);
    fclose(f);
    
    return (result == 1) ? value : -1;
}

gpu_error_t amd_linux_get_gpu_info(int32_t index, gpu_info_t* info) {
    if (!info) return GPU_ERROR_INVALID_PARAM;
    
    // Find the correct card device
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return GPU_ERROR_API_FAILED;
    
    struct dirent* entry;
    int current_index = 0;
    char card_path[512] = {0};
    int found = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "card", 4) == 0 && 
            strlen(entry->d_name) == 5 && 
            entry->d_name[4] >= '0' && entry->d_name[4] <= '9') {
            
            char temp_path[512];
            snprintf(temp_path, sizeof(temp_path), "/sys/class/drm/%s", entry->d_name);
            
            if (is_amd_device(temp_path)) {
                if (current_index == index) {
                    strncpy(card_path, temp_path, sizeof(card_path) - 1);
                    found = 1;
                    break;
                }
                current_index++;
            }
        }
    }
    
    closedir(dir);
    
    if (!found) return GPU_ERROR_INVALID_PARAM;
    
    // Initialize structure
    memset(info, 0, sizeof(gpu_info_t));
    info->index = index;
    info->vendor = GPU_VENDOR_AMD;
    
    // Read GPU name from device
    char name_path[512];
    snprintf(name_path, sizeof(name_path), "%s/device/product_name", card_path);
    if (!read_sysfs_string(name_path, info->name, sizeof(info->name))) {
        // Fallback: try model or just use generic name
        snprintf(name_path, sizeof(name_path), "%s/device/model", card_path);
        if (!read_sysfs_string(name_path, info->name, sizeof(info->name))) {
            snprintf(info->name, sizeof(info->name), "AMD GPU %d", index);
        }
    }
    
    // Read device ID for UUID
    char device_path[512];
    snprintf(device_path, sizeof(device_path), "%s/device/device", card_path);
    unsigned int device_id = 0;
    FILE* f = fopen(device_path, "r");
    if (f) {
        fscanf(f, "%x", &device_id);
        fclose(f);
    }
    snprintf(info->uuid, sizeof(info->uuid), "AMD-Linux-0x%04X-%d", device_id, index);
    
    // Read PCI bus ID
    char pci_path[512];
    snprintf(pci_path, sizeof(pci_path), "%s/device/uevent", card_path);
    f = fopen(pci_path, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "PCI_SLOT_NAME=", 14) == 0) {
                strncpy(info->pci_bus_id, line + 14, sizeof(info->pci_bus_id) - 1);
                // Remove newline
                size_t len = strlen(info->pci_bus_id);
                if (len > 0 && info->pci_bus_id[len-1] == '\n') {
                    info->pci_bus_id[len-1] = '\0';
                }
                break;
            }
        }
        fclose(f);
    }
    if (info->pci_bus_id[0] == '\0') {
        snprintf(info->pci_bus_id, sizeof(info->pci_bus_id), "PCI:%d", index);
    }
    
    // Read memory information (in bytes, convert to MB)
    char mem_path[512];
    snprintf(mem_path, sizeof(mem_path), "%s/device/mem_info_vram_total", card_path);
    long mem_total = read_sysfs_long(mem_path);
    if (mem_total > 0) {
        info->memory_total = (int32_t)(mem_total / (1024 * 1024));
    }
    
    snprintf(mem_path, sizeof(mem_path), "%s/device/mem_info_vram_used", card_path);
    long mem_used = read_sysfs_long(mem_path);
    if (mem_used > 0) {
        info->memory_used = (int32_t)(mem_used / (1024 * 1024));
        if (info->memory_total > 0) {
            info->memory_free = info->memory_total - info->memory_used;
            info->memory_utilization = (float)info->memory_used / info->memory_total * 100.0f;
        }
    }
    
    // Read GPU utilization
    char util_path[512];
    snprintf(util_path, sizeof(util_path), "%s/device/gpu_busy_percent", card_path);
    long gpu_util = read_sysfs_long(util_path);
    if (gpu_util >= 0) {
        info->gpu_utilization = (float)gpu_util;
    }
    
    // Read temperature (in millidegrees, convert to Celsius)
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s/device/hwmon/hwmon*/temp1_input", card_path);
    // Note: This glob pattern won't work directly, we'd need to enumerate hwmon directories
    // For now, try hwmon0 and hwmon1
    for (int hwmon = 0; hwmon < 4; hwmon++) {
        snprintf(temp_path, sizeof(temp_path), "%s/device/hwmon/hwmon%d/temp1_input", card_path, hwmon);
        long temp = read_sysfs_long(temp_path);
        if (temp > 0) {
            info->temperature = temp / 1000.0f;
            break;
        }
    }
    
    // Read power usage (in microwatts, convert to watts)
    char power_path[512];
    for (int hwmon = 0; hwmon < 4; hwmon++) {
        snprintf(power_path, sizeof(power_path), "%s/device/hwmon/hwmon%d/power1_average", card_path, hwmon);
        long power = read_sysfs_long(power_path);
        if (power > 0) {
            info->power_usage = power / 1000000.0f;
            break;
        }
    }
    
    // Read clock speeds (in Hz, convert to MHz)
    char clock_path[512];
    snprintf(clock_path, sizeof(clock_path), "%s/device/pp_dpm_sclk", card_path);
    f = fopen(clock_path, "r");
    if (f) {
        char line[256];
        // Find the line with asterisk (current clock)
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '*')) {
                unsigned int clock_mhz;
                if (sscanf(line, "%*d: %uMhz", &clock_mhz) == 1) {
                    info->core_clock = clock_mhz;
                    break;
                }
            }
        }
        fclose(f);
    }
    
    snprintf(clock_path, sizeof(clock_path), "%s/device/pp_dpm_mclk", card_path);
    f = fopen(clock_path, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '*')) {
                unsigned int clock_mhz;
                if (sscanf(line, "%*d: %uMhz", &clock_mhz) == 1) {
                    info->memory_clock = clock_mhz;
                    break;
                }
            }
        }
        fclose(f);
    }
    
    // Read fan speed (percentage)
    char fan_path[512];
    for (int hwmon = 0; hwmon < 4; hwmon++) {
        snprintf(fan_path, sizeof(fan_path), "%s/device/hwmon/hwmon%d/pwm1", card_path, hwmon);
        long fan_pwm = read_sysfs_long(fan_path);
        if (fan_pwm >= 0) {
            // PWM is typically 0-255
            info->fan_speed = (fan_pwm / 255.0f) * 100.0f;
            break;
        }
    }
    
    return GPU_SUCCESS;
}