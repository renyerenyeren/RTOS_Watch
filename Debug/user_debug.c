//
// Created by capting on 2025/10/28.
//

#include "user_debug.h"

#include "elog.h"
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"


void user_debug_init(void)
{
  SEGGER_RTT_Init();
  SEGGER_SYSVIEW_Conf();
  ElogErrCode ret = elog_init();
  if (ELOG_NO_ERR != ret) SEGGER_RTT_printf(0,"elog is ng");
  elog_set_text_color_enabled(true);

  elog_set_fmt(ELOG_LVL_ASSERT ,ELOG_FMT_ALL&~(ELOG_FMT_P_INFO|ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_ERROR  ,ELOG_FMT_ALL&~(ELOG_FMT_P_INFO|ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_WARN   ,ELOG_FMT_ALL&~(ELOG_FMT_P_INFO|ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_INFO   ,ELOG_FMT_ALL&~(ELOG_FMT_P_INFO|ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_DEBUG  ,ELOG_FMT_ALL&~(ELOG_FMT_P_INFO|ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_VERBOSE,(ELOG_FMT_LVL|
                                          ELOG_FMT_TAG));

  elog_start();
  elog_flush();
}


