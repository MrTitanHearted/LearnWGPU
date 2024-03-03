set(WGPU_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(WGPU_LIBRARY_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)

if(WGPU_SHARED_LIBS)
    set(WGPU_LIBRARY wgpu_native)
else()
    set(WGPU_LIBRARY wgpu_native.so)
endif()

if(UNIX AND NOT APPLE)
    set(OS_LIBRARIES "-lm" "-ldl")
elseif(APPLE)
    set(OS_LIBRARIES "-framework CoreFoundation -framework QuartzCore -framework Metal")
endif()

add_library(wgpu INTERFACE IMPORTED)
add_library(WebGPU::WebGPU ALIAS wgpu)

set_target_properties(wgpu PROPERTIES INTERFACE_LINK_DIRECTORIES ${WGPU_LIBRARY_DIR})
target_include_directories(wgpu INTERFACE ${WGPU_INCLUDE_DIR})
target_link_libraries(wgpu INTERFACE ${WGPU_LIBRARY} ${OS_LIBRARIES})

install(FILES ${WGPU_LIBRARY} DESTINATION ${CMAKE_BINARY_DIR})