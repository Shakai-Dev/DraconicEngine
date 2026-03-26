set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_subdirectory(bgfx.cmake)

# Prevent module scanning for bgfx & it's deps
set_target_properties(bgfx PROPERTIES CXX_SCAN_FOR_MODULES OFF)

if(TARGET bx)
    set_target_properties(bx PROPERTIES CXX_SCAN_FOR_MODULES OFF)
endif()

if(TARGET bimg)
    set_target_properties(bimg PROPERTIES CXX_SCAN_FOR_MODULES OFF)
endif()
