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

#define INITIAL_EXC_RETURN  0XFFFFFFFD

/*-----------------------------------------------------------*/
/*                      vPortRaisePrivilege                  */
/*-----------------------------------------------------------*/
.section .time_critical.vPortRaisePrivilege, "ax", %progbits
.global vPortRaisePrivilege
.type vPortRaisePrivilege, %function
vPortRaisePrivilege:
    mrs r0, control
    movs r1, #1
    bics r0, r1
    msr control, r0
    bx lr

.size vPortRaisePrivilege, .-vPortRaisePrivilege

/*-----------------------------------------------------------*/
/*                     vPortResetPrivilege                   */
/*-----------------------------------------------------------*/
.section .time_critical.vPortResetPrivilege, "ax", %progbits
.global vPortResetPrivilege
.type vPortResetPrivilege, %function
vPortResetPrivilege:
    mrs r0, control
    movs r1, #1
    orrs r0, r1
    msr control, r0
    bx lr

.size vPortResetPrivilege, .-vPortResetPrivilege

/*-----------------------------------------------------------*/
/*                   vPortDisableInterrupts                  */
/*-----------------------------------------------------------*/
.section .time_critical.ulPortDisableInterrupts, "ax", %progbits
.global ulPortDisableInterrupts
.type ulPortDisableInterrupts, %function
ulPortDisableInterrupts:
    mrs r0, PRIMASK
    cpsid i
    bx lr

.size ulPortDisableInterrupts, .-ulPortDisableInterrupts

/*-----------------------------------------------------------*/
/*                   vPortRestoreInterrupts                  */
/*-----------------------------------------------------------*/
.section .time_critical.vPortEnableInterrupts, "ax", %progbits
.global vPortEnableInterrupts
.type vPortEnableInterrupts, %function
vPortEnableInterrupts:
    cpsie i
    bx lr

.size vPortEnableInterrupts, .-vPortEnableInterrupts

/*-----------------------------------------------------------*/
/*                    vPortRestoreInterrupts                 */
/*-----------------------------------------------------------*/
.section .time_critical.vPortRestoreInterrupts, "ax", %progbits
.global vPortRestoreInterrupts
.type vPortRestoreInterrupts, %function
vPortRestoreInterrupts:
    msr PRIMASK, r0
    bx lr

.size vPortRestoreInterrupts, .-vPortRestoreInterrupts

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
    cpsie f
    dsb
    isb

    /* system call to start first task. */
    svc 0

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
    ldr	r3, =pxCurrentTCB
    ldr r1, [r3]

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r0, [r1]

    /* discard everything up to r0 */
    adds r0, #32

    /* set the new process stack pointer */
    msr psp, r0
    isb

    /* reset privileges - nPRIV set to 1 for unpriveledged execution in Thread Mode */
    mrs r0, control
    movs r1, #1
    orrs r0, r1
    msr control, r0

    /* return from handler mode to thread mode: basically pops r0-r4, r12, r14(lr), r15(pc), psr */
    ldr r0, =INITIAL_EXC_RETURN
    bx r0

.align 4
vPortSetFirstTaskContext_Locals: 
    .word pxCurrentTCB

.size vPortSetFirstTaskContext, .-vPortSetFirstTaskContext

/*-----------------------------------------------------------*/
/*                       vPortSVCHandler                     */
/*-----------------------------------------------------------*/
.section .time_critical.vPortSVCHandler, "ax", %progbits
.global vPortSVCHandler
.type vPortSVCHandler, %function
vPortSVCHandler:
    /* check what stack pointer to use */
    mov  r1, lr
    movs r2, #4
    tst r1, r2
    bne 2f
1:
    mrs r0, msp
    b 3f
2:
    mrs r0, psp
3:
    b vPortServiceHandler

.size vPortSVCHandler, .-vPortSVCHandler

/*-----------------------------------------------------------*/
/*                     vPortPendSVHandler                    */
/*-----------------------------------------------------------*/
.section .time_critical.vPortPendSVHandler, "ax", %progbits
.global vPortPendSVHandler
.type vPortPendSVHandler, %function

vPortPendSVHandler:
    /* get the process stack pointer */
    mrs r0, psp

    /* get the location of the pxCurrentTCB */
    ldr	r2, PendSV_Handler_Locals
    ldr	r1, [r2]

    /* go down in the stack in order to use the increment after function - M0+ is missing STMDB */
    subs r0, r0, #32 

    /* backup the lower registers to the task stack */
    stmia r0!, {r4-r7}

    /* backup the upper registers to the task stack */
    mov r4, r8
    mov r5, r9
    mov r6, r10
    mov r7, r11
    stmia r0!, {r4-r7}

    /* change the current context (address pointed by pxCurrentTCB) */
    push {r3, r14}
    cpsid i
    bl vTaskSwitchContext
    cpsie i
    pop {r2, r3} /* lr goes in r3. r2 now holds tcb pointer. */

    /* first item in pxCurrentTCB is the task top of stack */
    ldr r1, [r2]                  
    ldr r0, [r1]

    /* restore the upper registers from the task stack */
    adds r0, r0, #16
    ldmia r0!, {r4-r7}
    mov r8, r4
    mov r9, r5
    mov r10, r6
    mov r11, r7

    /* this is now the process stack pointer */
    msr psp, r0

    /* restore the lower registers from the task stack */
    subs r0, r0, #32
    ldmia r0!, {r4-r7}

    /* return from handler mode */
    bx r3

.align 4
PendSV_Handler_Locals: 
    .word pxCurrentTCB 

.size vPortPendSVHandler, .-vPortPendSVHandler

.end
