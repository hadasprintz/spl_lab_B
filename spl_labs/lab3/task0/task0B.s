section .rodata
    message db "Hello world", 10

section .text
    global _start

_start:
    mov eax, 4 ; WRITE
    mov ebx, 1 ; STDOUT
    mov ecx, message ; load message
    mov edx, 12 ; len
    int 0x80 ; write message to STDOUT
    mov eax, 1 ; exit
    mov ebx, 0 ; status
    int 0x80 ; exit 0