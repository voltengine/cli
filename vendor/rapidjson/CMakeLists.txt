cmake_minimum_required(VERSION 3.20)

project(RapidJSON VERSION 1.0.0 LANGUAGES CXX)

add_library(RapidJSON INTERFACE)

target_include_directories(RapidJSON INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include"
)
