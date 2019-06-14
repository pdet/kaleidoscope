project (Kaleidoscope)
cmake_minimum_required (VERSION 2.6)

set(CMAKE_C_COMPILER /usr/local/Cellar/llvm/8.0.0_1/bin/clang CACHE PATH "" FORCE)

set(CMAKE_CXX_COMPILER /usr/local/Cellar/llvm/8.0.0_1/bin/clang++ CACHE PATH "" FORCE)

SET(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEFAULT "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O0")
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(src/include)
add_executable(Kaleidoscope src/logger/logger.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/ast/BinaryExprAST.cpp
        src/ast/CallExprAST.cpp src/ast/FunctionAST.cpp src/ast/NumberExprAST.cpp src/ast/PrototypeAST.cpp
        src/ast/VariableExprAST.cpp src/main.cpp)


# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader mcjit native)

# Link against LLVM libraries
target_link_libraries(Kaleidoscope ${llvm_libs})