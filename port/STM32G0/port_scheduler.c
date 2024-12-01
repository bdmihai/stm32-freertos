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

#include "port.h"
#include "core_cm0plus.h"
#include "stm32g0xx.h"
#include "task.h"

/**
 * @brief This is the startup of the scheduler. All necessary HW for the RTOS is
 * prepared and configured.
 *
 */
BaseType_t xPortStartScheduler(void)
{
    /* make sure all interupts are disabled */
    portDISABLE_INTERRUPTS();

    /* set PendSV_IRQn, SVCall_IRQn and SysTick_IRQn priority */
    NVIC_SetPriority(SVCall_IRQn, configSVCall_INTERRUPT_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn, configPendSV_INTERRUPT_PRIORITY);

    /* start the timer that generates the tick ISR - interupts are disabled */
    vPortConfigureSysTick();

    /* start the first task */
    vPortStartFirstTask();

    /* Should never get here as the tasks will now be executing!  Call the task
    exit error function to prevent compiler warnings about a static function
    not being called in the case that the application writer overrides this
    functionality. Call vTaskSwitchContext() so link time optimisation does not
    remove the symbol. */
    vTaskSwitchContext();
    vPortTaskExitError();

    /* Should not get here! */
    return 0;
}

/**
 * @brief End scheduler.
 *
 */
void vPortEndScheduler(void)
{
    /* Not implemented in ports where there is nothing to return to.
    Artificially force an assert. */
}
