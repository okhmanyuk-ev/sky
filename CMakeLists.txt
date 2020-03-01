cmake_minimum_required(VERSION 3.8.0)
project(sky)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(X86 TRUE)
endif()

file(GLOB CORE_SRC
    src/core/*.cpp
    src/core/*.h
)

file(GLOB CONSOLE_SRC
    src/console/*.cpp
    src/console/*.h
)

file(GLOB COMMON_SRC
    src/common/*.cpp
    src/common/*.h
)

file(GLOB PLATFORM_SRC
	src/platform/*.cpp
	src/platform/*.mm
    src/platform/*.h
)

file(GLOB GRAPHICS_SRC
    src/graphics/*.cpp
    src/graphics/*.h
)

file(GLOB RENDERER_SRC
	src/renderer/*.cpp
	src/renderer/*.mm
    src/renderer/*.h
)

file (GLOB RENDERER_SHADERS_SRC
	src/renderer/shaders/*.cpp
    src/renderer/shaders/*.h
)

file(GLOB RENDERER_D3D11_SRC
	src/renderer/d3d11/*.cpp
	src/renderer/d3d11/*.mm
    src/renderer/d3d11/*.h
)

file(GLOB RENDERER_OPENGL_SRC
	src/renderer/opengl/*.cpp
	src/renderer/opengl/*.mm
    src/renderer/opengl/*.h
)

file(GLOB AUDIO_SRC
	src/audio/*.cpp
	src/audio/*.h
)

file(GLOB SCENE_SRC
    src/scene/*.cpp
    src/scene/*.h
)

file(GLOB SHARED_SRC
    src/shared/*.cpp
    src/shared/*.h
)

file(GLOB NETWORK_SRC
    src/network/*.cpp
    src/network/*.h
)

source_group("core" FILES ${CORE_SRC})
source_group("console" FILES ${CONSOLE_SRC})
source_group("common" FILES ${COMMON_SRC})
source_group("platform" FILES ${PLATFORM_SRC})
source_group("graphics" FILES ${GRAPHICS_SRC})
source_group("renderer" FILES ${RENDERER_SRC})
source_group("renderer\\d3d11" FILES ${RENDERER_D3D11_SRC})
source_group("renderer\\opengl" FILES ${RENDERER_OPENGL_SRC})
source_group("renderer\\shaders" FILES ${RENDERER_SHADERS_SRC})
source_group("audio" FILES ${AUDIO_SRC})
source_group("scene" FILES ${SCENE_SRC})
source_group("shared" FILES ${SHARED_SRC})
source_group("network" FILES ${NETWORK_SRC})

add_library(${PROJECT_NAME} STATIC
	${CORE_SRC}
	${CONSOLE_SRC}
	${COMMON_SRC}
	${PLATFORM_SRC}
	${GRAPHICS_SRC}
	${RENDERER_SRC}
	${RENDERER_SHADERS_SRC}
	${RENDERER_D3D11_SRC}
	${RENDERER_OPENGL_SRC}
	${AUDIO_SRC}
	${SCENE_SRC}
	${SHARED_SRC}
	${NETWORK_SRC}
)

if(APPLE)
	set(OBJC_SOURCES
		sky/src/renderer/system_gl.cpp
		sky/src/platform/asset.cpp
	)
	set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fembed-bitcode")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)

# glm

target_compile_definitions(${PROJECT_NAME} PUBLIC -DGLM_ENABLE_EXPERIMENTAL)
target_include_directories(${PROJECT_NAME} PUBLIC lib/glm)

# stb

target_include_directories(${PROJECT_NAME} PUBLIC lib/stb)

# rectpack2D

target_include_directories(${PROJECT_NAME} PUBLIC lib/rectpack2D/include)

# tinyutf8

file(GLOB TINYUTF8_SRC
	lib/tinyutf8/*.cpp
	lib/tinyutf8/*.h
	lib/tinyutf8/*.hpp
)
target_include_directories(${PROJECT_NAME} PUBLIC lib/tinyutf8)
add_library(tinyutf8 STATIC ${TINYUTF8_SRC})
target_link_libraries(${PROJECT_NAME} tinyutf8)

# fmod
# TODO: make for all platforms

target_include_directories(${PROJECT_NAME} PUBLIC lib/fmod/include)
	
if(WIN32)
	add_library(fmod SHARED IMPORTED)
	target_link_libraries(${PROJECT_NAME} fmod)
endif()

if(WIN32)
	if(X64)
		set_target_properties(fmod PROPERTIES
			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodL_vc.lib
			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmod_vc.lib)
	else()
		set_target_properties(fmod PROPERTIES IMPORTED_IMPLIB 
			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodL_vc.lib
			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmod_vc.lib)
	endif()
elseif(ANDROID)
	#set_target_properties(fmod PROPERTIES IMPORTED_LOCATION lib/fmod/android/${ANDROID_ABI}/libfmod.so)
endif()

function(copy_required_libs TARGET_NAME)
	if(WIN32)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(X64 TRUE)
		elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
			set(X86 TRUE)
		endif()
		if(X64)
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmodL.dll">
				$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmod.dll">
				$<TARGET_FILE_DIR:${TARGET_NAME}>)
		else()
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmodL.dll">
				$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmod.dll">
				$<TARGET_FILE_DIR:${TARGET_NAME}>)
		endif()
	elseif(ANDROID)
	#	add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
	#		../../../../../../sky/lib/fmod/android/${ANDROID_ABI}/libfmod.so
	#		${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libfmod.so)
	endif()
endfunction()

if(APPLE)
#	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmodL_iphonesimulator.a)
#	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmodL_iphoneos.a)
##	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmod_iphonesimulator.a) --- this two was commented 
##	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sky/lib/fmod/ios/libfmod_iphoneos.a)
endif()

# imgui

file(GLOB IMGUI_SRC 
	lib/imgui/*.cpp 
	lib/imgui/*.h
)	
target_include_directories(${PROJECT_NAME} PUBLIC lib/imgui)
add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} imgui)

# fmt

file(GLOB FMT_SRC
	lib/fmt/src/*.cc
)
add_library(fmt STATIC ${FMT_SRC})
target_link_libraries(${PROJECT_NAME} fmt)
target_include_directories(fmt PUBLIC lib/fmt/include)

# nlohmann_json

target_include_directories(${PROJECT_NAME} PUBLIC lib/nlohmann_json/include)

# glew

file(GLOB GLEW_SRC 
	lib/glew/src/glew.c
    lib/glew/include/GL/glew.h
)
add_library(glew STATIC ${GLEW_SRC})
target_link_libraries(${PROJECT_NAME} glew)
target_include_directories(glew PUBLIC lib/glew/include)
target_compile_definitions(glew PRIVATE -DGLEW_STATIC)
