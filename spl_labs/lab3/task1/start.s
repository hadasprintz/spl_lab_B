section .data
    SYS_WRITE   EQU 4
    SYS_READ    EQU 3
    STDOUT      EQU 1
    STDIN       EQU 0
    Infile  dd 0    ; Input file descriptor (stdin)
    Outfile dd 1    ; Output file descriptor (stdout)
    newline     db 10
    in_err_msg db "Error: Cannot open input file", 10, 0  ; Error message for input file
    out_err_msg db "Error: Cannot open output file", 10, 0  ; Error message for output file

section .text
    global _start
    global system_call
    global main
    extern strlen

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

main:
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]   ; Get first argument argc
    cmp esi, 1
    je no_arguments
    mov edi, [ebp+12]  ; Get third argument argv

next_argument:
    ; Print argv[i]
    add edi, 4          ; skip argv[0]
    mov ecx, [edi]   ; Push argv[i]
    push ecx
    call strlen        ; Get length of string
    mov edx, eax       ; Move string length to edx
    mov eax, SYS_WRITE ; Write syscall number
    mov ebx, STDOUT    ; Output file descriptor
    pop ecx             ; load string to ecx

    cmp word[ecx], "-i"
    je open_input
    cmp word[ecx], "-o"
    je open_output

    int 0x80           ; Call kernel

    ; Print newline
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, newline
    mov edx, 1
    int 0x80
    jmp next


next:
    dec esi
    cmp esi, 1
    je end_encode
    jmp next_argument

no_arguments:
    ; Call encode function when no arguments are provided
    call encode

    mov esp, ebp
    pop ebp
    ret

encode:
    ; Read character from stdin
read_char:
    mov eax, 3  ; Read syscall number
    mov ebx, dword[Infile] ; Input file descriptor
    mov ecx, esp ; Buffer to store character
    mov edx, 1  ; Number of bytes to read
    int 0x80    ; Call kernel

    ; Check if EOF
    cmp eax, 0
    je end_encode

    ; Check if character is in range 'A' to 'z'
    cmp byte [esp], 'A'
    jl write_char
    cmp byte [esp], 'z'
    jg write_char

    ; Encode character by adding 1
    inc byte [esp]
    jmp write_char

write_char:
    ; Write character to stdout
    mov eax, 4  ; Write syscall number
    mov ebx, dword [Outfile] ; Output file descriptor
    mov ecx, esp ; Pointer to character
    mov edx, 1  ; Number of bytes to write
    int 0x80    ; Call kernel

    ; Repeat the process for next character
    jmp read_char

end_encode:
    mov esp, ebp
    pop ebp
    ret

open_input:
    add ecx, 2                    ; Move to filename (skip "-i")
    mov ebx, ecx
    mov eax, 0x5
    mov ecx, 0
    int 0x80
    cmp eax, -1
    je input_file_error
    mov dword[Infile], eax

    jmp next


open_output:
    add eax, 2                    ; Move to filename
    mov ebx, eax
    mov eax, 0x5
    mov ecx, 0x241
    mov edx, 0x1B6
    int 0x80
    cmp eax, -1
    je output_file_error
    mov dword[Outfile], eax

    jmp next

input_file_error:
    ; Print error message for input file
    mov ecx, in_err_msg
    push ecx
    call strlen
    mov edx, eax
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    pop ecx
    int 0x80
    ; Print newline
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, newline
    mov edx, 1
    int 0x80
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, newline
    mov edx, 1
    int 0x80

    jmp next                   ; Jump to encode after error handling

output_file_error:
    ; Print error message for output file
    mov ecx, out_err_msg
    push ecx
    call strlen
    mov edx, eax
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    pop ecx
    int 0x80
    ; Print newline
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, newline
    mov edx, 1
    int 0x80
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, newline
    mov edx, 1
    int 0x80



    jmp next                    ; Jump to encode after error handling


section .note.GNU-stack
    align 4
    dd 0x474e552d
    dd 0x1002
    dd 0x1
