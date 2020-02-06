if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(X86 TRUE)
endif()

file(GLOB CORE_SRC
    sky/src/core/*.cpp
    sky/src/core/*.h
)

file(GLOB CONSOLE_SRC
    sky/src/console/*.cpp
    sky/src/console/*.h
)

file(GLOB COMMON_SRC
    sky/src/common/*.cpp
    sky/src/common/*.h
)

file(GLOB PLATFORM_SRC
	sky/src/platform/*.cpp
	sky/src/platform/*.mm
    sky/src/platform/*.h
)

file(GLOB GRAPHICS_SRC
    sky/src/graphics/*.cpp
    sky/src/graphics/*.h
)

file(GLOB RENDERER_SRC
	sky/src/renderer/*.cpp
	sky/src/renderer/*.mm
    sky/src/renderer/*.h
)

file(GLOB RENDERER_D3D11_SRC
	sky/src/renderer/d3d11/*.cpp
	sky/src/renderer/d3d11/*.mm
    sky/src/renderer/d3d11/*.h
)

file(GLOB RENDERER_OPENGL_SRC
	sky/src/renderer/opengl/*.cpp
	sky/src/renderer/opengl/*.mm
    sky/src/renderer/opengl/*.h
)

file(GLOB AUDIO_SRC
	sky/src/audio/*.cpp
	sky/src/audio/*.h
)

file(GLOB SCENE_SRC
    sky/src/scene/*.cpp
    sky/src/scene/*.h
)

file(GLOB SHARED_SRC
    sky/src/shared/*.cpp
    sky/src/shared/*.h
)

file(GLOB NETWORK_SRC
    sky/src/network/*.cpp
    sky/src/network/*.h
)

source_group("core" FILES ${CORE_SRC})
source_group("console" FILES ${CONSOLE_SRC})
source_group("common" FILES ${COMMON_SRC})
source_group("platform" FILES ${PLATFORM_SRC})
source_group("graphics" FILES ${GRAPHICS_SRC})
source_group("renderer" FILES ${RENDERER_SRC})
source_group("renderer\\d3d11" FILES ${RENDERER_D3D11_SRC})
source_group("renderer\\opengl" FILES ${RENDERER_OPENGL_SRC})
source_group("audio" FILES ${AUDIO_SRC})
source_group("scene" FILES ${SCENE_SRC})
source_group("shared" FILES ${SHARED_SRC})
source_group("network" FILES ${NETWORK_SRC})

include_directories(sky/src)
add_library(sky STATIC 
	${CORE_SRC}
	${CONSOLE_SRC}
	${COMMON_SRC}
	${PLATFORM_SRC}
	${GRAPHICS_SRC}
	${RENDERER_SRC}
	${RENDERER_D3D11_SRC}
	${RENDERER_OPENGL_SRC}
	${AUDIO_SRC}
	${SCENE_SRC}
	${SHARED_SRC}
	${NETWORK_SRC}
)
target_link_libraries(${PROJECT_NAME} sky)

if(APPLE)
	set(OBJC_SOURCES
		sky/src/renderer/system_gl.cpp
		sky/src/platform/asset.cpp
	)
	set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fembed-bitcode")
endif()

if(WIN32)
	add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
endif()

# glm

target_compile_definitions(sky PRIVATE -DGLM_ENABLE_EXPERIMENTAL)
include_directories(sky/lib/glm)

# stb

include_directories(sky/lib/stb)

# glew

file(GLOB GLEW_SRC 
	sky/lib/glew/src/glew.c
    sky/lib/glew/include/GL/glew.h
)
include_directories(sky/lib/glew/include)
add_library(glew STATIC ${GLEW_SRC})
target_link_libraries(${PROJECT_NAME} glew)
target_compile_definitions(glew PRIVATE -DGLEW_STATIC)

# imgui

file(GLOB IMGUI_SRC 
	sky/lib/imgui/*.cpp 
	sky/lib/imgui/*.h
)	
include_directories(sky/lib/imgui)
add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} imgui)

# tinygltf

include_directories(sky/lib/tinygltf)

# tinyobjloader

include_directories(sky/lib/tinyobjloader)

# tinyutf8

file(GLOB TINYUTF8_SRC
	sky/lib/tinyutf8/*.cpp
	sky/lib/tinyutf8/*.h
	sky/lib/tinyutf8/*.hpp
)
include_directories(sky/lib/tinyutf8)
add_library(tinyutf8 STATIC ${TINYUTF8_SRC})
target_link_libraries(${PROJECT_NAME} tinyutf8)

# fmod

include_directories(sky/lib/fmod/include)
if(WIN32)
	if(X64)
		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmodL_vc.lib)
		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmod_vc.lib)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmodL.dll">
			$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmod.dll">
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	else()
		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmodL_vc.lib)
		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmod_vc.lib)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmodL.dll">
			$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmod.dll">
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	endif()
endif()

if(APPLE)
	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmodL_iphonesimulator.a)
	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmodL_iphoneos.a)
#	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmod_iphonesimulator.a)
#	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmod_iphoneos.a)
endif()

# rectpack2D

include_directories(sky/lib/rectpack2D/include)

# nlohmann_json

include_directories(sky/lib/nlohmann_json/include)

# fmt

file(GLOB FMT_SRC
	sky/lib/fmt/src/*.cc
)
include_directories(sky/lib/fmt/include)
add_library(fmt STATIC ${FMT_SRC})
target_link_libraries(${PROJECT_NAME} fmt)