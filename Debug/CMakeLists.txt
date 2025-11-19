cmake_minimum_required(VERSION 3.10)


# ------------------------------
# 1. 引入子模块（关键！执行子目录的 CMakeLists.txt）
# ------------------------------
add_subdirectory(elog)    # 生成 elog 目标
add_subdirectory(SEGGER)  # 生成 SEGGER 目标

# ------------------------------
# 2. 定义 debug 库（整合所有调试功能）
# ------------------------------
add_library(user_debug STATIC
        user_debug.c  # debug 模块自身的逻辑（若有）
)

# ------------------------------
# 3. 链接子模块并暴露接口
# ------------------------------
target_link_libraries(user_debug PUBLIC
        elog    # 公开链接：依赖 debug 的目标会自动获得 elog 的头文件和符号
        SEGGER  # 公开链接：依赖 debug 的目标会自动获得 SEGGER 的头文件和符号
)

# （可选）暴露 debug 模块自身的头文件（若有）
target_include_directories(user_debug PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
)