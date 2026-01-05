#include "../gpu_info.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <stdio.h>
#include <string.h>

gpu_error_t amd_macos_get_gpu_count(int32_t* count) {
    if (!count) return GPU_ERROR_INVALID_PARAM;
    
    int amd_count = 0;
    io_iterator_t iterator;
    
    // Get list of all GPUs
    kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                     IOServiceMatching("IOPCIDevice"),
                                                     &iterator);
    
    if (kr != KERN_SUCCESS) {
        *count = 0;
        return GPU_ERROR_API_FAILED;
    }
    
    io_object_t service;
    while ((service = IOIteratorNext(iterator))) {
        // Check if it's a display device
        CFTypeRef vendorID = IORegistryEntryCreateCFProperty(service, CFSTR("vendor-id"),
                                                               kCFAllocatorDefault, 0);
        
        if (vendorID) {
            UInt32 vendor = 0;
            CFDataGetBytes(vendorID, CFRangeMake(0, sizeof(UInt32)), (UInt8*)&vendor);
            CFRelease(vendorID);
            
            // AMD vendor ID is 0x1002
            if (vendor == 0x1002) {
                amd_count++;
            }
        }
        
        IOObjectRelease(service);
    }
    
    IOObjectRelease(iterator);
    *count = amd_count;
    return GPU_SUCCESS;
}

gpu_error_t amd_macos_get_gpu_info(int32_t index, gpu_info_t* info) {
    if (!info) return GPU_ERROR_INVALID_PARAM;
    
    int current_index = 0;
    io_iterator_t iterator;
    
    kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                     IOServiceMatching("IOPCIDevice"),
                                                     &iterator);
    
    if (kr != KERN_SUCCESS) {
        return GPU_ERROR_API_FAILED;
    }
    
    io_object_t service;
    int found = 0;
    
    while ((service = IOIteratorNext(iterator))) {
        CFTypeRef vendorID = IORegistryEntryCreateCFProperty(service, CFSTR("vendor-id"),
                                                               kCFAllocatorDefault, 0);
        
        if (vendorID) {
            UInt32 vendor = 0;
            CFDataGetBytes(vendorID, CFRangeMake(0, sizeof(UInt32)), (UInt8*)&vendor);
            CFRelease(vendorID);
            
            if (vendor == 0x1002) {  // AMD
                if (current_index == index) {
                    found = 1;
                    
                    memset(info, 0, sizeof(gpu_info_t));
                    info->index = index;
                    info->vendor = GPU_VENDOR_AMD;
                    
                    // Get model name
                    CFTypeRef model = IORegistryEntryCreateCFProperty(service, CFSTR("model"),
                                                                        kCFAllocatorDefault, 0);
                    if (model && CFGetTypeID(model) == CFDataGetTypeID()) {
                        const char* modelStr = (const char*)CFDataGetBytePtr(model);
                        snprintf(info->name, sizeof(info->name), "%s", modelStr);
                        CFRelease(model);
                    } else {
                        snprintf(info->name, sizeof(info->name), "AMD GPU %d", index);
                    }
                    
                    // Get device ID for UUID
                    CFTypeRef deviceID = IORegistryEntryCreateCFProperty(service, CFSTR("device-id"),
                                                                           kCFAllocatorDefault, 0);
                    if (deviceID) {
                        UInt32 devID = 0;
                        CFDataGetBytes(deviceID, CFRangeMake(0, sizeof(UInt32)), (UInt8*)&devID);
                        snprintf(info->uuid, sizeof(info->uuid), "AMD-macOS-0x%04X", devID);
                        CFRelease(deviceID);
                    }
                    
                    snprintf(info->pci_bus_id, sizeof(info->pci_bus_id), "PCI:%d", index);
                    
                    // macOS does not provide detailed GPU monitoring APIs
                    // Metal framework and IOKit have limited metrics access
                    info->memory_total = 0;
                    info->memory_used = 0;
                    info->memory_free = 0;
                    info->gpu_utilization = 0.0f;
                    info->memory_utilization = 0.0f;
                    info->temperature = 0.0f;
                    info->power_usage = 0.0f;
                    info->core_clock = 0;
                    info->memory_clock = 0;
                    info->fan_speed = 0.0f;
                    
                    IOObjectRelease(service);
                    break;
                }
                current_index++;
            }
        }
        
        IOObjectRelease(service);
    }
    
    IOObjectRelease(iterator);
    return found ? GPU_SUCCESS : GPU_ERROR_API_FAILED;
}
