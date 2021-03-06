/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2021 Mihai Baneu                                             |
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
#include "stm32f4xx.h"
#include "task.h"
#include "portmacro.h"
#include "port.h"

/**
 * @brief Blink n times.
 *
 * This is a dummy function as the real blink should be implemented by application.
 *
 */
__attribute__((weak)) void blink(const int n)
{
    (void)(n);

    while (1) {
        __asm volatile("nop");
    }
}

/**
 * @brief Blink n times.
 *
 * @param n how many times to blink.
 */
void vPortAssert(const int n)
{
    blink(n);
}

/**
 * @brief Used to catch tasks that attempt to return from their implementing function.
 *
 * A function that implements a task must not exit or attempt to return to its caller
 * as there is nothing to return to. If a task wants to exit it should instead call
 * vTaskDelete.
 */
void vPortTaskExitError( void )
{
    volatile uint32_t ulDummy = 0UL;

    /* Artificially force an assert() to be triggered if configASSERT() is defined,
    then stop here so application writers can catch the error. */
    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();
    while( ulDummy == 0 ) {
        /* ulDummy is used purely to quieten other warnings
        about code appearing after this function is called - making ulDummy
        volatile makes the compiler think the function could return and
        therefore not output an 'unreachable code' warning for code that appears
        after it. */
    }
}

/**
 * @brief Blink two short pulses if malloc fails.
 *
 * This function will only be called if configUSE_MALLOC_FAILED_HOOK is set to
 * 1 in FreeRTOSConfig.h.  It is a hook function that will get called if a call
 * to pvPortMalloc() fails. pvPortMalloc() is called internally by the kernel
 * whenever a task, queue, timer or semaphore is created.  It is also called by
 * various parts of the demo application.  If heap_1.c or heap_2.c are used,
 * then the size of the heap available to pvPortMalloc() is defined by
 * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API
 * function can be used to query the size of free heap space that remains
 * (although it does not provide information on how the remaining heap might be
 * fragmented).
 */
void vApplicationMallocFailedHook()
{
    blink(2);
}

/**
 * @brief Run time stack overflow checking hook function.
 *
 * Run time stack overflow checking is performed if configCHECK_FOR_STACK_OVERFLOW
 * is defined to 1 or 2.  This hook function is called if a stack overflow is detected.
 * \param[in] pxTask Task handle
 * \param[in] pcTaskName Task name
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;
    blink(3);
}

/**
 * @brief Validation of the Interupt priority.
 *
 * This function checks that the interupts that call ...FromISR rtos functions have
 * the correct prio set: smaller or eq to
 *
 */
void vPortValidateInterruptPriority( void )
{
    uint32_t ulCurrentInterrupt;
    uint8_t ucCurrentPriority;

    /* obtain the number of the currently executing interrupt. */
    ulCurrentInterrupt = __get_IPSR();

    /* exceptions 1-15 are reserved for cortex M - only external interupts are considered */
    if( ulCurrentInterrupt >= 16 ) {
        /* get the interrupt's priority. */
        ucCurrentPriority = NVIC_GetPriority(ulCurrentInterrupt - 16);

        /* The following assertion will fail if a service routine (ISR) for
        an interrupt that has been assigned a priority above
        configMAX_SYSCALL_INTERRUPT_PRIORITY calls an ISR safe FreeRTOS API
        function.  ISR safe FreeRTOS API functions must *only* be called
        from interrupts that have been assigned a priority at or below
        configMAX_SYSCALL_INTERRUPT_PRIORITY.

        Numerically low interrupt priority numbers represent logically high
        interrupt priorities, therefore the priority of the interrupt must
        be set to a value equal to or numerically *higher* than
        configMAX_SYSCALL_INTERRUPT_PRIORITY.

        Interrupts that use the FreeRTOS API must not be left at their
        default priority of zero as that is the highest possible prioritipsry,
        which is guaranteed to be above configMAX_SYSCALL_INTERRUPT_PRIORITY,
        and therefore also guaranteed to be invalid.

        FreeRTOS maintains separate thread and ISR API functions to ensure
        interrupt entry is as fast and simple as possible.

        The following links provide detailed information:
        http://www.freertos.org/RTOS-Cortex-M3-M4.html
        http://www.freertos.org/FAQHelp.html */
        configASSERT(ucCurrentPriority >= (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8U - __NVIC_PRIO_BITS)));
    }

    /* Priority grouping:  The interrupt controller (NVIC) allows the bits
    that define each interrupt's priority to be split between bits that
    define the interrupt's pre-emption priority bits and bits that define
    the interrupt's sub-priority.  For simplicity all bits must be defined
    to be pre-emption priority bits.  The following assertion will fail if
    this is not the case (if some bits represent a sub-priority).

    If the application only uses CMSIS libraries for interrupt
    configuration then the correct setting can be achieved on all Cortex-M
    devices by calling NVIC_SetPriorityGrouping( 0 ); before starting the
    scheduler.  Note however that some vendor specific peripheral libraries
    assume a non-zero priority group setting, in which cases using a value
    of zero will result in unpredictable behaviour. */
    configASSERT(NVIC_GetPriorityGrouping() < __NVIC_PRIO_BITS);
}
