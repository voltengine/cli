cmake_minimum_required(VERSION 3.20)
project(VoltCLI VERSION 0.1.0 LANGUAGES CXX)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "^(Linux|Windows)$")
    message(FATAL_ERROR "Unknown OS: ${CMAKE_SYSTEM_NAME}")
endif()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "64")
    set(VOLT_CLI_ARCH "AMD64")
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "86")
    set(VOLT_CLI_ARCH "X86")
else()
    message(FATAL_ERROR "Unknown CPU architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(VOLT_CLI_TOOLCHAIN_PREFIX ${CMAKE_SYSTEM_NAME}${VOLT_CLI_ARCH})
unset(VOLT_CLI_ARCH)

add_compile_definitions(VOLT_CLI_TOOLCHAIN_PREFIX="${VOLT_CLI_TOOLCHAIN_PREFIX}")
message("Volt CLI toolchain prefix: ${VOLT_CLI_TOOLCHAIN_PREFIX}")

# Set output directory to "${CMAKE_BINARY_DIR}/bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/bin")
endforeach()

# Find Conan packages
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
find_package(CURL REQUIRED)
# find_package(date REQUIRED)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/date")
find_package(nlohmann_json REQUIRED)
find_package(termcolor REQUIRED)

file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
)

add_executable(VoltCLI ${TARGET_SOURCES})

set_target_properties(VoltCLI PROPERTIES
    CXX_STANDARD 20
    OUTPUT_NAME "volt"
)

target_include_directories(VoltCLI PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(VoltCLI PRIVATE
    CURL::CURL
    Date::Date
    nlohmann_json::nlohmann_json
    termcolor::termcolor
)

target_precompile_headers(VoltCLI PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp"
)
