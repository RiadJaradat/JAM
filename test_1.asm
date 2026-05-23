section .data
    msg1: db "True", 10
    len1: equ $-msg1
    msg2: db "False", 10
    len2: equ $-msg2
    msg3: db "Done", 10
    len3: equ $-msg3
section .bss
    isRunning resb 1 ; for boolean variables
section .text
    global _start
    _start:
        mov byte [isRunning], 1
        jmp main

    main:
        movzx eax, byte [isRunning]
        mov ebx, 1
        cmp eax, ebx
        jne .else1
        mov rax, 1
        mov rdi, 1
        mov rsi, msg1
        mov rdx, len1
        syscall
        jmp .L1
    .else1:
        mov rax, 1
        mov rdi, 1
        mov rsi, msg2
        mov rdx, len2
        syscall
        jmp .L1
    .L1:
        mov rax, 1
        mov rdi, 1
        mov rsi, msg3
        mov rdx, len3
        syscall

        mov rax, 60
        mov rdi, 0
        syscall

