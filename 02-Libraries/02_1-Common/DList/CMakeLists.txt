cmake_minimum_required(VERSION 3.10)

# 定义 DList 驱动库
add_library(dlist STATIC)

# 添加 DList 模块的源文件
target_sources(dlist PRIVATE
        dlist.c
)

# 暴露 DList 模块的头文件路径
target_include_directories(dlist PUBLIC
        .
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(dlist PRIVATE

)