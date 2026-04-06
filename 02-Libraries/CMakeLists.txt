cmake_minimum_required(VERSION 3.10)

# 引入 Common-DWT 子模块
add_subdirectory(02_1-Common/DWT_delay)
# 引入 Common-I2C 子模块
add_subdirectory(02_1-Common/Soft_IIC)
# 引入 Common-dlist 子模块
add_subdirectory(02_1-Common/DList)

# 引入 ThirdParty-elog 子模块
add_subdirectory(02_2-ThirdParty/elog)
# 引入 ThirdParty-SEGGER 子模块
add_subdirectory(02_2-ThirdParty/SEGGER)
# 引入 ThirdParty-SFUD 子模块
add_subdirectory(02_2-ThirdParty/SFUD)

# ------------------------------
# 定义 Libraries 接口库（用于聚合所有 Libraries 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(libraries INTERFACE)

# 链接模块（公开链接）
# 任何链接了 "libraries" 的目标，都会自动链接
target_link_libraries(libraries INTERFACE
        ##Common
        DWT_delay
        soft_i2c
        dlist
        ##ThirdParty
        elog
        SEGGER
        SfudLib
)

# 暴露 libraries 公共头文件路径（如果 libraries 目录下有顶层的公共头文件）
 target_include_directories(libraries INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/inc
 )