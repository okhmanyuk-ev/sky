if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(X86 TRUE)
endif()

file(GLOB CORE_SRC
    "sky/src/Core/*.cpp"
    "sky/src/Core/*.h"
)

file(GLOB CONSOLE_SRC
    "sky/src/Console/*.cpp"
    "sky/src/Console/*.h"
)

file(GLOB COMMON_SRC
    "sky/src/Common/*.cpp"
    "sky/src/Common/*.h"
)

file(GLOB PLATFORM_SRC
	sky/src/Platform/*.cpp
	sky/src/Platform/*.mm
    sky/src/Platform/*.h
)

file(GLOB GRAPHICS_SRC
    "sky/src/Graphics/*.cpp"
    "sky/src/Graphics/*.h"
)

file(GLOB RENDERER_SRC
	sky/src/Renderer/*.cpp
	sky/src/Renderer/*.mm
    sky/src/Renderer/*.h
)

file(GLOB RENDERER_D3D11_SRC
	sky/src/Renderer/D3D11/*.cpp
	sky/src/Renderer/D3D11/*.mm
    sky/src/Renderer/D3D11/*.h
)

file(GLOB RENDERER_OPENGL_SRC
	sky/src/Renderer/OpenGL/*.cpp
	sky/src/Renderer/OpenGL/*.mm
    sky/src/Renderer/OpenGL/*.h
)

file(GLOB AUDIO_SRC
	"sky/src/Audio/*.cpp"
	"sky/src/Audio/*.h"
)

file(GLOB SCENE_SRC
    "sky/src/Scene/*.cpp"
    "sky/src/Scene/*.h"
)

file(GLOB SHARED_SRC
    "sky/src/Shared/*.cpp"
    "sky/src/Shared/*.h"
)

file(GLOB NETWORK_SRC
    "sky/src/Network/*.cpp"
    "sky/src/Network/*.h"
)

source_group("Core" FILES ${CORE_SRC})
source_group("Console" FILES ${CONSOLE_SRC})
source_group("Common" FILES ${COMMON_SRC})
source_group("Platform" FILES ${PLATFORM_SRC})
source_group("Graphics" FILES ${GRAPHICS_SRC})
source_group("Renderer" FILES ${RENDERER_SRC})
source_group("Renderer\\D3D11" FILES ${RENDERER_D3D11_SRC})
source_group("Renderer\\OpenGL" FILES ${RENDERER_OPENGL_SRC})
source_group("Audio" FILES ${AUDIO_SRC})
source_group("Scene" FILES ${SCENE_SRC})
source_group("Shared" FILES ${SHARED_SRC})
source_group("Network" FILES ${NETWORK_SRC})

include_directories("sky/src")
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
		sky/src/Renderer/system_gl.cpp
		sky/src/Platform/asset.cpp
	)
	set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fembed-bitcode")
endif()

if(WIN32)
	add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
endif()

# glm

target_compile_definitions(sky PRIVATE -DGLM_ENABLE_EXPERIMENTAL)
include_directories("sky/lib/glm")

# stb

include_directories("sky/lib/stb")

# glew

file(GLOB GLEW_SRC 
	"sky/lib/glew/src/glew.c"
    "sky/lib/glew/include/GL/glew.h"
)
include_directories("sky/lib/glew/include")
add_library(glew STATIC ${GLEW_SRC})
target_link_libraries(${PROJECT_NAME} glew)
target_compile_definitions(glew PRIVATE -DGLEW_STATIC)

# imgui

file(GLOB IMGUI_SRC 
	sky/lib/imgui/*.cpp 
	sky/lib/imgui/*.h
)	
include_directories("sky/lib/imgui")
add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} imgui)

# tinygltf

include_directories("sky/lib/tinygltf")

# tinyobjloader

include_directories("sky/lib/tinyobjloader")

# tinyutf8

file(GLOB TINYUTF8_SRC
	sky/lib/tinyutf8/*.cpp
	sky/lib/tinyutf8/*.h
	sky/lib/tinyutf8/*.hpp
)
include_directories("sky/lib/tinyutf8")
add_library(tinyutf8 STATIC ${TINYUTF8_SRC})
target_link_libraries(${PROJECT_NAME} tinyutf8)

# fmod

include_directories("sky/lib/fmod/include")
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

include_directories("sky/lib/rectpack2D/include")

# nlohmann_json

include_directories("sky/lib/nlohmann_json/include")