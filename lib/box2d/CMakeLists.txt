project(box2d)

file(GLOB BOX2D_SOURCE_FILES
	src/collision/*.cpp
	src/collision/*.h

	src/common/*.cpp
	src/common/*.h

	src/dynamics/*.cpp
	src/dynamics/*.h

	src/rope/*.cpp
	src/rope/*.h)

file(GLOB BOX2D_HEADER_FILES
	include/box2d/*.h
	include/box2d/*.cpp)
	
add_library(${PROJECT_NAME} STATIC ${BOX2D_SOURCE_FILES} ${BOX2D_HEADER_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)