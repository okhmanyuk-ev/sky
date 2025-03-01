set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXAMPLES_FOLDER "examples")

# 01 - hello_world

add_executable(01_hello_world 01_hello_world/main.cpp)
target_link_libraries(01_hello_world sky)
set_property(TARGET 01_hello_world PROPERTY FOLDER ${EXAMPLES_FOLDER})
copy_required_libs(01_hello_world "${PROJECT_SOURCE_DIR}/lib/fmod")

# 02 - windows

add_executable(02_windows 02_windows/main.cpp)
target_link_libraries(02_windows sky)
set_property(TARGET 02_windows PROPERTY FOLDER ${EXAMPLES_FOLDER})
copy_required_libs(02_windows "${PROJECT_SOURCE_DIR}/lib/fmod")

# 03 - cube

add_executable(03_cube 03_cube/main.cpp)
target_link_libraries(03_cube sky)
set_property(TARGET 03_cube PROPERTY FOLDER ${EXAMPLES_FOLDER})
copy_required_libs(03_cube "${PROJECT_SOURCE_DIR}/lib/fmod")

# 04 - physics

add_executable(04_physics 04_physics/main.cpp)
target_link_libraries(04_physics sky)
set_property(TARGET 04_physics PROPERTY FOLDER ${EXAMPLES_FOLDER})
copy_required_libs(04_physics "${PROJECT_SOURCE_DIR}/lib/fmod")

# 05 - labels

add_executable(05_labels 05_labels/main.cpp)
target_link_libraries(05_labels sky)
set_property(TARGET 05_labels PROPERTY FOLDER ${EXAMPLES_FOLDER})
copy_required_libs(05_labels "${PROJECT_SOURCE_DIR}/lib/fmod")
if(MSVC)
	set_target_properties(05_labels PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
