add_executable(concept_auto
type-constraint-auto.cpp
)

add_executable(decl_auto
decl-auto.cpp
)

add_executable(auto_deduce
auto-deduce.cpp
)

install(TARGETS concept_auto decl_auto auto_deduce
    RUNTIME DESTINATION bin
)


