add_library(elog STATIC)

target_sources(elog PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/elog.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/elog_async.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/elog_buf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/elog_port.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/elog_utils.c
)

target_include_directories(elog PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(elog PRIVATE
        STM32_Drivers
        SEGGER
)
