# node-gpu

Cross-platform Node.js native addon for reading GPU (Graphics Processing Unit) information from NVIDIA, AMD, and Intel graphics cards.

## Features

- 🎮 Multi-vendor GPU support (NVIDIA, AMD, Intel)
- 🖥️ Cross-platform support (Windows, macOS, Linux)
- ⚡ Fast native C implementation
- 📊 Comprehensive GPU metrics (temperature, utilization, memory, clocks, etc.)
- 📦 Easy to use Node.js API
- 🔒 No external dependencies (besides node-gyp for building)

## Installation

```bash
npm install
```

This will automatically compile the native addon for your platform.

## Usage

```javascript
const gpu = require('node-gpu');

// Get number of GPUs
const gpuCount = gpu.getGpuCount();
console.log('Number of GPUs:', gpuCount);

// Get information for first GPU
const gpuInfo = gpu.getGpuInfo(0);
console.log('GPU Name:', gpuInfo.name);
console.log('Vendor:', gpuInfo.vendor);
console.log('Memory Total:', gpuInfo.memoryTotal, 'MB');
console.log('GPU Utilization:', gpuInfo.gpuUtilization, '%');
console.log('Temperature:', gpuInfo.temperature, '°C');
console.log('Power Usage:', gpuInfo.powerUsage, 'W');

// Get all GPU information at once
const allGpus = gpu.getAllGpuInfo();
allGpus.forEach((gpu, index) => {
    console.log(`GPU ${index}:`, gpu.name, `(${gpu.vendor})`);
});
```

## API

### `getGpuCount()`
Returns the number of GPUs detected in the system.

**Returns:** `number`

### `getGpuInfo(index)`
Gets detailed information about a specific GPU.

**Parameters:**
- `index` (number): Zero-based GPU index

**Returns:** Object with the following properties:
- `index` (number): GPU index
- `vendor` (string): GPU vendor (NVIDIA, AMD, Intel, or Unknown)
- `name` (string): GPU model name
- `uuid` (string): GPU unique identifier
- `pciBusId` (string): PCI bus ID
- `memoryTotal` (number): Total memory in MB
- `memoryUsed` (number): Used memory in MB
- `memoryFree` (number): Free memory in MB
- `gpuUtilization` (number): GPU utilization percentage (0-100)
- `memoryUtilization` (number): Memory utilization percentage (0-100)
- `temperature` (number): GPU temperature in Celsius
- `powerUsage` (number): Power usage in Watts
- `coreClock` (number): Core clock speed in MHz
- `memoryClock` (number): Memory clock speed in MHz
- `fanSpeed` (number): Fan speed percentage (0-100)

### `getAllGpuInfo()`
Gets information about all GPUs in the system.

**Returns:** Array of GPU info objects

### `initialize()`
Manually initialize the GPU library (automatically called on module load).

### `cleanup()`
Cleanup GPU library resources.

## Supported Information

- GPU vendor and model name
- Memory usage (total, used, free)
- GPU and memory utilization
- Temperature monitoring
- Power consumption
- Clock speeds (core and memory)
- Fan speed
- PCI bus information
- GPU UUID

## Platform-Specific Notes

### Windows
- NVIDIA: Uses NVML (NVIDIA Management Library)
- AMD: Uses ADL (AMD Display Library) and ADLX
- Intel: Uses Windows APIs
- Some information may require administrator privileges

### macOS
- Uses Metal framework and IOKit
- NVIDIA GPUs may have limited support on recent macOS versions

### Linux
- NVIDIA: Requires NVIDIA drivers with NVML support
- AMD: Uses sysfs and AMD driver APIs
- Intel: Uses sysfs and i915 driver information
- May require appropriate permissions for some metrics

## Building from Source

### Prerequisites

#### All Platforms
- **Node.js** >= 16.0.0
- **Python** 3.x (for node-gyp)
- **node-gyp** build tools

