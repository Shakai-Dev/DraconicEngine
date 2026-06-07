add_library(meshoptimizer STATIC 
    src/meshoptimizer.h
    src/allocator.cpp
    src/clusterizer.cpp
    src/indexanalyzer.cpp
    src/indexcodec.cpp
    src/indexgenerator.cpp
    src/meshletcodec.cpp
    src/meshletutils.cpp
    src/opacitymap.cpp
    src/overdrawoptimizer.cpp
    src/partition.cpp
    src/quantization.cpp
    src/rasterizer.cpp
    src/simplifier.cpp
    src/spatialorder.cpp
    src/stripifier.cpp
    src/tangentspace.cpp
    src/vcacheoptimizer.cpp
    src/vertexcodec.cpp
    src/vertexfilter.cpp
    src/vfetchoptimizer.cpp
)

target_include_directories(meshoptimizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)