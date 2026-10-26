# src/CMakeLists.txt

add_library(parcomp
    workload.cpp
    banking_workload.cpp
)

target_include_directories(parcomp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(parcomp
    PUBLIC
        Threads::Threads
)

set_project_warnings(parcomp)
