cmake_minimum_required(VERSION 3.10)
project(sky)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(X86 TRUE)
endif()

set(LIBS_FOLDER "libs")

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

file(GLOB IMSCENE_SRC
    src/imscene/*.cpp
    src/imscene/*.h
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
source_group("imscene" FILES ${IMSCENE_SRC})
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
	${IMSCENE_SRC}
	${SHARED_SRC}
	${NETWORK_SRC}
)

if(APPLE)
	set(OBJC_SOURCES
		src/renderer/system_gl.cpp
		src/platform/asset.cpp
	)
	set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fembed-bitcode")

	target_link_libraries(${PROJECT_NAME}
		"-ObjC"
		"-framework UIKit"
		"-framework CoreGraphics"
		"-framework GLKit"
		"-framework OpenGLES"
		"-framework Foundation"
		"-framework AVFoundation"
		"-framework AudioToolbox"
		"-framework StoreKit"
	)

	add_definitions(-DGLES_SILENCE_DEPRECATION)
elseif(ANDROID)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -frtti -fexceptions -Wno-error=format-security")

	add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	target_include_directories(${PROJECT_NAME} PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

	target_link_libraries(${PROJECT_NAME}
		android
		native_app_glue
		EGL
		GLESv3
		log
		m
	)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)

if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

# glm

set(GLM_QUIET ON)
set(BUILD_SHARED_LIBS OFF)
set(GLM_TEST_ENABLE OFF)
add_subdirectory(lib/glm)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DGLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${PROJECT_NAME} glm)
target_include_directories(${PROJECT_NAME} PUBLIC lib/glm)

# stb

target_include_directories(${PROJECT_NAME} PUBLIC lib/stb)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_IMAGE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_RECT_PACK_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_TRUETYPE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_IMAGE_WRITE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTBI_WRITE_NO_STDIO)

# rectpack2D

target_include_directories(${PROJECT_NAME} PUBLIC lib/rectpack2D/include)

# tinyutf8

add_subdirectory(lib/tinyutf8)
target_include_directories(${PROJECT_NAME} PUBLIC lib/tinyutf8)
target_link_libraries(${PROJECT_NAME} tinyutf8)
if(MSVC)
	target_compile_options(tinyutf8 PRIVATE /wd4267)
endif()
set_property(TARGET tinyutf8 PROPERTY FOLDER ${LIBS_FOLDER})

# fmod

target_include_directories(${PROJECT_NAME} PUBLIC lib/fmod/include)

if(WIN32)
	add_library(fmod SHARED IMPORTED)
	add_library(fmodstudio SHARED IMPORTED)
	target_link_libraries(${PROJECT_NAME} fmod)
	target_link_libraries(${PROJECT_NAME} fmodstudio)
	set_target_properties(fmod PROPERTIES
		MAP_IMPORTED_CONFIG_MINSIZEREL Release
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
	)
	set_target_properties(fmodstudio PROPERTIES
		MAP_IMPORTED_CONFIG_MINSIZEREL Release
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
	)
	if(X64)
		set_target_properties(fmod PROPERTIES
			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodL_vc.lib
			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmod_vc.lib)
		set_target_properties(fmodstudio PROPERTIES
			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodstudioL_vc.lib
			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodstudio_vc.lib)
	else()
		if(MINGW)
			set_target_properties(fmod PROPERTIES
				IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmod_vc.lib)
			set_target_properties(fmodstudio PROPERTIES
				IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodstudio_vc.lib)
		else()
			set_target_properties(fmod PROPERTIES
				IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodL_vc.lib
				IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmod_vc.lib)
			set_target_properties(fmodstudio PROPERTIES
				IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodstudioL_vc.lib
				IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodstudio_vc.lib)
		endif()
	endif()
elseif(ANDROID)
	add_library(fmod SHARED IMPORTED)
	add_library(fmodstudio SHARED IMPORTED)
	
	set_target_properties(fmod PROPERTIES IMPORTED_LOCATION
		${PROJECT_SOURCE_DIR}/lib/fmod/android/${ANDROID_ABI}/libfmod.so)

	set_target_properties(fmodstudio PROPERTIES IMPORTED_LOCATION
		${PROJECT_SOURCE_DIR}/lib/fmod/android/${ANDROID_ABI}/libfmodstudio.so)

	target_link_libraries(${PROJECT_NAME} fmod)
	target_link_libraries(${PROJECT_NAME} fmodstudio)
elseif(APPLE)
	target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodL_iphonesimulator.a)
	target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodL_iphoneos.a)
	target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmod_iphonesimulator.a)
	target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmod_iphoneos.a)
	target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudioL_iphonesimulator.a)
	target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudioL_iphoneos.a)
	target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudio_iphonesimulator.a)
	target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudio_iphoneos.a)
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

			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmodstudioL.dll">
				$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x64/fmodstudio.dll">
				$<TARGET_FILE_DIR:${TARGET_NAME}>)
		else()
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmodL.dll">
				$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmod.dll">
				$<TARGET_FILE_DIR:${TARGET_NAME}>)

			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmodstudioL.dll">
				$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/sky/lib/fmod/x86/fmodstudio.dll">
				$<TARGET_FILE_DIR:${TARGET_NAME}>)
		endif()
	elseif(ANDROID)
		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
			${PROJECT_SOURCE_DIR}/sky/lib/fmod/android/${ANDROID_ABI}/libfmod.so
			${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libfmod.so)

		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
			${PROJECT_SOURCE_DIR}/sky/lib/fmod/android/${ANDROID_ABI}/libfmodstudio.so
			${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libfmodstudio.so)
	endif()
endfunction()

# imgui

file(GLOB IMGUI_SRC 
	lib/imgui/*.cpp 
	lib/imgui/*.h
)	
target_include_directories(${PROJECT_NAME} PUBLIC lib/imgui)
add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} imgui)
set_property(TARGET imgui PROPERTY FOLDER ${LIBS_FOLDER})

# fmt

add_subdirectory(lib/fmt)
target_link_libraries(${PROJECT_NAME} fmt)
set_property(TARGET fmt PROPERTY FOLDER ${LIBS_FOLDER})

# nlohmann_json

target_include_directories(${PROJECT_NAME} PUBLIC lib/nlohmann_json)

# glew

if(WIN32)
	file(GLOB GLEW_SRC 
		lib/glew/src/glew.c
		lib/glew/include/GL/glew.h
	)
	add_library(glew STATIC ${GLEW_SRC})
	target_link_libraries(${PROJECT_NAME} glew)
	target_include_directories(glew PUBLIC lib/glew/include)
	target_compile_definitions(glew PRIVATE -DGLEW_STATIC)
	set_property(TARGET glew PROPERTY FOLDER ${LIBS_FOLDER})
endif()

# tinyobjloader

target_include_directories(${PROJECT_NAME} PUBLIC lib/tinyobjloader)

# asio

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE lib/asio/include)
target_link_libraries(${PROJECT_NAME} asio)

if(WIN32)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DWIN32_LEAN_AND_MEAN)
endif()

# websocketpp

add_library(websocketpp INTERFACE)
target_compile_definitions(websocketpp INTERFACE -DASIO_STANDALONE)
target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_FUNCTIONAL_)
target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_)
target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_)
target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_MEMORY_)
target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_STL_)
if(MSVC)
	target_compile_options(websocketpp INTERFACE /wd4267)
endif()
target_include_directories(websocketpp INTERFACE lib/websocketpp)
target_link_libraries(${PROJECT_NAME} websocketpp)
