add_library(
    gfxstream-magma-server
    Decoder.cpp
    magma_dec/magma_dec.cpp
    magma_dec/magma_server_context.cpp
    ${gfxstream-magma-server-backend-sources})
target_compile_definitions(
    gfxstream-magma-server
    PRIVATE
    ${gfxstream-magma-server-backend-defines})
target_link_libraries(
    gfxstream-magma-server
    PRIVATE
    apigen-codec-common
    ${gfxstream-magma-server-backend-libs})
target_include_directories(
    gfxstream-magma-server
    PRIVATE
    ${GFXSTREAM_REPO_ROOT}
    ${GFXSTREAM_REPO_ROOT}/include
    ${GFXSTREAM_REPO_ROOT}/host
    ${GFXSTREAM_REPO_ROOT}/host/apigen-codec-common
    ${GFXSTREAM_REPO_ROOT}/host/magma
    ${GFXSTREAM_REPO_ROOT}/host/magma/magma_dec
    ${GFXSTREAM_REPO_ROOT}/third-party/fuchsia/magma/include
    ${GFXSTREAM_REPO_ROOT}/third-party/fuchsia/magma/include/lib)
