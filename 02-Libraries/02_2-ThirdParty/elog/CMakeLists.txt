add_library(elog STATIC)

target_sources(elog PRIVATE
        src/elog.c
        src/elog_async.c
        src/elog_buf.c
        src/elog_port.c
        src/elog_utils.c
)

target_include_directories(elog PUBLIC
        inc
)

target_link_libraries(elog PRIVATE
        STM32_Drivers
        SEGGER
)
