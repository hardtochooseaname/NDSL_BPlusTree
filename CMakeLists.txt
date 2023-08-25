cmake_minimum_required(VERSION 3.20)  

# Create a project
# and top level 'CMakeLists.txt' should be in the root dir of this project
project(bplustree) 

# 添加头文件路径
include_directories(include)

# 添加 -g 选项以启用调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Add source files and specify a target executable file
# that cmake will generate for this project
add_executable(bplustree 
                bplustree_final.cxx 
                src/bpt_test.cxx 
                src/tree.cxx 
                src/node.cxx)