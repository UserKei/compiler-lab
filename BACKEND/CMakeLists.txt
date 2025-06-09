cmake_minimum_required(VERSION 3.10)
project(CompilerToolsAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 尝试找到Crow
find_package(Crow QUIET)

if(NOT Crow_FOUND)
    # 如果找不到Crow，尝试使用pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(CROW QUIET IMPORTED_TARGET crow)
    endif()
    
    if(NOT CROW_FOUND)
        # 如果都找不到，给出安装指导
        message(FATAL_ERROR 
            "Crow not found. Please install Crow framework:\n"
            "  macOS: brew install crowcpp/crow/crow\n"
            "  Ubuntu: sudo apt-get install libcrow-dev\n"
            "  Or build from source: https://github.com/CrowCpp/Crow")
    endif()
endif()

# Add executable
add_executable(compiler_api
    src/main.cpp
    src/core/lr0_parser.cpp
    src/core/slr1_parser.cpp
    src/core/regex_automata.cpp
    src/api/handlers.cpp
    src/api/routes.cpp
)

# Include directories
target_include_directories(compiler_api PRIVATE
    src
    include
)

# Link libraries
if(Crow_FOUND)
    target_link_libraries(compiler_api Crow::Crow)
else()
    target_link_libraries(compiler_api PkgConfig::CROW)
endif()

# Include directories
target_include_directories(compiler_api PRIVATE .)

# Enable debug info
set(CMAKE_BUILD_TYPE Debug)
