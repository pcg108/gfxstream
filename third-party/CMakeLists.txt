# Configure and add `astc-encoder`, if needed
if (ASTC_CPU_DECODING)
    set(DECOMPRESSOR ON)   # Disable compression code
    set(CLI OFF)           # Disable the command line interface

    # Compile with the AVX2 instruction set. This is the fastest option available on x86_64.
    # At run time, if the CPU doesn't support AVX2, the library will simply return an error status
    # during initialization and we will fall back on the compute shader to decompress ASTC textures.
    #
    # In the future, we should define `ASTCENC_DYNAMIC_LIBRARY` and build multiple versions of the
    # library for each SIMD architecture, and dynamically load the fastest one at run time.
    # See also: https://github.com/ARM-software/astc-encoder/issues/79
    set(ISA_AVX2 ON)

    add_subdirectory(astc-encoder)
endif ()

# Enable perfetto in CMake if needed
# add_subdirectory(perfetto-tracing-only)
add_subdirectory(renderdoc)
add_subdirectory(stb)

if(ENABLE_VKCEREAL_TESTS)
    set(AEMU_BASE_USE_LZ4 ON CACHE BOOL "" FORCE)
endif()

if(NOT TARGET lz4_static AND AEMU_BASE_USE_LZ4)
    set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
        set(LZ4_PATH ${PROJECT_SOURCE_DIR}/../../../external/lz4/build/cmake)
        if(EXISTS ${LZ4_PATH})
            add_subdirectory(${LZ4_PATH} lz4)
        endif()
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(PkgConfig REQUIRED)
        pkg_search_module(lz4 REQUIRED IMPORTED_TARGET GLOBAL liblz4)
        add_library(lz4_static ALIAS PkgConfig::lz4)
    endif()
endif()

if(NOT TARGET gtest_main AND ENABLE_VKCEREAL_TESTS)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
        set(GOOGLETEST_PATH ${PROJECT_SOURCE_DIR}/../../../external/googletest)
        if(EXISTS ${GOOGLETEST_PATH})
            add_subdirectory(${GOOGLETEST_PATH} googletest)
        endif()
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(PkgConfig REQUIRED)
        pkg_search_module(gtest REQUIRED IMPORTED_TARGET GLOBAL gtest)
        pkg_search_module(gtest_main REQUIRED IMPORTED_TARGET GLOBAL gtest_main)
        pkg_search_module(gmock REQUIRED IMPORTED_TARGET GLOBAL gmock)
        pkg_search_module(gmock_main REQUIRED IMPORTED_TARGET GLOBAL gmock_main)
        add_library(gtest ALIAS PkgConfig::gtest)
        add_library(gtest_main ALIAS PkgConfig::gtest_main)
        add_library(gmock ALIAS PkgConfig::gmock)
        add_library(gmock_main ALIAS PkgConfig::gmock_main)
    endif()
endif()

