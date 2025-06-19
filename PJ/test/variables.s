.section .note.GNU-stack

@ Here is the RPI code

@ Here's function: _^main^_^main

.balign 4
.global main
.section .text

main:
         push {r4-r10, fp, lr}
         add fp, sp, #32
         sub sp, sp, #28
main$L100: 
         mov r4, #0
         mov r3, #1
         mov r2, #2
         mov r10, #3
         str r10, [fp, #-36]
         mov r10, #4
         str r10, [fp, #-40]
         mov r10, #5
         str r10, [fp, #-44]
         mov r10, #6
         str r10, [fp, #-48]
         mov r10, #7
         str r10, [fp, #-52]
         mov r10, #8
         str r10, [fp, #-56]
         mov r8, #9
         mov r7, #10
         mov r6, #11
         add r0, r4, r3
         add r0, r0, r2
         ldr r10, [fp, #-36]
         add r0, r0, r10
         ldr r10, [fp, #-40]
         add r0, r0, r10
         ldr r10, [fp, #-44]
         add r0, r0, r10
         ldr r10, [fp, #-48]
         add r0, r0, r10
         ldr r10, [fp, #-52]
         add r0, r0, r10
         ldr r10, [fp, #-56]
         add r0, r0, r10
         add r0, r0, r8
         add r0, r0, r7
         add r5, r0, r6
         mul r1, r5, r4
         mul r0, r5, r3
         add r1, r1, r0
         mul r0, r5, r2
         add r0, r1, r0
         ldr r10, [fp, #-36]
         mul r1, r5, r10
         add r1, r0, r1
         ldr r10, [fp, #-40]
         mul r0, r5, r10
         add r1, r1, r0
         ldr r10, [fp, #-44]
         mul r0, r5, r10
         add r1, r1, r0
         ldr r10, [fp, #-48]
         mul r0, r5, r10
         add r1, r1, r0
         ldr r10, [fp, #-52]
         mul r0, r5, r10
         add r0, r1, r0
         ldr r10, [fp, #-56]
         mul r1, r5, r10
         add r1, r0, r1
         mul r0, r5, r8
         add r1, r1, r0
         mul r0, r5, r7
         add r1, r1, r0
         mul r0, r5, r6
         add r10, r1, r0
         str r10, [fp, #-60]
         add r0, r4, r3
         ldr r10, [fp, #-44]
         sdiv r1, r2, r10
         add r0, r0, r1
         mul r1, r6, r7
         add r0, r0, r1
         bl putint
         mov r0, r5
         bl putint
         ldr r9, [fp, #-60]
         mov r0, r9
         bl putint
         ldr r9, [fp, #-52]
         ldr r10, [fp, #-48]
         mul r0, r9, r10
         ldr r9, [fp, #-40]
         add r0, r9, r0
         sub r0, r0, r8
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
