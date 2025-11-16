add_executable(build_process_app
main.cpp
)

target_link_libraries(build_process_app
PRIVATE
build_process_lib
)

if(ENABLE_SAVE_TEMPS)
    target_compile_options(build_process_app PRIVATE -save-temps=obj)
endif()

install(TARGETS build_process_app
    RUNTIME DESTINATION bin
)


