.section .note.GNU-stack

@ Here is the RPI code

@ Here's function: _^main^_^main

.balign 4
.global main
.section .text

main:
         push {r4-r10, fp, lr}
         add fp, sp, #32
         sub sp, sp, #4
main$L123: 
         mov r0, #4
         bl malloc
         mov r5, r0
         mov r0, #0
         str r0, [r5]
         mov r0, #24
         bl malloc
         mov r10, r0
         str r10, [fp, #-36]
         mov r0, #5
         ldr r10, [fp, #-36]
         str r0, [r10]
         mov r0, #24
         bl malloc
         mov r8, r0
         mov r0, #5
         str r0, [r8]
         add r0, r8, #4
         mov r1, #1
         str r1, [r0]
         add r0, r8, #8
         mov r1, #2
         str r1, [r0]
         add r0, r8, #12
         mov r1, #3
         str r1, [r0]
         add r0, r8, #16
         mov r1, #4
         str r1, [r0]
         add r0, r8, #20
         mov r1, #5
         str r1, [r0]
         mov r4, #0
         mov r0, #8
         bl malloc
         mov r7, r0
         mov r0, #16
         bl malloc
         mov r1, #3
         str r1, [r0]
         add r1, r0, #4
         mov r2, #1
         str r2, [r1]
         add r1, r0, #8
         mov r2, #2
         str r2, [r1]
         add r1, r0, #12
         mov r2, #3
         str r2, [r1]
         str r0, [r7, #0]
         add r0, r7, #4
         ldr r1, =D$m
         str r1, [r0]
         mov r0, r5
         bl getarray
         mov r6, r0
main$L102: 
         cmp r4, r6
         blt main$L108
main$L103: 
         ldr r9, [fp, #-36]
         ldr r5, [r9]
         ldr r0, [r8]
         cmp r5, r0
         bne main$L111
main$L112: 
         add r0, r5, #1
         mov r1, #4
         mul r0, r0, r1
         bl malloc
         mov r4, r0
         str r5, [r4]
         mov r0, #4
         add r1, r5, #1
         mov r2, #4
         mul r2, r1, r2
main$L117: 
         cmp r0, r2
         blt main$L116
main$L115: 
         ldr r5, [r4]
         add r0, r5, #1
         mov r1, #4
         mul r0, r0, r1
         bl malloc
         str r5, [r0]
         mov r1, #4
         add r2, r5, #1
         mov r3, #4
         mul r3, r2, r3
main$L122: 
         cmp r1, r3
         blt main$L121
main$L120: 
         ldr r1, [r7, #4]
         mov r0, r7
         blx r1
         mov r0, #0
         sub sp, fp, #32
         pop {r4-r10, fp, pc}
main$L106: 
         mov r0, #-1
         bl exit
main$L107: 
         add r0, r4, #1
         mov r1, #4
         mul r0, r0, r1
         add r0, r5, r0
         ldr r0, [r0]
         bl putint
         mov r0, #32
         bl putch
         add r4, r4, #1
         b main$L102
main$L108: 
         ldr r0, [r5]
         cmp r4, r0
         bge main$L106
         b main$L107
main$L111: 
         mov r0, #-1
         bl exit
main$L116: 
         add r5, r4, r0
         ldr r9, [fp, #-36]
         add r1, r9, r0
         ldr r3, [r1]
         add r1, r8, r0
         ldr r1, [r1]
         add r1, r3, r1
         str r1, [r5]
         add r0, r0, #4
         b main$L117
main$L121: 
         add r5, r0, r1
         add r2, r4, r1
         ldr r2, [r2]
         mov r6, #0
         sub r2, r6, r2
         str r2, [r5]
         add r1, r1, #4
         b main$L122

@ Here's function: D^m

.balign 4
.global D$m
.section .text

D$m:
         push {r4-r10, fp, lr}
         add fp, sp, #32
D$m$L100: 
         ldr r0, [r0, #0]
         sub sp, fp, #32
         pop {r4-r10, fp, pc}

.global malloc
.global getint
.global putint
.global putch
.global putarray
.global getch
.global getarray
.global starttime
.global stoptime
