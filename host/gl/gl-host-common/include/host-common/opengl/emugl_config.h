// Copyright 2020 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "aemu/base/c_header.h"
#include "aemu/base/export.h"
// #include "android/skin/winsys.h"

#include <stdbool.h>
#include <stdint.h>

ANDROID_BEGIN_HEADER

// List of values describing how EGL/GLES emulation should work in a given
// Android virtual device.
//
// kAndroidGlesEmulationOff
//    Means there is no GPU emulation, equivalent to "-gpu off" and instructs
//    the guest system to use its old GLES 1.x software renderer.
//
// kAndroidGlesEmulationHost
//    Means Host GPU emulation is being used. All EGL/GLES commands are
//    sent to the host GPU or CPU through a simple wire protocol. This
//    corresponds to "-gpu host" and "-gpu mesa".
//
// kAndroidGlesEmulationGuest
//    Means a guest GLES 2.x library (e.g. SwiftShader) is being used in
//    the guest. This should only be used with accelerated emulation, or
//    results will be very very slow.
typedef enum {
    kAndroidGlesEmulationOff = 0,
    kAndroidGlesEmulationHost,
    kAndroidGlesEmulationGuest,
} AndroidGlesEmulationMode;
// A small structure used to model the EmuGL configuration
// to use.
// |enabled| is true if GPU emulation is enabled, false otherwise.
// |backend| contains the name of the backend to use, if |enabled|
// is true.
// |status| is a string used to report error or the current status
// of EmuGL emulation.
typedef struct {
    bool enabled;
    bool use_backend;
    int bitness;
    char backend[64];
    char status[256];
    bool use_host_vulkan;
} EmuglConfig;

// Check whether or not the host GPU is blacklisted. If so, fall back
// to software rendering.
bool isHostGpuBlacklisted();

typedef struct {
    char* make;
    char* model;
    char* device_id;
    char* revision_id;
    char* version;
    char* renderer;
} emugl_host_gpu_props;

typedef struct {
    int num_gpus;
    emugl_host_gpu_props* props;
} emugl_host_gpu_prop_list;

// Get a description of host GPU properties.
// Need to free after use.
emugl_host_gpu_prop_list emuglConfig_get_host_gpu_props();

// Enum tracking all current available renderer backends
// for the emulator.
typedef enum SelectedRenderer {
    SELECTED_RENDERER_UNKNOWN = 0,
    SELECTED_RENDERER_HOST = 1,
    SELECTED_RENDERER_OFF = 2,
    SELECTED_RENDERER_GUEST = 3,
    SELECTED_RENDERER_MESA = 4,
    SELECTED_RENDERER_SWIFTSHADER = 5,
    SELECTED_RENDERER_ANGLE = 6, // ANGLE D3D11 with D3D9 fallback
    SELECTED_RENDERER_ANGLE9 = 7, // ANGLE forced to D3D9
    SELECTED_RENDERER_SWIFTSHADER_INDIRECT = 8,
    SELECTED_RENDERER_ANGLE_INDIRECT = 9,
    SELECTED_RENDERER_ANGLE9_INDIRECT = 10,
    SELECTED_RENDERER_ERROR = 255,
} SelectedRenderer;

enum GrallocImplementation { MINIGBM, GOLDFISH_GRALLOC };

// Returns SelectedRenderer value the selected gpu mode.
// Assumes that the -gpu command line option
// has been taken into account already.
SelectedRenderer emuglConfig_get_renderer(const char* gpu_mode);

// Returns the renderer that is active, after config is done.
SelectedRenderer emuglConfig_get_current_renderer();

// Returns the '-gpu <mode>' option. If '-gpu <mode>' option is NULL, returns
// the hw.gpu.mode hardware property.
const char* emuglConfig_get_user_gpu_option();

void emuglConfig_get_vulkan_hardware_gpu(char** vendor, int* major, int* minor,
        int* patch, uint64_t* deviceMemBytes);

// Returns a string representation of the renderer enum. Return value is a
// static constant string, it is NOT heap-allocated.
const char* emuglConfig_renderer_to_string(SelectedRenderer renderer);

// Returns if the current renderer supports snapshot.
bool emuglConfig_current_renderer_supports_snapshot();

void free_emugl_host_gpu_props(emugl_host_gpu_prop_list props);

// Initialize an EmuglConfig instance based on the AVD's hardware properties
// and the command-line -gpu option, if any.
//
// |config| is the instance to initialize.
// |gpu_enabled| is the value of the hw.gpu.enabled hardware property.
// |gpu_mode| is the value of the hw.gpu.mode hardware property.
// |gpu_option| is the value of the '-gpu <mode>' option, or NULL.
// |bitness| is the host bitness (0, 32 or 64).
// |no_window| is true if the '-no-window' emulator flag was used.
// |blacklisted| is true if the GPU driver is on the list of
// crashy GPU drivers.
// |use_host_vulkan| is true if the '-use-host-vulkan' emulator flag was used.
//
// Returns true on success, or false if there was an error (e.g. bad
// mode or option value), in which case the |status| field will contain
// a small error message.
AEMU_EXPORT bool emuglConfig_init(EmuglConfig* config,
                                  bool gpu_enabled,
                                  const char* gpu_mode,
                                  const char* gpu_option,
                                  int bitness,
                                  bool no_window,
                                  bool blacklisted,
                                  bool google_apis,
                                  int uiPreferredBackend,
                                  bool use_host_vulkan);

// Setup GPU emulation according to a given |backend|.
// |bitness| is the host bitness, and can be 0 (autodetect), 32 or 64.
AEMU_EXPORT void emuglConfig_setupEnv(const EmuglConfig* config);

ANDROID_END_HEADER
