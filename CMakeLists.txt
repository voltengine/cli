cmake_minimum_required(VERSION 3.20)
project(volt_cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/bin")
endforeach()

file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(volt_cli ${TARGET_SOURCES})

set_target_properties(volt_cli PROPERTIES
    CXX_STANDARD 20
    OUTPUT_NAME "volt"
)

target_include_directories(volt_cli PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

add_subdirectory("vendor/rapidjson")
target_link_libraries(volt_cli RapidJSON)
