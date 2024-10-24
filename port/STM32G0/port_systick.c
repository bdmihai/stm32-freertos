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

#include "port.h"
#include "portmacro.h"
#include "core_cm0plus.h"
#include "task.h"

extern unsigned int system_cpu_f();

/**
 * @brief Setup the systick timer to generate the tick interrupts at the required frequency.
 *
 */
void vPortConfigureSysTick(void)
{
    /* stop and clear the SysTick. */
    SysTick->CTRL = 0UL;
    SysTick->VAL  = 0UL;

    /* set the SysTick interrupt to the lowest priority in the kernel */
    NVIC_SetPriority(SysTick_IRQn, configSysTick_INTERRUPT_PRIORITY);

    /* configure SysTick to interrupt at the requested rate. */
    SysTick->LOAD = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
    SysTick->CTRL = ( SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );
}

/**
 * @brief Handler for the SysTick interupt.
 *
 * The SysTick runs at the lowest interrupt priority, so when this interrupt
 * executes all interrupts must be unmasked.  There is therefore no need to
 * save and then restore the interrupt mask value as its value is already
 * known.
 */
void __attribute__((section(".time_critical.SysTick_Handler"))) SysTick_Handler(void)
{
    portDISABLE_INTERRUPTS();
    {
        /* increment the RTOS tick. If necessary trigger a context switch using
        the PendSV interrupt */
        if( xTaskIncrementTick() != pdFALSE ) {
            /* a context switch is required to allow the next task to run */
            vPortYieldFromISR();
        }
    }
    portENABLE_INTERRUPTS();
}
