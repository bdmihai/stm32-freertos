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

#include <stdarg.h>
#include <stdio.h>
#include "stm32rtos.h"
#include "stm32f4xx.h"
#include "task.h"
#include "port.h"

volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY; // Initialize as EMPTY

/**
 * @brief Yield the next task that needs to be executed.
 *
 * The PendSV interrupt is activated for changin the context.
 *
 */
void vPortYield(void)
{
    /* context switching is performed in the PendSV interrupt. Pend the PendSV interrupt. */
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

    /* Data Synchronization Barrier and Instruction Synchronization Barrier */
    __DSB();
    __ISB();
}

/**
 * @brief Handler for the Supervisor Call.
 * 
 * @param svc_args 
 */
void vPortServiceHandler(uint32_t *svc_args)
{
    uint8_t svc_number = ((char *) svc_args[6])[-2]; //Memory[(Stacked PC)-2]
    uint32_t priv;
    // r0 = svc_args[0];
    // r1 = svc_args[1];
    // r2 = svc_args[2];
    // r3 = svc_args[3];

    switch (svc_number)
    {
        case 0:
            vPortSetFirstTaskContext();
            break;
        case 1:
            priv = uxPortRaisePrivilege();
            vPortYield();
            vPortResetPrivilege(priv);
            break;
        case 2:
            vPortYield();
            break;
        case 55:
            priv = uxPortRaisePrivilege();
            ITM_SendChar(svc_args[0]);
            vPortResetPrivilege(priv);
            break;
        case 56:
            priv = uxPortRaisePrivilege();
            svc_args[0] = ITM_CheckChar();
            vPortResetPrivilege(priv);
            break;
        case 57:
            priv = uxPortRaisePrivilege();
            svc_args[0] = ITM_ReceiveChar();
            vPortResetPrivilege(priv);
            break;
    }
}

