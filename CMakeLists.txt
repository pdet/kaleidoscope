project (Kaleidoscope)
cmake_minimum_required (VERSION 2.6)
SET(CMAKE_CXX_STANDARD 11)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(src/include)
add_executable(Kaleidoscope src/logger/logger.cpp src/lexer/lexer.cpp src/parser/parser.cpp)


# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(Kaleidoscope ${llvm_libs})