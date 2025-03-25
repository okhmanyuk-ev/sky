cmake_minimum_required(VERSION 3.10)

project(sky)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(X64 TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(X86 TRUE)
endif()

set(LIBS_FOLDER "libs")

file(GLOB_RECURSE ALL_SRC
	src/*.cpp
	src/*.hpp
	src/*.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SRC})

add_library(${PROJECT_NAME} STATIC ${ALL_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC /MP /bigobj)
endif()

if(APPLE)
	if(CMAKE_OSX_SYSROOT STREQUAL iphoneos)
		set(IOS True)
	endif()
	if(IOS)
		target_compile_definitions(${PROJECT_NAME} PUBLIC -DPLATFORM_IOS)
	else()
		target_compile_definitions(${PROJECT_NAME} PUBLIC -DPLATFORM_MAC)
	endif()

	set(OBJC_SOURCES
		src/renderer/system.cpp
		src/platform/asset.cpp
	)
	set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fembed-bitcode")

	target_link_libraries(${PROJECT_NAME}
		"-ObjC"
		"-framework CoreGraphics"
		"-framework Foundation"
		"-framework AVFoundation"
		"-framework AudioToolbox"
		"-framework StoreKit"
	)

	if(IOS)
		target_link_libraries(${PROJECT_NAME}
			"-framework UIKit"
		)
	endif()
elseif(ANDROID)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -frtti -fexceptions -Wno-error=format-security")

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
elseif(EMSCRIPTEN)
	if(NOT DEFINED EMSCRIPTEN_SHELL_FILE_PATH)
		set(EMSCRIPTEN_SHELL_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/emscripten/shell.html)
	endif()

	message(STATUS "EMSCRIPTEN_SHELL_FILE_PATH is ${EMSCRIPTEN_SHELL_FILE_PATH}")

	target_link_options(${PROJECT_NAME} PUBLIC
		-sFETCH=1
		-sUSE_SDL=2
		-sNO_DISABLE_EXCEPTION_CATCHING=1
		-sDISABLE_EXCEPTION_THROWING=0
		-sALLOW_MEMORY_GROWTH
		-sCASE_INSENSITIVE_FS
		--shell-file ${EMSCRIPTEN_SHELL_FILE_PATH}
		--use-preload-cache
		-lwebsocket.js
	)
	target_compile_options(${PROJECT_NAME} PUBLIC
		-sUSE_SDL=2
		-sNO_DISABLE_EXCEPTION_CATCHING=1
		-sDISABLE_EXCEPTION_THROWING=0
	)
	target_link_options(${PROJECT_NAME} PUBLIC $<IF:$<CONFIG:DEBUG>,
		-g3
		-gsource-map
		-sEXIT_RUNTIME
		-sASSERTIONS=2
		-sSTACK_OVERFLOW_CHECK=2
		-sGL_ASSERTIONS=1
		#-sSAFE_HEAP=2 # uncomment to enable more checks but this highly reduce performance
	,>)
	target_compile_options(${PROJECT_NAME} PUBLIC $<IF:$<CONFIG:DEBUG>,
		-g3
		-gsource-map
	,>)
	target_link_options(${PROJECT_NAME} PUBLIC $<IF:$<CONFIG:RELEASE>,
		--emit-symbol-map
	,>)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	-D_CRT_SECURE_NO_WARNINGS
	-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
	-D_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
)
if(EMSCRIPTEN)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

# glm

set(GLM_QUIET ON)
set(BUILD_SHARED_LIBS OFF)
set(GLM_TEST_ENABLE OFF)
add_subdirectory(lib/glm)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DGLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${PROJECT_NAME} glm)
target_include_directories(${PROJECT_NAME} PUBLIC lib/glm)
target_compile_definitions(glm INTERFACE -D_CRT_SECURE_NO_WARNINGS) # for clang

# stb

target_include_directories(${PROJECT_NAME} PUBLIC lib/stb)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_IMAGE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_RECT_PACK_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_TRUETYPE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTB_IMAGE_WRITE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DSTBI_WRITE_NO_STDIO)

# rectpack2D

target_include_directories(${PROJECT_NAME} PUBLIC lib/rectpack2D/include)

# fmod

#target_include_directories(${PROJECT_NAME} PUBLIC lib/fmod/include)

#if(WIN32)
#	add_library(fmod SHARED IMPORTED)
#	add_library(fmodstudio SHARED IMPORTED)
#	target_link_libraries(${PROJECT_NAME} fmod)
#	target_link_libraries(${PROJECT_NAME} fmodstudio)
#	set_target_properties(fmod PROPERTIES
#		MAP_IMPORTED_CONFIG_MINSIZEREL Release
#		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
#	)
#	set_target_properties(fmodstudio PROPERTIES
#		MAP_IMPORTED_CONFIG_MINSIZEREL Release
#		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
#	)
#	if(X64)
#		set_target_properties(fmod PROPERTIES
#			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodL_vc.lib
#			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmod_vc.lib)
#		set_target_properties(fmodstudio PROPERTIES
#			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodstudioL_vc.lib
#			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x64/fmodstudio_vc.lib)
#	else()
#		set_target_properties(fmod PROPERTIES
#			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodL_vc.lib
#			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmod_vc.lib)
#		set_target_properties(fmodstudio PROPERTIES
#			IMPORTED_IMPLIB_DEBUG ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodstudioL_vc.lib
#			IMPORTED_IMPLIB_RELEASE ${PROJECT_SOURCE_DIR}/lib/fmod/x86/fmodstudio_vc.lib)
#	endif()
#elseif(ANDROID)
#	add_library(fmod SHARED IMPORTED)
#	add_library(fmodstudio SHARED IMPORTED)
#
#	set_target_properties(fmod PROPERTIES IMPORTED_LOCATION
#		${PROJECT_SOURCE_DIR}/lib/fmod/android/${ANDROID_ABI}/libfmod.so)
#
#	set_target_properties(fmodstudio PROPERTIES IMPORTED_LOCATION
#		${PROJECT_SOURCE_DIR}/lib/fmod/android/${ANDROID_ABI}/libfmodstudio.so)
#
#	target_link_libraries(${PROJECT_NAME} fmod)
#	target_link_libraries(${PROJECT_NAME} fmodstudio)
#elseif(APPLE)
#	if(IOS)
#		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodL_iphonesimulator.a)
#		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodL_iphoneos.a)
#		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmod_iphonesimulator.a)
#		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmod_iphoneos.a)
#		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudioL_iphonesimulator.a)
#		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudioL_iphoneos.a)
#		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudio_iphonesimulator.a)
#		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/lib/fmod/ios/libfmodstudio_iphoneos.a)
#	else()
#		# TODO: implement for mac
#	endif()
#endif()

function(copy_required_libs TARGET_NAME)
	#if(NOT BASE_FOLDER)
	#	set(BASE_FOLDER "${PROJECT_SOURCE_DIR}/sky/lib/fmod")
	#endif()
#	set(BASE_FOLDER "${PROJECT_SOURCE_DIR}/sky/lib/fmod")
#	if(ARGC GREATER 1)
#		list(GET ARGN 0 BASE_FOLDER)
#	endif()
#	if(WIN32)
#		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#			set(X64 TRUE)
#		elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#			set(X86 TRUE)
#		endif()
#		if(X64)
#			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
#				$<$<CONFIG:Debug>:"${BASE_FOLDER}/x64/fmodL.dll">
#				$<$<NOT:$<CONFIG:Debug>>:"${BASE_FOLDER}/x64/fmod.dll">
#				$<TARGET_FILE_DIR:${TARGET_NAME}>)
#
#			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
#				$<$<CONFIG:Debug>:"${BASE_FOLDER}/x64/fmodstudioL.dll">
#				$<$<NOT:$<CONFIG:Debug>>:"${BASE_FOLDER}/x64/fmodstudio.dll">
#				$<TARGET_FILE_DIR:${TARGET_NAME}>)
#		else()
#			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
#				$<$<CONFIG:Debug>:"${BASE_FOLDER}/x86/fmodL.dll">
#				$<$<NOT:$<CONFIG:Debug>>:"${BASE_FOLDER}/x86/fmod.dll">
#				$<TARGET_FILE_DIR:${TARGET_NAME}>)
#
#			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
#				$<$<CONFIG:Debug>:"${BASE_FOLDER}/x86/fmodstudioL.dll">
#				$<$<NOT:$<CONFIG:Debug>>:"${BASE_FOLDER}/x86/fmodstudio.dll">
#				$<TARGET_FILE_DIR:${TARGET_NAME}>)
#		endif()
#	elseif(ANDROID)
#		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
#			${BASE_FOLDER}/android/${ANDROID_ABI}/libfmod.so
#			${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libfmod.so)
#
#		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
#			${BASE_FOLDER}/android/${ANDROID_ABI}/libfmodstudio.so
#			${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libfmodstudio.so)
#	endif()
endfunction()

# imgui

function(disable_warnings TARGET_NAME)
	if(MSVC)
		target_compile_options(${TARGET_NAME} PRIVATE /W0)
	else()
		target_compile_options(${TARGET_NAME} PRIVATE -w)
	endif()
endfunction()

file(GLOB IMGUI_SRC
	lib/imgui/*.cpp
	lib/imgui/*.h
	lib/imgui/misc/cpp/*.cpp
	lib/imgui/misc/cpp/*.h
)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui SYSTEM PUBLIC lib/imgui)
target_include_directories(imgui SYSTEM PUBLIC lib/imgui/misc/cpp)
target_link_libraries(${PROJECT_NAME} imgui)
set_property(TARGET imgui PROPERTY FOLDER ${LIBS_FOLDER})
disable_warnings(imgui)

# fmt

add_subdirectory(lib/fmt)
target_link_libraries(${PROJECT_NAME} fmt)
set_property(TARGET fmt PROPERTY FOLDER ${LIBS_FOLDER})

# nlohmann_json

target_include_directories(${PROJECT_NAME} PUBLIC lib/nlohmann_json)

# tinyobjloader

file(GLOB TINYOBJLOADER_SRC
	lib/tinyobjloader/*.cc
	lib/tinyobjloader/*.h
)
target_include_directories(${PROJECT_NAME} PUBLIC lib/tinyobjloader)
add_library(tinyobjloader STATIC ${TINYOBJLOADER_SRC})
target_link_libraries(${PROJECT_NAME} tinyobjloader)
set_property(TARGET tinyobjloader PROPERTY FOLDER ${LIBS_FOLDER})

# asio

if(NOT EMSCRIPTEN)
	add_library(asio INTERFACE)
	target_include_directories(asio INTERFACE lib/asio/include)
	target_link_libraries(${PROJECT_NAME} asio)
	if(EMSCRIPTEN)
		target_compile_definitions(asio INTERFACE -DASIO_HAS_PTHREADS)
	elseif(WIN32)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DWIN32_LEAN_AND_MEAN)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -D_WIN32_WINNT=0x0601)
	endif()
endif()

# websocketpp

if(NOT EMSCRIPTEN)
	add_library(websocketpp INTERFACE)
	target_compile_definitions(websocketpp INTERFACE -DASIO_STANDALONE)
	target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_FUNCTIONAL_)
	target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_SYSTEM_ERROR_)
	target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_RANDOM_DEVICE_)
	target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_MEMORY_)
	target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_STL_)
	target_compile_definitions(websocketpp INTERFACE -D_WEBSOCKETPP_CPP11_THREAD_)
	if(MSVC)
		target_compile_options(websocketpp INTERFACE /wd4267)
	endif()
	target_include_directories(websocketpp INTERFACE lib/websocketpp)
	target_link_libraries(${PROJECT_NAME} websocketpp)
endif()

# openssl

if(SKY_USE_OPENSSL)
	target_compile_definitions(${PROJECT_NAME} PUBLIC -DSKY_USE_OPENSSL)
	if(NOT EMSCRIPTEN)
		if (LINUX)
			target_link_libraries(${PROJECT_NAME} ssl crypto)
		else()
			find_package(OpenSSL REQUIRED)
			target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
			target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
		endif()
	endif()
endif()

# box2d

add_subdirectory(lib/box2d)
target_link_libraries(${PROJECT_NAME} box2d)
set_property(TARGET box2d PROPERTY FOLDER ${LIBS_FOLDER})

# magic_enum

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE lib/magic_enum)
target_compile_definitions(magic_enum INTERFACE
    MAGIC_ENUM_RANGE_MIN=0
    MAGIC_ENUM_RANGE_MAX=256
)
target_link_libraries(${PROJECT_NAME} magic_enum)

# glfw

if(WIN32 OR APPLE AND NOT IOS)
	set(GLFW_INSTALL OFF)
	set(GLFW_BUILD_DOCS OFF)
	add_subdirectory(lib/glfw)
	set_property(TARGET glfw PROPERTY FOLDER ${LIBS_FOLDER}/glfw)
	set_property(TARGET update_mappings PROPERTY FOLDER ${LIBS_FOLDER}/glfw)
	target_link_libraries(${PROJECT_NAME} glfw)
endif()

# tinyxml2

add_library(tinyxml2 STATIC
	lib/tinyxml2/tinyxml2.cpp
	lib/tinyxml2/tinyxml2.h
)
target_include_directories(imgui PUBLIC lib/tinyxml2)
target_link_libraries(${PROJECT_NAME} tinyxml2)
set_property(TARGET tinyxml2 PROPERTY FOLDER ${LIBS_FOLDER})

# skygfx

add_subdirectory(lib/skygfx)
target_link_libraries(${PROJECT_NAME} skygfx)

# examples

if(NOT EMSCRIPTEN)
	add_subdirectory(examples)
endif()
