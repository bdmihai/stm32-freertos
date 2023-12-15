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
 |  Author: Mihai Baneu                           Last modified: 26.Jan.2021  |
 |  Based on original M4 port from http://www.FreeRTOS.org                    |
 |___________________________________________________________________________*/

#include "stm32rtos.h"
#include "stm32f4xx.h"
#include "portmacro.h"
#include "port.h"
#include "task.h"


void vPortConfigureStatsTimer(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
}

uint32_t vPortGetStatsTimerValue(void)
{
    return DWT->CYCCNT;
}

uint32_t vPortGetRunTimeStats(TaskStatus_t *pxTaskStatusArray, UBaseType_t *uxArraySize)
{
    uint32_t ulTotalTime;

    /* Take a snapshot of the number of tasks in case it changes while this
    function is executing. */
    *uxArraySize = uxTaskGetNumberOfTasks();

    /* Allocate an array index for each task.  NOTE!  If
    configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
    equate to NULL. */
    pxTaskStatusArray = pvPortMalloc((*uxArraySize) * sizeof(TaskStatus_t));

    if( pxTaskStatusArray != NULL ) {
        *uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, (*uxArraySize), &ulTotalTime);
    }

    return ulTotalTime;
}