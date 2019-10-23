if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(X86 TRUE)
endif()

file(GLOB CORE_SRC
    "ion/src/Core/*.cpp"
    "ion/src/Core/*.h"
)

file(GLOB CONSOLE_SRC
    "ion/src/Console/*.cpp"
    "ion/src/Console/*.h"
)

file(GLOB COMMON_SRC
    "ion/src/Common/*.cpp"
    "ion/src/Common/*.h"
)

file(GLOB PLATFORM_SRC
	ion/src/Platform/*.cpp
	ion/src/Platform/*.mm
    ion/src/Platform/*.h
)

file(GLOB GRAPHICS_SRC
    "ion/src/Graphics/*.cpp"
    "ion/src/Graphics/*.h"
)

file(GLOB RENDERER_SRC
	ion/src/Renderer/*.cpp
	ion/src/Renderer/*.mm
    ion/src/Renderer/*.h
)

file(GLOB RENDERER_D3D11_SRC
	ion/src/Renderer/D3D11/*.cpp
	ion/src/Renderer/D3D11/*.mm
    ion/src/Renderer/D3D11/*.h
)

file(GLOB RENDERER_OPENGL_SRC
	ion/src/Renderer/OpenGL/*.cpp
	ion/src/Renderer/OpenGL/*.mm
    ion/src/Renderer/OpenGL/*.h
)

file(GLOB AUDIO_SRC
	"ion/src/Audio/*.cpp"
	"ion/src/Audio/*.h"
)

file(GLOB SCENE_SRC
    "ion/src/Scene/*.cpp"
    "ion/src/Scene/*.h"
)

file(GLOB SHARED_SRC
    "ion/src/Shared/*.cpp"
    "ion/src/Shared/*.h"
)

file(GLOB NETWORK_SRC
    "ion/src/Network/*.cpp"
    "ion/src/Network/*.h"
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

include_directories("ion/src")
add_library(ion STATIC 
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
target_link_libraries(${PROJECT_NAME} ion)

if(APPLE)
	set(OBJC_SOURCES
		ion/src/Renderer/system_gl.cpp
		ion/src/Platform/asset.cpp
	)
	set_source_files_properties(${OBJC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fembed-bitcode")
endif()

if(WIN32)
	add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
endif()

# glm

target_compile_definitions(ion PRIVATE -DGLM_ENABLE_EXPERIMENTAL)
include_directories("ion/lib/glm")

# stb

include_directories("ion/lib/stb")

# glew

file(GLOB GLEW_SRC 
	"ion/lib/glew/src/glew.c"
    "ion/lib/glew/include/GL/glew.h"
)
include_directories("ion/lib/glew/include")
add_library(glew STATIC ${GLEW_SRC})
target_link_libraries(${PROJECT_NAME} glew)
target_compile_definitions(glew PRIVATE -DGLEW_STATIC)

# imgui

file(GLOB IMGUI_SRC 
	ion/lib/imgui/*.cpp 
	ion/lib/imgui/*.h
)	
include_directories("ion/lib/imgui")
add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} imgui)

# xgfx

file(GLOB XGFX_SRC 
	ion/lib/xgfx/*.cpp 
	ion/lib/xgfx/*.h
)	
include_directories("ion/lib/xgfx")
add_library(xgfx STATIC ${XGFX_SRC})
target_link_libraries(${PROJECT_NAME} xgfx)

# tinygltf

include_directories("ion/lib/tinygltf")

#tinyobjloader

include_directories("ion/lib/tinyobjloader")

# fmod

include_directories("ion/lib/fmod/include")
if(WIN32)
	if(X64)
		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/ion/lib/fmod/x64/fmodL_vc.lib)
		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/ion/lib/fmod/x64/fmod_vc.lib)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/ion/lib/fmod/x64/fmodL.dll">
			$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/ion/lib/fmod/x64/fmod.dll">
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	else()
		target_link_libraries(${PROJECT_NAME} debug ${PROJECT_SOURCE_DIR}/ion/lib/fmod/x86/fmodL_vc.lib)
		target_link_libraries(${PROJECT_NAME} optimized ${PROJECT_SOURCE_DIR}/ion/lib/fmod/x86/fmod_vc.lib)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/ion/lib/fmod/x86/fmodL.dll">
			$<$<NOT:$<CONFIG:Debug>>:"${PROJECT_SOURCE_DIR}/ion/lib/fmod/x86/fmod.dll">
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	endif()
endif()

if(APPLE)
	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/ion/lib/fmod/ios/libfmodL_iphonesimulator.a)
	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/ion/lib/fmod/ios/libfmodL_iphoneos.a)
#	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/ion/lib/fmod/ios/libfmod_iphonesimulator.a)
#	target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/ion/lib/fmod/ios/libfmod_iphoneos.a)
endif()

# rectpack2D

include_directories("ion/lib/rectpack2D/include")

# nlohmann_json

include_directories("ion/lib/nlohmann_json/include")