set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(EXAMPLES_FOLDER "examples")

# 01 - triangle

add_executable(01_hello_world 01_hello_world/main.cpp)
target_link_libraries(01_hello_world sky)
set_property(TARGET 01_hello_world PROPERTY FOLDER ${EXAMPLES_FOLDER})
copy_required_libs(01_hello_world "${PROJECT_SOURCE_DIR}/lib/fmod")