if(NOT TARGET aemu_common)
    set(AEMU_COMMON_BUILD_CONFIG "gfxstream" CACHE STRING "")
    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
	#set(AEMU_COMMON_PATH ${PROJECT_SOURCE_DIR}/../../../hardware/google/aemu)
	set(AEMU_COMMON_PATH /scratch/prashanth/virt-gpu/aemu)
	if(EXISTS ${AEMU_COMMON_PATH})
            add_subdirectory(${AEMU_COMMON_PATH} aemu_common)
        endif()
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(PkgConfig REQUIRED)
        set(AEMU_PC_FILES
            aemu_base
            aemu_logging
            aemu_host_common
            aemu_base_testing_support
            aemu_host_common_testing_support
            aemu_snapshot)
        foreach(PC_FILE IN LISTS AEMU_PC_FILES)
            pkg_search_module(${PC_FILE} REQUIRED IMPORTED_TARGET GLOBAL ${PC_FILE}>=0.0.0)
        endforeach()

        add_library(aemu-base.headers INTERFACE)
        target_include_directories(aemu-base.headers INTERFACE ${aemu_base_INCLUDE_DIRS})

        add_library(aemu-host-common.headers INTERFACE)
        target_include_directories(aemu-host-common.headers INTERFACE ${aemu_host_common_INCLUDE_DIRS})
        target_link_libraries(aemu-host-common.headers INTERFACE renderdoc gfxstream_vulkan_headers)

        add_library(gfxstream-snapshot.headers INTERFACE)
        target_include_directories(gfxstream-snapshot.headers INTERFACE ${aemu_snapshot_INCLUDE_DIRS})
        add_library(gfxstream-snapshot ALIAS PkgConfig::aemu_snapshot)

        add_library(logging-base INTERFACE)
        target_link_libraries(logging-base INTERFACE PkgConfig::aemu_logging)

        if (WIN32)
            set(aemu-base-platform-deps Shlwapi)
        else()
            set(aemu-base-platform-deps dl rt)
        endif()
        add_library(aemu-base INTERFACE)
        target_link_libraries(
            aemu-base
            INTERFACE
            PkgConfig::aemu_base
            logging-base
            ${aemu-base-platform-deps})
        if(TARGET lz4_static)
            target_link_libraries(aemu-base INTERFACE lz4_static)
        endif()
        if(TARGET perfetto-tracing-only)
            target_link_libraries(aemu-base INTERFACE perfetto-tracing-only)
        endif()
        add_library(aemu-host-common INTERFACE)
        target_link_libraries(aemu-host-common INTERFACE PkgConfig::aemu_host_common aemu-base)

        add_library(aemu-base-testing-support INTERFACE)
        target_link_libraries(
            aemu-base-testing-support
            INTERFACE
            PkgConfig::aemu_base_testing_support
            aemu-base
            gmock
            gtest)

        add_library(aemu-host-common-testing-support INTERFACE)
        target_link_libraries(
            aemu-host-common-testing-support
            INTERFACE
            PkgConfig::aemu_host_common_testing_support
            PkgConfig::aemu_host_common
            PkgConfig::aemu_base
            PkgConfig::aemu_logging
            gmock
            gtest)
        if(TARGET lz4_static)
            target_link_libraries(aemu-host-common-testing-support INTERFACE lz4_static)
        endif()

        if(TARGET perfetto-tracing-only)
            target_link_libraries(aemu-host-common-testing-support INTERFACE perfetto-tracing-only)
        endif()
        add_library(aemu_common INTERFACE)
        target_link_libraries(aemu_common INTERFACE aemu-base aemu-host-common)
        target_include_directories(aemu_common INTERFACE aemu-base.headers aemu-host-common.headers)
    endif()
endif()

if(NOT TARGET gfxstream_vulkan_headers)
    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
        set(GFXSTREAM_PROTOCOLS_PATH ${PROJECT_SOURCE_DIR}/../../../external/gfxstream-protocols)
        if(EXISTS ${GFXSTREAM_PROTOCOLS_PATH})
            add_subdirectory(${GFXSTREAM_PROTOCOLS_PATH} gfxstream-protocols)
        endif()
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(Vulkan 1.2.198 REQUIRED)
        add_library(gfxstream_vulkan_headers INTERFACE)
        target_include_directories(gfxstream_vulkan_headers INTERFACE ${Vulkan_INCLUDE_DIRS})
        target_compile_definitions(gfxstream_vulkan_headers
                                   INTERFACE VK_GFXSTREAM_STRUCTURE_TYPE_EXT)
    endif()
endif()

