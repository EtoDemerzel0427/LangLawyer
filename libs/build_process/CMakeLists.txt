add_library(build_process_lib SHARED
    lib.cpp
)

target_include_directories(build_process_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/build_process>
)

if(ENABLE_SAVE_TEMPS)
    target_compile_options(build_process_lib PRIVATE -save-temps=obj)
endif()

install(TARGETS build_process_lib
    EXPORT build_processTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/build_process
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/api.hpp
    DESTINATION include/build_process
)

install(EXPORT build_processTargets
    FILE build_processTargets.cmake
    NAMESPACE langlawyer::
    DESTINATION lib/cmake/build_process
)


