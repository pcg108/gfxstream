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

#include <stddef.h>

#include "aemu/base/c_header.h"
#include "aemu/base/export.h"
#include "host-common/multi_display_agent.h"
#include "host-common/vm_operations.h"
#include "host-common/window_agent.h"
#include "render-utils/virtio_gpu_ops.h"

#ifdef __cplusplus
#include "host-common/opengl/misc.h"
#include "render-utils/RenderLib.h"
#endif

#ifndef USING_ANDROID_BP
ANDROID_BEGIN_HEADER
#endif

/* A version of android_initOpenglesEmulation that is called from a library
 * that has static access to libOpenglRender. */
AEMU_EXPORT int android_prepareOpenglesEmulation(void);
AEMU_EXPORT int android_setOpenglesEmulation(void* renderLib, void* eglDispatch, void* glesv2Dispatch);

/* Call this function to initialize the hardware opengles emulation.
 * This function will abort if we can't find the corresponding host
 * libraries through dlopen() or equivalent.
 */
AEMU_EXPORT int android_initOpenglesEmulation(void);

/* Tries to start the renderer process. Returns 0 on success, -1 on error.
 * At the moment, this must be done before the VM starts. The onPost callback
 * may be NULL.
 *
 * width and height: the framebuffer dimensions that will be reported
 *                   to the guest display driver.
 * guestApiLevel: API level of guest image (23 for mnc, 24 for nyc, etc)
 */
AEMU_EXPORT int android_startOpenglesRenderer(int width, int height,
                                              bool isPhone, int guestApiLevel,
                                              const QAndroidVmOperations *vm_operations,
                                              const QAndroidEmulatorWindowAgent *window_agent,
                                              const QAndroidMultiDisplayAgent *multi_display_agent,
                                              const void* gfxstreamFeatures,
                                              int* glesMajorVersion_out,
                                              int* glesMinorVersion_out);

AEMU_EXPORT bool android_asyncReadbackSupported();

/* See the description in render_api.h. */
typedef void (*OnPostFunc)(void* context, uint32_t displayId, int width,
                           int height, int ydir, int format, int type,
                           unsigned char* pixels);
AEMU_EXPORT void android_setPostCallback(OnPostFunc onPost,
                             void* onPostContext,
                             bool useBgraReadback,
                             uint32_t displayId);

typedef void (*ReadPixelsFunc)(void* pixels, uint32_t bytes, uint32_t displayId);
AEMU_EXPORT ReadPixelsFunc android_getReadPixelsFunc();


typedef void (*FlushReadPixelPipeline)(int displayId);

/* Gets the function that can be used to make sure no
 * frames are left in the video producer pipeline.
 * This can result in a post callback.
 */
FlushReadPixelPipeline android_getFlushReadPixelPipeline();

/* Retrieve the Vendor/Renderer/Version strings describing the underlying GL
 * implementation. The call only works while the renderer is started.
 *
 * Expects |*vendor|, |*renderer| and |*version| to be NULL.
 *
 * On exit, sets |*vendor|, |*renderer| and |*version| to point to new
 * heap-allocated strings (that must be freed by the caller) which represent the
 * OpenGL hardware vendor name, driver name and version, respectively.
 * In case of error, |*vendor| etc. are set to NULL.
 */
AEMU_EXPORT void android_getOpenglesHardwareStrings(char** vendor,
                                                    char** renderer,
                                                    char** version);

AEMU_EXPORT int android_showOpenglesWindow(void* window,
                                           int wx,
                                           int wy,
                                           int ww,
                                           int wh,
                                           int fbw,
                                           int fbh,
                                           float dpr,
                                           float rotation,
                                           bool deleteExisting,
                                           bool hideWindow);

AEMU_EXPORT int android_hideOpenglesWindow(void);

AEMU_EXPORT void android_setOpenglesTranslation(float px, float py);

AEMU_EXPORT void android_setOpenglesScreenMask(int width, int height, const unsigned char* rgbaData);

AEMU_EXPORT void android_redrawOpenglesWindow(void);

AEMU_EXPORT bool android_hasGuestPostedAFrame(void);
AEMU_EXPORT void android_resetGuestPostedAFrame(void);

typedef bool (*ScreenshotFunc)(const char* dirname, uint32_t displayId);
AEMU_EXPORT void android_registerScreenshotFunc(ScreenshotFunc f);
AEMU_EXPORT bool android_screenShot(const char* dirname, uint32_t displayId);

/* Stop the renderer process */
EMUGL_COMMON_API void android_stopOpenglesRenderer(bool wait);

/* Finish all renderer work, deleting current
 * render threads. Renderer is allowed to get
 * new render threads after that. */
AEMU_EXPORT void android_finishOpenglesRenderer();

/* set to TRUE if you want to use fast GLES pipes, 0 if you want to
 * fallback to local TCP ones
 */
AEMU_EXPORT extern int  android_gles_fast_pipes;

// Notify the renderer that a guest graphics process is created or destroyed.
AEMU_EXPORT void android_onGuestGraphicsProcessCreate(uint64_t puid);
// TODO(kaiyili): rename this API to android_onGuestGraphicsProcessDestroy
AEMU_EXPORT void android_cleanupProcGLObjects(uint64_t puid);

AEMU_EXPORT void android_waitForOpenglesProcessCleanup();

#ifdef __cplusplus
namespace gfxstream {
class Renderer;
}

AEMU_EXPORT const gfxstream::RendererPtr& android_getOpenglesRenderer();
EMUGL_COMMON_API void android_setOpenglesRenderer(gfxstream::RendererPtr* renderer);
#endif

AEMU_EXPORT struct AndroidVirtioGpuOps* android_getVirtioGpuOps(void);

/* Get EGL/GLESv2 dispatch tables */
AEMU_EXPORT const void* android_getEGLDispatch();
AEMU_EXPORT const void* android_getGLESv2Dispatch();

/* Set vsync rate at runtime */
AEMU_EXPORT void android_setVsyncHz(int vsyncHz);

AEMU_EXPORT void android_setOpenglesDisplayConfigs(int configId, int w, int h,
                                                   int dpiX, int dpiY);
AEMU_EXPORT void android_setOpenglesDisplayActiveConfig(int configId);

#ifndef USING_ANDROID_BP
ANDROID_END_HEADER
#endif