if(NOT TARGET flatbuffers)
    set(FLATBUFFERS_BUILD_TESTS OFF)
    set(FLATBUFFERS_BUILD_CPP17 ON)

    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
	    #set(FLATBUFFERS_PATH ${PROJECT_SOURCE_DIR}/../../../external/flatbuffers)
	set(FLATBUFFERS_PATH /scratch/prashanth/virt-gpu/flatbuffers)
	if(EXISTS ${FLATBUFFERS_PATH})
            add_subdirectory(${FLATBUFFERS_PATH} flatbuffers)
        endif()
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(PkgConfig REQUIRED)
        pkg_search_module(flatbuffers REQUIRED IMPORTED_TARGET GLOBAL flatbuffers>=2.0.6)
        add_library(flatbuffers ALIAS PkgConfig::flatbuffers)
        find_program(FLATC flatc NO_CACHE REQUIRED)
        add_executable(flatc IMPORTED GLOBAL)
        set_property(TARGET flatc PROPERTY IMPORTED_LOCATION ${FLATC})
    endif()
endif()

if (QNX) # QNX SDP provides EGL headers
    add_library(gfxstream_egl_headers INTERFACE)
    target_include_directories(gfxstream_egl_headers INTERFACE ${QNX_TARGET}/usr/include)
endif()

if(NOT TARGET gfxstream_egl_headers)
    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
        # ANGLE provides the EGL headers for us.
	# set(ANGLE_PATH ${PROJECT_SOURCE_DIR}/../../../external/angle)
	set(ANGLE_PATH /scratch/prashanth/virt-gpu/angle)
	if(NOT EXISTS ${ANGLE_PATH})
            message(FATAL_ERROR "ANGLE is not found.")
        endif()
        add_library(gfxstream_egl_headers INTERFACE)
        target_include_directories(gfxstream_egl_headers INTERFACE ${ANGLE_PATH}/include)
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(PkgConfig REQUIRED)
        pkg_search_module(egl REQUIRED IMPORTED_TARGET GLOBAL egl>=1.5)
        add_library(gfxstream_egl_headers ALIAS PkgConfig::egl)
    endif()
endif()

if(WITH_BENCHMARK)
    # Add Google's benchmarking framework
    set(BENCHMARK_ENABLE_EXCEPTIONS OFF)  # We disable exceptions in gfxstream
    set(BENCHMARK_ENABLE_TESTING OFF)     # Don't build the unit tests for the library, to save time
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)

    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
        set(GOOGLE_BENCHMARK_PATH ${PROJECT_SOURCE_DIR}/../../../external/google-benchmark)
        if(EXISTS ${GOOGLE_BENCHMARK_PATH})
            add_subdirectory(${GOOGLE_BENCHMARK_PATH} google-benchmark)
        endif()
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        message(FATAL_ERROR "Not implemented")
    endif()
endif()

if(NOT TARGET libdrm_headers)
    if(DEPENDENCY_RESOLUTION STREQUAL "AOSP")
	#set(LIBDRM_PATH ${PROJECT_SOURCE_DIR}/../../../external/libdrm)
	set(LIBDRM_PATH /scratch/prashanth/virt-gpu/drm)
	if(NOT EXISTS ${LIBDRM_PATH})
            message(FATAL_ERROR "LIBDRM is not found.")
        endif()
        add_library(libdrm_headers INTERFACE)
        target_include_directories(libdrm_headers INTERFACE ${LIBDRM_PATH}/include/drm)
    elseif(DEPENDENCY_RESOLUTION STREQUAL "SYSTEM")
        find_package(PkgConfig REQUIRED)
        pkg_search_module(drm REQUIRED IMPORTED_TARGET GLOBAL libdrm)
        add_library(libdrm_headers ALIAS PkgConfig::drm)
    endif()
endif()

if(NOT TARGET aemu_common)
    message(FATAL_ERROR "The dependency aemu_common not found")
endif()
if(NOT TARGET gfxstream_vulkan_headers)
    message(FATAL_ERROR "The dependency gfxstream_vulkan_headers not found")
endif()
if(NOT TARGET flatbuffers)
    message(FATAL_ERROR "The dependency flatbuffers not found.")
endif()
if(NOT TARGET libdrm_headers)
    message(FATAL_ERROR "The dependency libdrm_headers not found.")
endif()
