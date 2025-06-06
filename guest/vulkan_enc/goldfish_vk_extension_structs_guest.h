// Copyright (C) 2018 The Android Open Source Project
// Copyright (C) 2018 Google Inc.
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

// Autogenerated module goldfish_vk_extension_structs_guest
//
// (header) generated by scripts/genvk.py -registry ../../vulkan/registry/vk.xml -registryGfxstream
// xml/vk_gfxstream.xml cereal -o /tmp/
//
// Please do not modify directly;
// re-run gfxstream-protocols/scripts/generate-vulkan-sources.sh,
// or directly from Python by defining:
// VULKAN_REGISTRY_XML_DIR : Directory containing vk.xml
// VULKAN_REGISTRY_SCRIPTS_DIR : Directory containing genvk.py
// CEREAL_OUTPUT_DIR: Where to put the generated sources.
//
// python3 $VULKAN_REGISTRY_SCRIPTS_DIR/genvk.py -registry $VULKAN_REGISTRY_XML_DIR/vk.xml cereal -o
// $CEREAL_OUTPUT_DIR
//
#pragma once
#include <vulkan/vulkan.h>

#include "goldfish_vk_private_defs.h"
#include "vk_android_native_buffer_gfxstream.h"
#include "vk_platform_compat.h"
#include "vulkan_gfxstream.h"
// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer

namespace gfxstream {
namespace vk {

uint32_t goldfish_vk_struct_type(const void* structExtension);

size_t goldfish_vk_extension_struct_size(VkStructureType rootType, const void* structExtension);

size_t goldfish_vk_extension_struct_size_with_stream_features(uint32_t streamFeatures,
                                                              VkStructureType rootType,
                                                              const void* structExtension);

}  // namespace vk
}  // namespace gfxstream
