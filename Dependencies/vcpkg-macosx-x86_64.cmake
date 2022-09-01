set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES x86_64)

if("${PORT}" STREQUAL "libbson")
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DENABLE_EXTRA_ALIGNMENT=OFF")
endif()