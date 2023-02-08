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
 
.syntax unified

/*-----------------------------------------------------------*/
/*                     uxPortRaisePrivilege                  */
/*-----------------------------------------------------------*/
.section .text.uxPortRaisePrivilege, "ax", %progbits
.global uxPortRaisePrivilege
.type uxPortRaisePrivilege, %function

/* This function sets the elevated privileges */
uxPortRaisePrivilege:
    mrs r1, control
    mov r0, r1
    and r0, #1
    bic r1, #1
    msr control, r1
    bx lr

.size uxPortRaisePrivilege, .-uxPortRaisePrivilege

/*-----------------------------------------------------------*/
/*                     vPortResetPrivilege                   */
/*-----------------------------------------------------------*/
.section .text.vPortResetPrivilege, "ax", %progbits
.global vPortResetPrivilege
.type vPortResetPrivilege, %function

/* This function resets the elevated privileges */
vPortResetPrivilege:
    mrs r1, control
    orr r1, r0
    msr control, r1	
    bx lr

.size vPortResetPrivilege, .-vPortResetPrivilege

/*-----------------------------------------------------------*/
/*                     vPortStartFirstTask                   */
/*-----------------------------------------------------------*/
.section .text.vPortStartFirstTask, "ax", %progbits
.global vPortStartFirstTask
.type vPortStartFirstTask, %function

/* This function starts the first task by executing the Supervisor Call command. In 
this way the context of the first task is executed. */
vPortStartFirstTask:
    /* set the msp back to the start of the stack. */
    ldr r0, =__stack_end
    msr msp, r0
    
    /* globally enable interrupts. */
    cpsie i
    cpsie f
    dsb
    isb

    /* system call to start first task. */
    svc 0
    nop

.word __stack_end
.size vPortStartFirstTask, .-vPortStartFirstTask

/*-----------------------------------------------------------*/
/*                  vPortSetFirstTaskContext                 */
/*-----------------------------------------------------------*/
.section .text.vPortSetFirstTaskContext, "ax", %progbits
.global vPortSetFirstTaskContext
.type vPortSetFirstTaskContext, %function

/* This function sets the context of the first task and returns into this task. */
vPortSetFirstTaskContext:
    /* get the location of the pxCurrentTCB */
    ldr	r3, pxCurrentTCBConst2
    ldr r1, [r3]

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r0, [r1]

    /* pop the registers that are not automatically saved on exception entry and the critical nesting count */
    ldmia r0!, {r4-r11, r14}

    /* set the new process stack pointer */
    msr psp, r0
    isb

    /* enable the interrupts */
    mov r0, #0
    msr	basepri, r0

    /* reset privileges */
    mrs r0, control
    orr r0, #1
    msr control, r0

    /* return from handler mode */
    bx r14

    .align 4
pxCurrentTCBConst2: .word pxCurrentTCB

.size vPortSetFirstTaskContext, .-vPortSetFirstTaskContext

/*-----------------------------------------------------------*/
/*                         SVC_Handler                       */
/*-----------------------------------------------------------*/
.section .text.SVC_Handler, "ax", %progbits
.global SVC_Handler
.type SVC_Handler, %function

/* This is the SuperVisor Call Handler -> forward to C for sevice*/
SVC_Handler:
    mrs r0, control
    bic r0, #1
    msr control, r0
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b vPortServiceHandler

.size SVC_Handler, .-SVC_Handler

/*-----------------------------------------------------------*/
/*                       PendSV_Handler                      */
/*-----------------------------------------------------------*/
.section .text.PendSV_Handler, "ax", %progbits
.global PendSV_Handler
.type PendSV_Handler, %function

PendSV_Handler:
    /* get the process stack pointer */
    mrs r0, psp
    isb

    /* get the location of the pxCurrentTCB */
    ldr	r2, pxCurrentTCBConst
    ldr	r1, [r2]

    /* backup the fpu registers to the task stack */
    tst r14, #0x10
    it eq
    vstmdbeq r0!, {s16-s31}

    /* backup the core registers to the task stack */
    stmdb r0!, {r4-r11, r14}

    /* first item in pxCurrentTCB is the task top of stack - update with the new value */
    str r0, [r1]
    
    /* backup necessary scratch registers to stack - keep stack 8 bytes aligned */
    stmdb sp!, {r0, r2}

    /* disable interupts with lower priority as the max syscall to avoid inconsistent 
    data changes from interupts where FromISR functions get called */
    ldr r0, uxMaxSyscallPriorityConst
    ldr r0, [r0]
    msr basepri, r0
    dsb
    isb

    /* change the current context (address pointed by pxCurrentTCB) */
    bl vTaskSwitchContext

    /* reenable the interrupts */
    mov r0, #0
    msr basepri, r0	

    /* restore necessary scratch registers from stack */
    ldmia sp!, {r0, r2}

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r1, [r2]                  
    ldr r0, [r1]

    /* restore the core registers from the task stack */
    ldmia r0!, {r4-r11, r14}

    /* restore the fpu registers from the task stack */
    tst r14, #0x10
    it eq
    vldmiaeq r0!, {s16-s31}

    /* set the new process stack pointer */
    msr psp, r0
    isb

    /* return from handler mode */
    bx r14

    .align 4
pxCurrentTCBConst: .word pxCurrentTCB
    .align 4
uxMaxSyscallPriorityConst: .word uxMaxSyscallPriority

.size PendSV_Handler, .-PendSV_Handler

/*-----------------------------------------------------------*/
/*                        vPortSendChar                      */
/*-----------------------------------------------------------*/
.section .text.vPortSendChar, "ax", %progbits
.global vPortSendChar
.type vPortSendChar, %function

vPortSendChar:
    svc 55
    bx lr

/*-----------------------------------------------------------*/
/*                        uxPortCheckChar                    */
/*-----------------------------------------------------------*/
.section .text.uxPortCheckChar, "ax", %progbits
.global uxPortCheckChar
.type uxPortCheckChar, %function

uxPortCheckChar:
    svc 56
    bx lr

.size uxPortCheckChar, .-uxPortCheckChar

/*-----------------------------------------------------------*/
/*                        ucPortGetChar                      */
/*-----------------------------------------------------------*/
.section .text.ucPortGetChar, "ax", %progbits
.global ucPortGetChar
.type ucPortGetChar, %function

ucPortGetChar:
    svc 57
    bx lr

.size ucPortGetChar, .-ucPortGetChar

.end
