/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2023 Mihai Baneu                                             |
 │                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 08.Feb.2023  |
 |  Based on original M4 port from http://www.FreeRTOS.org                    |
 |___________________________________________________________________________*/

#pragma once

#include "stm32rtos.h"
#include "stm32g0xx.h"

/* used to catch tasks that attempt to return from their implementing function. */
extern void vPortTaskExitError(void);

/* setup the systick timer */
extern void vPortConfigureSysTick(void);

/* yield the next highest prio task from a priviledged mode */
extern void vPortYieldFromISR(void);

/* start the first task */
extern void vPortStartFirstTask(void);

/* set first task context */
extern void vPortSetFirstTaskContext(void);

/* svc C handler */
extern void vPortServiceHandler(uint32_t *svc_args);

/* privilege control and critical section handling */
extern void vPortRaisePrivilege(void);
extern void vPortResetPrivilege(void);

extern uint32_t ulPortDisableInterrupts();
extern void vPortEnableInterrupts();
extern void vPortRestoreInterrupts(uint32_t mask);

extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

/* stats gathering function */
extern void vPortConfigureStatsTimer(void);
extern uint32_t vPortGetStatsTimerValue(void);
