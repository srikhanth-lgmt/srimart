# Custom vcpkg triplet for SudharshanMart production containers.
#
# Mirrors the community x64-linux triplet (static linkage) but builds a
# Release-only install tree. This lets the Docker build produce a fully
# self-contained binary that links every vcpkg dependency statically, so the
# runtime image needs no vcpkg shared objects at all.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Build only the Release configuration (skip the debug tree entirely).
set(VCPKG_BUILD_TYPE release)
