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
 |  Author: Mihai Baneu                           Last modified: 08.Jan.2021  |
 |  Based on original M4 port from http://www.FreeRTOS.org                    |
 |___________________________________________________________________________*/

#include "stm32rtos.h"
#include "portmacro.h"
#include "port.h"

/**
 * This function will be called by each tick interrupt if configUSE_TICK_HOOK
 * is set to 1 in FreeRTOSConfig.h.  User code can be added here, but the tick
 * hook is called from an interrupt context, so code must not attempt to block,
 * and only the interrupt safe FreeRTOS API functions can be used (those that
 * end in FromISR()).
 */
__attribute__((weak)) void vApplicationTickHook() { }

/**
 *  vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
 *  to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
 * task.  It is essential that code added to this hook function never attempts
 * to block in any way (for example, call xQueueReceive() with a block time
 * specified, or call vTaskDelay()).  If the application makes use of the
 * vTaskDelete() API function (as this demo application does) then it is also
 * important that vApplicationIdleHook() is permitted to return to its calling
 * function, because it is the responsibility of the idle task to clean up
 * memory allocated by the kernel to any task that has since been deleted.
 *
 */
__attribute__((weak)) void vApplicationIdleHook(void) { }

/**
 * The RTOS daemon task is the same as the Timer Service Task. Sometimes it is 
 * referred to as the daemon task because the task is now used for more than 
 * just servicing timers.
 * If configUSE_DAEMON_TASK_STARTUP_HOOK is set to 1 in FreeRTOSConfig.h then the 
 * Daemon Task Startup Hook will be called as soon as the Daemon Task starts 
 * executing for the first time. This is useful if the application includes 
 * initialisation code that would benefit from executing after the scheduler has 
 * been started, which allows the initialisation code to make use of the RTOS 
 * functionality.
 * 
 * If configUSE_DAEMON_TASK_STARTUP_HOOK is set to 1 then the application writer 
 * must provide an implementation of the Daemon Task startup hook function.
 */
__attribute__((weak)) void vApplicationDaemonTaskStartupHook(void) {}