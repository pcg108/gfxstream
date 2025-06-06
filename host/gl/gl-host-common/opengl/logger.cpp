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

#include "host-common/opengl/logger.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <algorithm>
#include <fstream>
#include <string>

#include "aemu/base/files/PathUtils.h"
#include "aemu/base/msvc.h"
#include "aemu/base/synchronization/Lock.h"

#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <vector>

using android::base::AutoLock;
using android::base::Lock;
using android::base::PathUtils;

// The purpose of the OpenGL logger is to log
// information about such things as EGL initialization
// and possibly miscellanous OpenGL commands,
// in order to get a better idea of what is going on
// in crash reports.

// The OpenGLLogger implementation's initialization method
// by default uses the crash reporter's data directory.

static const int kBufferLen = 2048;

typedef std::pair<uint64_t, std::string> TimestampedLogEntry;

class OpenGLLogger {
   public:
    OpenGLLogger() = default;
    OpenGLLogger(const char* filename);
    void stop();

    // Coarse log: Call this infrequently.
    void writeCoarse(const char* str);

    // Fine log: When we want to log very frequent events.
    // Fine logs can be toggled on/off.
    void writeFineTimestamped(const char* str);

    void setLoggerFlags(AndroidOpenglLoggerFlags flags);
    bool isFineLogging() const;

    static OpenGLLogger* get();

   private:
    void writeFineLocked(uint64_t time, const char* str);
    void stopFineLogLocked();

    Lock mLock;
    AndroidOpenglLoggerFlags mLoggerFlags = OPENGL_LOGGER_NONE;
    uint64_t mPrevTimeUs = 0;
    std::string mFileName;
    std::ofstream mFileHandle;
    std::string mFineLogFileName;
    std::ofstream mFineLogFileHandle;
    std::vector<TimestampedLogEntry> mFineLog;
    DISALLOW_COPY_ASSIGN_AND_MOVE(OpenGLLogger);
};

static OpenGLLogger* sOpenGLLogger() {
    static OpenGLLogger sLogger;
    return &sLogger;
}

OpenGLLogger* OpenGLLogger::get() { return sOpenGLLogger(); }


OpenGLLogger::OpenGLLogger(const char* filename) : mFileName(filename) {
    mFileHandle.open(mFileName, std::ios::app);
}

void OpenGLLogger::writeCoarse(const char* str) {
    AutoLock lock(mLock);
    if (mLoggerFlags & OPENGL_LOGGER_PRINT_TO_STDOUT) {
        printf("%s\n", str);
    }
    if (mFileHandle) {
        mFileHandle << str << std::endl;
    }
}

void OpenGLLogger::stop() {
    AutoLock lock(mLock);
    stopFineLogLocked();
    mFileHandle.close();
}

void OpenGLLogger::writeFineLocked(uint64_t time, const char* str) {
    if (mLoggerFlags & OPENGL_LOGGER_PRINT_TO_STDOUT) {
        printf("%s", str);
    } else {
        mFineLog.emplace_back(time, str);
    }
}

void OpenGLLogger::writeFineTimestamped(const char* str) {
    if (mLoggerFlags & OPENGL_LOGGER_DO_FINE_LOGGING) {
        char buf[kBufferLen] = {};
        struct timeval tv;
        gettimeofday(&tv, NULL);

        uint64_t curr_micros = (tv.tv_usec) % 1000;
        uint64_t curr_millis = (tv.tv_usec / 1000) % 1000;
        uint64_t curr_secs = tv.tv_sec;
        uint64_t curr_us = tv.tv_sec * 1000000ULL + tv.tv_usec;
        snprintf(buf, sizeof(buf) - 1,
                 "time_us="
                 "%" PRIu64
                 " s "
                 "%" PRIu64
                 " ms "
                 "%" PRIu64
                 " us deltaUs "
                 "%" PRIu64 " | %s",
                 curr_secs, curr_millis, curr_micros, curr_us - mPrevTimeUs, str);
        AutoLock lock(mLock);
        writeFineLocked(curr_micros + 1000ULL * curr_millis + 1000ULL * 1000ULL * curr_secs, buf);
        mPrevTimeUs = curr_us;
    }
}

void OpenGLLogger::setLoggerFlags(AndroidOpenglLoggerFlags flags) {
    AutoLock lock(mLock);
    bool needStopFineLog = (mLoggerFlags & OPENGL_LOGGER_DO_FINE_LOGGING) &&
                           (!(flags & OPENGL_LOGGER_DO_FINE_LOGGING));

    if (needStopFineLog) {
        stopFineLogLocked();
    }

    mLoggerFlags = flags;
}

bool OpenGLLogger::isFineLogging() const {
    // For speed, we'll just let this read of mLoggerFlags race.
    return (mLoggerFlags & OPENGL_LOGGER_DO_FINE_LOGGING);
}

void OpenGLLogger::stopFineLogLocked() {
    // Only print message when fine-grained
    // logging is turned on.
    if (!mFineLog.empty()) {
        fprintf(stderr, "Writing fine-grained GL log to %s...", mFineLogFileName.c_str());
    }

    // Sort log entries according to their timestamps.
    // This is because the log entries might arrive
    // out of order.
    std::sort(mFineLog.begin(), mFineLog.end(),
              [](const TimestampedLogEntry& x, const TimestampedLogEntry& y) {
                  return x.first < y.first;
              });

    if (mFineLogFileHandle) {
        for (const auto& entry : mFineLog) {
            // The fine log does not print newlines
            // as it is used with the opengl debug
            // printout in emugl, which adds
            // newlines of its own.
            mFineLogFileHandle << entry.second;
        }
        mFineLogFileHandle.close();
        if (!mFineLog.empty()) {
            fprintf(stderr, "done\n");
        }
    }
    mFineLog.clear();
}

// C interface

void android_init_opengl_logger() { OpenGLLogger::get(); }

void android_opengl_logger_set_flags(AndroidOpenglLoggerFlags flags) {
    OpenGLLogger::get()->setLoggerFlags(flags);
}

void android_opengl_logger_write(char severity, const char* file, unsigned int line,
                                 int64_t timestamp_us, const char* message) {
    char buf[kBufferLen] = {};
    auto gl_log = OpenGLLogger::get();

    if (severity == 'V' || severity == 'D') {
        // Not logging details if it is not requested
        if (!gl_log->isFineLogging()) {
            return;
        }

        snprintf(buf, sizeof(buf) - 1, "%c %s:%d %s", severity, file, line, message);
        gl_log->writeFineTimestamped(buf);
        return;
    }

    // Other log levels..
    snprintf(buf, sizeof(buf) - 1, "%c %s:%d %s", severity, file, line, message);
    gl_log->writeCoarse(buf);
}

void android_stop_opengl_logger() {
    OpenGLLogger* gl_log = OpenGLLogger::get();
    gl_log->stop();
}
