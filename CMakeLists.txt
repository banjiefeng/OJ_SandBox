# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (xdoj)

set(CMAKE_CXX_STANDARD 17)

# 查找头文件的路径
include_directories(./include)
include_directories(./CodeSandbox)
include_directories(./CodeSandbox/languageRunners ./CodeSandbox/sandboxRuntime ./CodeSandbox/scheduler ./CodeSandbox/service ./CodeSandbox/workerPools)
# 查找头文件的路径 mongocxx
include_directories(/usr/local/include/mongocxx/v_noabi /usr/local/include/libmongoc-1.0)
include_directories(/usr/local/include/bsoncxx/v_noabi /usr/local/include/libbson-1.0)
# 查找头文件的路径 redis-plus-plus
include_directories(hiredis sw)

# 查找库的路径
link_directories(${CMAKE_SOURCE_DIR}/lib)
# 查找库的路径
link_directories(/usr/local/lib)

# 查找当前目录下的所有源文件

file(GLOB_RECURSE DIR_SRCS1 
    "./CodeSandbox/*.cpp"
)

# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(./src DIR_SRCS2)

# 指定生成目标
add_executable(xdoj ${DIR_SRCS1} ${DIR_SRCS2})

# 链接库 
target_link_libraries(xdoj jsoncpp pthread mongocxx bsoncxx judger hiredis redis++ seccomp)

# 指定编译参数
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -std=c++17")