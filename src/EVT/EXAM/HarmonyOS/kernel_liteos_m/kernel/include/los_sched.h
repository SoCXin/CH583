/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LOS_SCHED_H
#define _LOS_SCHED_H

#include "los_task.h"
#include "los_interrupt.h"
#include "los_tick.h"
#include "los_sortlink.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_SCHED_MINI_PERIOD       (g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND_MINI)
#define OS_SCHED_MAX_RESPONSE_TIME OS_SORT_LINK_UINT64_MAX

extern BOOL g_taskScheduled;
typedef BOOL (*SchedScan)(VOID);

VOID OsSchedResetSchedResponseTime(UINT64 responseTime);

VOID OsSchedSetIdleTaskSchedParam(LosTaskCB *idleTask);

UINT32 OsSchedSwtmrScanRegister(SchedScan func);

VOID OsSchedUpdateExpireTime(VOID);

UINT64 OsSchedGetNextExpireTime(UINT64 startTime);

VOID OsSchedTaskDeQueue(LosTaskCB *taskCB);

VOID OsSchedTaskEnQueue(LosTaskCB *taskCB);

VOID OsSchedTaskWait(LOS_DL_LIST *list, UINT32 timeout);

VOID OsSchedTaskWake(LosTaskCB *resumedTask);

BOOL OsSchedModifyTaskSchedParam(LosTaskCB *taskCB, UINT16 priority);

VOID OsSchedDelay(LosTaskCB *runTask, UINT32 tick);

VOID OsSchedYield(VOID);

VOID OsSchedTaskExit(LosTaskCB *taskCB);

VOID OsSchedSuspend(LosTaskCB *taskCB);

BOOL OsSchedResume(LosTaskCB *taskCB);

VOID OsSchedTick(VOID);

UINT32 OsSchedInit(VOID);

VOID OsSchedStart(VOID);

BOOL OsSchedTaskSwitch(VOID);

LosTaskCB *OsGetTopTask(VOID);

VOID OsSchedTimeConvertFreq(UINT32 oldFreq);

STATIC INLINE UINT64 OsGetCurrSchedTimeCycle(VOID)
{
    return LOS_SysCycleGet();
}

STATIC INLINE BOOL OsCheckKernelRunning(VOID)
{
    return (g_taskScheduled && LOS_CHECK_SCHEDULE);
}

/**
 * @ingroup los_sched
 * @brief Get the time, in nanoseconds, remaining before the next tick interrupt response.
 *
 * @par Description:
 * This API is used to get the time, in nanoseconds, remaining before the next tick interrupt response.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval #time, in nanoseconds.
 * @par Dependency:
 * <ul><li>los_sched.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
extern UINT64 LOS_SchedTickTimeoutNsGet(VOID);

/**
 * @ingroup los_sched
 * @brief The system-provided tick interrupt handler.
 *
 * @par Description:
 * This API is used to wake up a task that is blocked by time.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_sched.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
extern VOID LOS_SchedTickHandler(VOID);

/**
 * @ingroup los_sched
 * @brief Trigger a system dispatch.
 *
 * @par Description:
 * This API is used to trigger a system dispatch.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_sched.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
extern VOID LOS_Schedule(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SCHED_H */
