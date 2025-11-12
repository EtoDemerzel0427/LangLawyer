add_library(build_process_lib SHARED
lib.cpp
api.hpp
)

target_include_directories(build_process_lib
PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(build_process_app
main.cpp
)

target_link_libraries(build_process_app
PRIVATE
build_process_lib
)


