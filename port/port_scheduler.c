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
#include "task.h"

/* Each task maintains its own interrupt status in the critical nesting
variable. */
UBaseType_t uxCriticalNesting = 0XAAAAAAAA;

/* store th maxsyscall interupt priority into a variable for global access */
const uint32_t uxMaxSyscallPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY;

/**
 * @brief This is the startup of the scheduler. All necessary HW for the RTOS is
 * prepared and configured.
 *
 */
BaseType_t xPortStartScheduler(void)
{
    /* configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to 0.
    See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html */
    configASSERT(configMAX_SYSCALL_INTERRUPT_PRIORITY);

    /* set interrupt group priority (https://www.freertos.org/RTOS-Cortex-M3-M4.html) */
    /* 4 bits for pre-emption priority, 0 bits for subpriority */
    NVIC_SetPriorityGrouping(portNVIC_PRIORITYGROUP_4);

    /* make PendSV the lowest priority interrupts */
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), (configKERNEL_INTERRUPT_PRIORITY >> (8U - __NVIC_PRIO_BITS)), 0U));

    /* make SVC higher in priority than the syscall */
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), ((configMAX_SYSCALL_INTERRUPT_PRIORITY - 1) >> (8U - __NVIC_PRIO_BITS)), 0U));

#ifdef __VFP_FP__
    /* enable the FPU and configure automatic hardware state preservation and restoration
    for floating-point context */
    SCB->CPACR |= (0xFUL << 20);
    FPU->FPCCR |= (FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk);
#endif

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting = 0;

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
    configASSERT( uxCriticalNesting == 1000UL );
}

/**
 * @brief Enter a critical section.
 *
 * Interupts are disabled.
 *
 */
void __attribute__((section(".time_critical.vPortEnterCritical"))) vPortEnterCritical(void)
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;
}

/**
 * @brief Exit a critical section.
 *
 * If the nexting is 0 then the interupts get enabled.
 *
 */
void __attribute__((section(".time_critical.vPortExitCritical"))) vPortExitCritical(void)
{
    configASSERT(uxCriticalNesting);
    uxCriticalNesting--;
    if( uxCriticalNesting == 0 ) {
        portENABLE_INTERRUPTS();
    }
}
