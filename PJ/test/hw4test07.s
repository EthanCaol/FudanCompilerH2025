.section .note.GNU-stack

@ Here is the RPI code

@ Here's function: _^main^_^main

.balign 4
.global main
.section .text

main:
         push {r4-r10, fp, lr}
         add fp, sp, #32
main$L115: 
         mov r0, #19
main$L100: 
         mov r1, #1
         mov r0, #0
         cmp r1, r0
         bne main$L112
main$L101: 
         mov r0, #0
         sub sp, fp, #32
         pop {r4-r10, fp, pc}
main$L112: 
         b main$L100

.global malloc
.global getint
.global putint
.global putch
.global putarray
.global getch
.global getarray
.global starttime
.global stoptime
