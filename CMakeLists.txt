cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(waemon)

execute_process(
    COMMAND pkg-config --variable=pkgdatadir wayland-protocols
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Found wayland-protocols at ${WAYLAND_PROTOCOLS_DIR}")

set(WL_CODEGEN_DIR "${CMAKE_BINARY_DIR}/wl")
function(protocol proj protoPath protoName external)
    file(MAKE_DIRECTORY ${WL_CODEGEN_DIR})
    if (external)
        execute_process(
            COMMAND wayland-scanner client-header ${protoPath} ${WL_CODEGEN_DIR}/${protoName}.h
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        execute_process(
            COMMAND wayland-scanner private-code ${protoPath} ${WL_CODEGEN_DIR}/${protoName}.c
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        target_sources(${proj} PRIVATE ${WL_CODEGEN_DIR}/${protoName}.h ${WL_CODEGEN_DIR}/${protoName}.c)
    else()
        execute_process(
            COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOLS_DIR}/${protoPath} ${WL_CODEGEN_DIR}/${protoName}.h
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        execute_process(
            COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOLS_DIR}/${protoPath} ${WL_CODEGEN_DIR}/${protoName}.c
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        target_sources(${proj} PRIVATE ${WL_CODEGEN_DIR}/${protoName}.h ${WL_CODEGEN_DIR}/${protoName}.c)
    endif()
endfunction()

file(GLOB_RECURSE SRCFILES "src/*.cc")

add_executable(waemon ${SRCFILES})
set_property(TARGET waemon PROPERTY CXX_STANDARD 23)

find_package(OpenImageIO REQUIRED)
find_package(CAIRO REQUIRED)
find_package(nlohmann_json REQUIRED)


protocol(waemon "protocols/wlr-layer-shell-unstable-v1.xml" "wlr-layer-shell-unstable-v1" true)
protocol(waemon "stable/xdg-shell/xdg-shell.xml" "xdg-shell" false)
protocol(waemon "protocols/xdg-output-unstable-v1.xml" "xdg-output-unstable-v1" true)


target_link_libraries(waemon PRIVATE wayland-client ${OpenImageIO_LIBRARY} ${CAIRO_LIBRARIES} nlohmann_json::nlohmann_json)
target_include_directories(waemon PRIVATE src/ ${WL_CODEGEN_DIR} ${OpenImageIO_INCLUDE_DIR} ${CAIRO_INCLUDE_DIRS})


install(TARGETS waemon DESTINATION bin)