#### Windows
- **Visual Studio 2017 or later** (with C++ build tools)
  - Install via Visual Studio Installer with "Desktop development with C++" workload
  - Or install standalone: `npm install --global windows-build-tools`
- **Windows SDK** (usually included with Visual Studio)

#### macOS
- **Xcode Command Line Tools**
  ```bash
  xcode-select --install
  ```
- **macOS SDK** (included with Xcode)

#### Linux
- **GCC or Clang** compiler
  ```bash
  # Ubuntu/Debian
  sudo apt-get install build-essential
  
  # Fedora/RHEL
  sudo dnf groupinstall "Development Tools"
  
  # Arch
  sudo pacman -S base-devel
  ```

### GPU Driver Requirements

For the addon to retrieve GPU information, appropriate drivers must be installed:

#### NVIDIA GPUs
- **Windows/Linux**: NVIDIA GeForce or CUDA drivers
  - Download from: https://www.nvidia.com/download/index.aspx
  - Includes NVML (NVIDIA Management Library)
- **macOS**: Limited support (NVIDIA dropped macOS support after High Sierra)

#### AMD GPUs
- **Windows**: AMD Adrenalin drivers
  - Download from: https://www.amd.com/en/support
  - Includes ADLX SDK support (used by this addon)
- **Linux**: Mesa drivers or AMD proprietary drivers (AMDGPU-PRO)
  ```bash
  # Ubuntu/Debian
  sudo apt-get install mesa-utils
  ```
- **macOS**: Built-in AMD drivers (Metal framework)

#### Intel GPUs
- **Windows**: Intel Graphics drivers
  - Download from: https://www.intel.com/content/www/us/en/download-center/home.html
- **Linux**: Mesa drivers (usually pre-installed)
- **macOS**: Built-in Intel drivers

### Build Commands

```bash
# Install dependencies and build
npm install

# Or build manually
npm run build

# Clean build artifacts
npm run clean

# Test the addon
npm test
```

### Build Process

The build process:
1. Downloads and configures `node-addon-api` headers
2. Compiles C/C++ source files using node-gyp
3. Links against platform-specific libraries:
   - **Windows**: `dxgi.lib`, `dxguid.lib`, `setupapi.lib`
   - **macOS**: `IOKit`, `CoreFoundation`, `Metal` frameworks
   - **Linux**: Standard system libraries

### Troubleshooting Build Issues

#### Windows
- **Error: Cannot find Python**: Install Python 3.x and add to PATH
- **Error: Cannot find Visual Studio**: Run `npm install --global windows-build-tools`
- **Missing Windows SDK**: Install via Visual Studio Installer

#### macOS
- **Missing Xcode tools**: Run `xcode-select --install`
- **Permission denied**: Try `sudo npm install` (not recommended) or fix npm permissions

#### Linux
- **Missing compiler**: Install build-essential or equivalent
- **Missing headers**: Install `linux-headers-$(uname -r)`

### Development Build

For development with debug symbols:
```bash
npm run build -- --debug
```

## Requirements

- Node.js >= 16.0.0
- C++11 compatible compiler
- Python (for node-gyp)
- GPU drivers installed (see GPU Driver Requirements above)

## Third-Party Components

This project uses the following third-party components:

- **ADLX (AMD Display Library eXtension)** - Copyright © 2021-2025 Advanced Micro Devices, Inc.
  - Located in: `src/includes/adlx/`
  - Used for AMD GPU monitoring on Windows
  - License: Proprietary (All rights reserved)
  - Note: ADLX headers are used under AMD's SDK license for interfacing with AMD drivers

The main project code (excluding third-party components) is licensed under GPL-3.0.

## License

GPL-3.0-only

**Note:** The ADLX SDK components in `src/includes/adlx/` are proprietary and copyrighted by Advanced Micro Devices, Inc. These components are used to interface with AMD's graphics drivers and are not covered by the GPL-3.0 license.
