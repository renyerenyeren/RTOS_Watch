add_library(SEGGER STATIC)

target_sources(SEGGER PRIVATE
        src/SEGGER_RTT.c
        src/SEGGER_RTT_ASM_ARMv7M.s
        src/SEGGER_RTT_printf.c
        src/SEGGER_RTT_Syscalls_GCC.c
        src/SEGGER_SYSVIEW.c
        src/SEGGER_SYSVIEW_Config_FreeRTOS.c
        src/SEGGER_SYSVIEW_FreeRTOS.c
)

target_include_directories(SEGGER PUBLIC
        inc
)

target_link_libraries(SEGGER PRIVATE
        STM32_Drivers
)