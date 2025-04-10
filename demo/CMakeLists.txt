cmake_minimum_required(VERSION 3.2)

project(main)

set(CMAKE_CXX_STANDARD 11)

# conditionally set the flags based on the value of CMAKE_BUILD_TYPE
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-static -s")
elseif (CMAKE_BUILD_TYPE STREQUAL "Profile")
    set(CMAKE_CXX_FLAGS "-static -g -O0 -fprofile-arcs -ftest-coverage")
endif()

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PrimeChecker.cpp
)

enable_testing()
add_executable(${PROJECT_NAME}-Runner 
    ${CMAKE_CURRENT_SOURCE_DIR}/TestPrimeChecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PrimeChecker.cpp
)
# generate unit test report in JUnit format (required by GitLab)
add_test(NAME ${PROJECT_NAME}-Runner COMMAND ${PROJECT_NAME}-Runner --reporter junit -o test-report.xml)
