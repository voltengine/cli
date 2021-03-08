include_guard(GLOBAL)

file(STRINGS "${CMAKE_SOURCE_DIR}/.volt/cmake_dirs.txt" VOLT_TARGET_DIRS)

foreach(VOLT_TARGET_DIR ${VOLT_TARGET_DIRS})
	add_subdirectory(${VOLT_TARGET_DIR})
endforeach()
