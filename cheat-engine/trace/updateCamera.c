/*
 * updateCamera.c - High-level C equivalent of Halo MCC camera update system
 * 
 * Reconstructed from assembly analysis of halo1.dll+B14380
 * This represents the camera processing loop that handles multiple camera instances
 * with timing, smoothing, and state management.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ========================================================================
//                            DATA STRUCTURES  
// ========================================================================

// Camera function pointer type for camera-specific update callbacks
typedef void (*CameraUpdateFunc)(void* cameraData, void* params, void* outputBuffer);

// Camera timing and state data (inferred from assembly)
typedef struct {
    float deltaTime;            // Previous frame delta time
    float timingThreshold;      // Timing threshold for updates
    uint32_t stateFlags;        // Camera state and mode flags
    uint32_t reserved1;
    uint32_t reserved2;
    uint8_t mode1;              // Camera mode flag 1
    uint8_t mode2;              // Camera mode flag 2
    uint8_t padding[2];
} CameraTimingData;

// Main camera structure (248 bytes total - 0xF8)
typedef struct {
    uint32_t flags;                     // Camera flags and state bits
    uint32_t cameraId;                  // Camera identifier
    CameraUpdateFunc updateFunc;        // Function pointer for camera updates
    void* cameraSpecificData;           // Pointer to camera-specific data structure
    CameraTimingData timing;            // Timing and state data
    uint8_t reserved[200];              // Remaining camera data (simplified)
} CameraInstance;

// Processed camera output data (104 bytes - 0x68)
typedef struct {
    uint32_t activeFlag;        // Bit 0 = camera active
    uint32_t reserved1[3];
    float matrix[12];           // Camera transformation matrix (48 bytes)
    float position[3];          // Camera position
    float rotation[4];          // Camera rotation (quaternion)
    uint8_t additionalData[32]; // Additional processed camera data
} CameraOutputData;

// ========================================================================
//                           GLOBAL CAMERA DATA
// ========================================================================

// Camera system global state (based on memory addresses from assembly)
static struct {
    float currentDeltaTime;         // [halo1.dll+2D9B960] - Current frame delta time
    bool cameraSystemActive;        // [halo1.dll+1B7D4E8] - Camera system state flag  
    uint8_t cameraStateFlag;        // [halo1.dll+2D9B966] - Temporary camera state
    
    // Camera arrays (based on assembly memory layout)
    CameraInstance* cameraArray;           // [halo1.dll+2D9B970] - Main camera array
    void* cameraDataArray;                 // [halo1.dll+2D9B978] - Camera-specific data
    CameraOutputData* processedCameraData; // [halo1.dll+2D9B9C0] - Processed camera output
    CameraUpdateFunc* cameraFuncPointers;  // [halo1.dll+02EA3280] - Camera function pointer array
} g_cameraSystem;

// ========================================================================
//                          CAMERA SYSTEM FUNCTIONS
// ========================================================================

// Forward declarations for camera system functions (from assembly calls)
extern bool initializeCameraSystem(void);                    // halo1.dll+ADC658
extern int getNextActiveCameraIndex(int currentIndex);       // halo1.dll+AE498C  
extern void processCameraSpecific1(int cameraIndex, void* params); // halo1.dll+B1550C
extern void processCameraSpecific2(int cameraIndex, uint8_t flag1, uint8_t flag2); // halo1.dll+B145B4
extern void finalizeCameraProcessing(int cameraIndex);       // halo1.dll+B15ED0

// Camera function addresses for special handling
static const uintptr_t DEFAULT_CAMERA_FUNC = 0x1220840264ULL;  // halo1.dll+BE4124
static const uintptr_t SPECIAL_CAMERA_FUNC = 0x1239190348ULL;  // halo1.dll+C5263C

// Maximum delta time constant (0.2 seconds)
static const float MAX_DELTA_TIME = 0.2f;

// ========================================================================
//                         MAIN CAMERA UPDATE FUNCTION
// ========================================================================

void updateCamera(float deltaTime) {
    // Store delta time globally
    g_cameraSystem.currentDeltaTime = deltaTime;
    
    // Initialize camera system and check state
    bool systemReady = initializeCameraSystem();
    g_cameraSystem.cameraSystemActive = !systemReady;
    
    // Get first active camera index (-1 means iterate through all)
    int cameraIndex = getNextActiveCameraIndex(-1);
    
    // Exit if no cameras to process
    if (cameraIndex == -1) {
        return;
    }
    
    // ====================================================================
    //                    MAIN CAMERA PROCESSING LOOP
    // ====================================================================
    
    do {
        // Calculate camera data offset (each camera is 248 bytes)
        CameraInstance* currentCamera = &g_cameraSystem.cameraArray[cameraIndex];
        
        // Clear camera state flags
        currentCamera->flags &= 0xFFFE; // Clear bit 0
        
        // Process camera-specific functions
        uint8_t tempParams = 0;
        processCameraSpecific1(cameraIndex, &tempParams);
        
        g_cameraSystem.cameraStateFlag = 0;
        processCameraSpecific2(cameraIndex, tempParams, g_cameraSystem.cameraStateFlag);
        g_cameraSystem.cameraStateFlag = 0;
        
        // Initialize output buffer
        CameraOutputData outputBuffer;
        memset(&outputBuffer, 0, sizeof(outputBuffer));
        
        // ----------------------------------------------------------------
        //                    CAMERA FUNCTION HANDLING  
        // ----------------------------------------------------------------
        
        CameraUpdateFunc cameraFunc = currentCamera->updateFunc;
        g_cameraSystem.cameraFuncPointers[cameraIndex] = cameraFunc;
        
        if (cameraFunc != NULL) {
            // Handle special camera function types
            if ((uintptr_t)cameraFunc == DEFAULT_CAMERA_FUNC) {
                // For default camera, only process if it's the currently active camera
                int activeCameraIndex = getNextActiveCameraIndex(-1);
                if (cameraIndex != activeCameraIndex) {
                    goto skip_camera_processing;
                }
            }
            
            // Call camera-specific update function
            cameraFunc(
                &g_cameraSystem.cameraDataArray[cameraIndex * 248], // Camera-specific data
                &tempParams,                                        // Parameters
                &outputBuffer                                       // Output buffer
            );
            
            // Reload function pointer (may have changed)
            cameraFunc = currentCamera->updateFunc;
            g_cameraSystem.cameraFuncPointers[cameraIndex] = cameraFunc;
        }
        
        skip_camera_processing:
        
        // Check if camera produced valid output
        if (!(outputBuffer.activeFlag & 0x01)) {
            // Camera is inactive - clear active flag in output array
            g_cameraSystem.processedCameraData[cameraIndex].activeFlag &= 0xFFFFFFFE;
            goto continue_camera_loop;
        }
        
        // ----------------------------------------------------------------
        //                 CAMERA TIMING AND SMOOTHING LOGIC
        // ----------------------------------------------------------------
        
        float previousDeltaTime = currentCamera->timing.deltaTime;
        
        // Check for first frame (delta time == 0.0) or NaN
        if (previousDeltaTime == 0.0f || previousDeltaTime != previousDeltaTime) {
            goto copy_camera_data; // Skip timing processing on first frame
        }
        
        // Clamp delta time to maximum (0.2 seconds)
        if (previousDeltaTime > MAX_DELTA_TIME) {
            // Special handling for specific camera types
            if ((uintptr_t)cameraFunc == SPECIAL_CAMERA_FUNC) {
                // Reset camera state for special camera type
                currentCamera->timing.stateFlags = 0;
                currentCamera->timing.reserved1 = 0;
                currentCamera->timing.reserved2 = 0;
                currentCamera->timing.mode1 = 3;
                currentCamera->timing.mode2 = 3;
                previousDeltaTime = 0.0f;
            }
        } else {
            // Update timing threshold if needed
            if (currentCamera->timing.timingThreshold < previousDeltaTime) {
                currentCamera->timing.timingThreshold = previousDeltaTime;
            }
        }
        
        // Calculate processed delta time (previous - current, clamped to >= 0)
        float processedDelta = previousDeltaTime - deltaTime;
        if (processedDelta < 0.0f) {
            processedDelta = 0.0f;
        }
        
        // Store processed delta time back to camera
        currentCamera->timing.deltaTime = processedDelta;
        
        // ----------------------------------------------------------------
        //                COPY CAMERA DATA TO OUTPUT ARRAY
        // ----------------------------------------------------------------
        
        copy_camera_data:
        // Copy processed camera data to main camera output array
        memcpy(&g_cameraSystem.processedCameraData[cameraIndex], 
               &outputBuffer, 
               sizeof(CameraOutputData));
        
        continue_camera_loop:
        
        // ----------------------------------------------------------------
        //                      CONTINUE TO NEXT CAMERA
        // ----------------------------------------------------------------
        
        // Finalize current camera processing
        finalizeCameraProcessing(cameraIndex);
        
        // Get next active camera index
        cameraIndex = getNextActiveCameraIndex(cameraIndex);
        
    } while (cameraIndex != -1); // Continue until no more cameras
}

// ========================================================================
//                           HELPER FUNCTIONS
// ========================================================================

// Initialize camera system data structures (called once at startup)
bool initCameraSystem(CameraInstance* cameras, int maxCameras) {
    g_cameraSystem.cameraArray = cameras;
    g_cameraSystem.currentDeltaTime = 0.0f;
    g_cameraSystem.cameraSystemActive = false;
    g_cameraSystem.cameraStateFlag = 0;
    
    // Allocate camera data arrays
    g_cameraSystem.cameraDataArray = calloc(maxCameras, 248);  // Camera-specific data
    g_cameraSystem.processedCameraData = calloc(maxCameras, sizeof(CameraOutputData));
    g_cameraSystem.cameraFuncPointers = calloc(maxCameras, sizeof(CameraUpdateFunc));
    
    return (g_cameraSystem.cameraDataArray != NULL && 
            g_cameraSystem.processedCameraData != NULL &&
            g_cameraSystem.cameraFuncPointers != NULL);
}

// Cleanup camera system resources
void shutdownCameraSystem(void) {
    free(g_cameraSystem.cameraDataArray);
    free(g_cameraSystem.processedCameraData);
    free(g_cameraSystem.cameraFuncPointers);
    
    memset(&g_cameraSystem, 0, sizeof(g_cameraSystem));
}