/**
 * Example usage of node-gpu
 * 
 * This demonstrates how to retrieve GPU information from the system.
 * Run with: node example.js
 */

const gpu = require('./index');

console.log('=== node-gpu Example ===\n');

try {
    // Get GPU count
    console.log('--- GPU Count ---');
    const gpuCount = gpu.getGpuCount();
    console.log('Number of GPUs:', gpuCount);
    console.log();

    if (gpuCount === 0) {
        console.log('No GPUs detected in the system.');
        process.exit(0);
    }

    // Get information for each GPU
    for (let i = 0; i < gpuCount; i++) {
        console.log(`--- GPU ${i} Information ---`);
        const gpuInfo = gpu.getGpuInfo(i);
        
        console.log('Index:', gpuInfo.index);
        console.log('Vendor:', gpuInfo.vendor);
        console.log('Name:', gpuInfo.name || 'N/A');
        console.log('UUID:', gpuInfo.uuid || 'N/A');
        console.log('PCI Bus ID:', gpuInfo.pciBusId || 'N/A');
        console.log();
        
        console.log('Memory:');
        console.log('  Total:', gpuInfo.memoryTotal ? `${gpuInfo.memoryTotal} MB` : 'N/A');
        console.log('  Used:', gpuInfo.memoryUsed ? `${gpuInfo.memoryUsed} MB` : 'N/A');
        console.log('  Free:', gpuInfo.memoryFree ? `${gpuInfo.memoryFree} MB` : 'N/A');
        console.log();
        
        console.log('Utilization:');
        console.log('  GPU:', gpuInfo.gpuUtilization !== undefined ? `${gpuInfo.gpuUtilization.toFixed(1)}%` : 'N/A');
        console.log('  Memory:', gpuInfo.memoryUtilization !== undefined ? `${gpuInfo.memoryUtilization.toFixed(1)}%` : 'N/A');
        console.log();
        
        console.log('Temperature:', gpuInfo.temperature !== undefined ? `${gpuInfo.temperature.toFixed(1)}°C` : 'N/A');
        console.log('Power Usage:', gpuInfo.powerUsage !== undefined ? `${gpuInfo.powerUsage.toFixed(1)} W` : 'N/A');
        console.log();
        
        console.log('Clocks:');
        console.log('  Core Clock:', gpuInfo.coreClock ? `${gpuInfo.coreClock} MHz` : 'N/A');
        console.log('  Memory Clock:', gpuInfo.memoryClock ? `${gpuInfo.memoryClock} MHz` : 'N/A');
        console.log();
        
        console.log('Fan Speed:', gpuInfo.fanSpeed !== undefined ? `${gpuInfo.fanSpeed.toFixed(1)}%` : 'N/A');
        console.log();
    }

    // Get all GPU information at once
    console.log('--- All GPU Information (Single Call) ---');
    const allGpuInfo = gpu.getAllGpuInfo();
    console.log(JSON.stringify(allGpuInfo, null, 2));

} catch (error) {
    console.error('Error:', error.message);
    console.error('\nNote: Some information may require elevated privileges (administrator/root) or specific GPU drivers.');
    process.exit(1);
}
