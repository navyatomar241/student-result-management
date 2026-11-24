cmake_minimum_required(VERSION 3.15)
project(student_result_management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include header paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Optional system dependencies
find_package(Threads REQUIRED)

# Check for PostgreSQL libpqxx if available on the build system
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBPQXX QUIET libpqxx)
endif()

# Source files
set(SOURCES
    main.cpp
    database.cpp
    auth.cpp
    student.cpp
    teacher.cpp
    subject.cpp
    marks.cpp
    result.cpp
)

# Executable target
add_executable(student_result_server ${SOURCES})

target_link_libraries(student_result_server PRIVATE
    Threads::Threads
)

if(LIBPQXX_FOUND)
    target_include_directories(student_result_server PRIVATE ${LIBPQXX_INCLUDE_DIRS})
    target_link_libraries(student_result_server PRIVATE ${LIBPQXX_LIBRARIES})
    target_compile_definitions(student_result_server PRIVATE HAVE_LIBPQXX=1)
endif()

# Compiler options for safety and warnings
if(MSVC)
    target_compile_options(student_result_server PRIVATE /W4)
else()
    target_compile_options(student_result_server PRIVATE -Wall -Wextra -pedantic)
endif()

message(STATUS "Configured Student Result Management System backend target: student_result_server")
