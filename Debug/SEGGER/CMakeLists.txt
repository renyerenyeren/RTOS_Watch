add_library(SEGGER STATIC)

target_sources(SEGGER PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT_ASM_ARMv7M.s
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT_printf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT_Syscalls_GCC.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_SYSVIEW.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_SYSVIEW_Config_FreeRTOS.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_SYSVIEW_FreeRTOS.c
)

target_include_directories(SEGGER PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(SEGGER PRIVATE
        STM32_Drivers
)