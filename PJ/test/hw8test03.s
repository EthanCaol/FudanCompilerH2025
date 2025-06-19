.section .note.GNU-stack

@ Here is the RPI code

@ Here's function: _^main^_^main

.balign 4
.global main
.section .text

main:
         push {r4-r10, fp, lr}
         add fp, sp, #32
main$L122: 
         mov r0, #28
         bl malloc
         mov r6, r0
         mov r0, #6
         str r0, [r6]
         add r0, r6, #4
         mov r1, #1
         str r1, [r0]
         add r0, r6, #8
         mov r1, #2
         str r1, [r0]
         add r0, r6, #12
         mov r1, #3
         str r1, [r0]
         add r0, r6, #16
         mov r1, #4
         str r1, [r0]
         add r0, r6, #20
         mov r1, #5
         str r1, [r0]
         add r0, r6, #24
         mov r1, #6
         str r1, [r0]
         mov r0, #28
         bl malloc
         mov r5, r0
         mov r0, #6
         str r0, [r5]
         add r0, r5, #4
         mov r1, #1
         str r1, [r0]
         add r0, r5, #8
         mov r1, #2
         str r1, [r0]
         add r0, r5, #12
         mov r1, #3
         str r1, [r0]
         add r0, r5, #16
         mov r1, #4
         str r1, [r0]
         add r0, r5, #20
         mov r1, #5
         str r1, [r0]
         add r0, r5, #24
         mov r1, #6
         str r1, [r0]
         mov r4, #0
         ldr r7, [r6]
         ldr r0, [r5]
         cmp r7, r0
         bne main$L102
main$L103: 
         add r0, r7, #1
         mov r1, #4
         mul r0, r0, r1
         bl malloc
         str r7, [r0]
         mov r1, #4
         add r2, r7, #1
         mov r3, #4
         mul r3, r2, r3
main$L108: 
         cmp r1, r3
         blt main$L107
main$L106: 
         mov r6, r0
main$L111: 
         ldr r0, [r6]
         cmp r4, r0
         blt main$L121
main$L112: 
         mov r0, #10
         bl putch
         mov r0, r4
         sub sp, fp, #32
         pop {r4-r10, fp, pc}
main$L102: 
         mov r0, #-1
         bl exit
main$L107: 
         add r8, r0, r1
         add r2, r6, r1
         ldr r7, [r2]
         add r2, r5, r1
         ldr r2, [r2]
         add r2, r7, r2
         str r2, [r8]
         add r1, r1, #4
         b main$L108
main$L115: 
         mov r0, #-1
         bl exit
main$L116: 
         add r0, r4, #1
         mov r1, #4
         mul r0, r0, r1
         add r0, r6, r0
         ldr r1, [r0]
         ldr r0, [r5]
         cmp r4, r0
         bge main$L119
main$L120: 
         add r0, r4, #1
         mov r2, #4
         mul r0, r0, r2
         add r0, r5, r0
         ldr r0, [r0]
         add r0, r1, r0
         bl putint
         mov r0, #32
         bl putch
         add r4, r4, #1
         b main$L111
main$L119: 
         mov r0, #-1
         bl exit
main$L121: 
         ldr r0, [r6]
         cmp r4, r0
         bge main$L115
         b main$L116

.global malloc
.global getint
.global putint
.global putch
.global putarray
.global getch
.global getarray
.global starttime
.global stoptime
