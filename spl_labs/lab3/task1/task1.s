section .rodata
    out_fmt db "%s", 10, 0  ; Format string for printf
    in_err_msg db "Error: Cannot open input file", 10, 0  ; Error message for input file
    out_err_msg db "Error: Cannot open output file", 10, 0  ; Error message for output file

section .data
    Infile  dd 0    ; Input file descriptor (stdin)
    Outfile dd 1    ; Output file descriptor (stdout)

section .text
    global main
    extern printf
    extern encode

main:
    push ebp
    mov ebp, esp
    mov esi, [ebp+8]   ; Get first argument argc
    cmp esi, 1          ; if argc == 1
    je encode
    mov edi, [ebp+12]  ; Get third argument argv
    add edi, 4          ; advance edi to &argv[1]
    dec esi             ; decrement arg counter

parse_arguments:
    pushad
    push dword [edi]   ; push argv[i] (i=0 first)
    push dword out_fmt
    call printf         ; printf(out_fmt, argv[i])
    add esp, 8          ; "remove" printf arguments
    popad

    mov eax, [edi]      ; Load current argument
    cmp byte [eax], '-' ; Check if starts with '-'
    jne next_argument   ; If not, go to the next argument

    mov al, byte [eax + 1]  ; check the second character
    cmp al, 'i'             ; If equal to 'i'
    je open_input           ; If yes, open input file
    cmp al, 'o'             ; If equal to 'o'
    je open_output          ; If yes, open output file

next_argument:
    add edi, 4          ; advance edi to &argv[i+1]
    dec esi             ; decrement arg counter
    jnz parse_arguments            ; loop if not yet zero

    ; Call encode function after printing all argv[i]
    call encode

    mov esp, ebp
    pop ebp
    ret

open_input:
    add eax, 2                    ; Move to filename (skip "-i")
    mov ebx, eax
    mov eax, 5
    mov ecx, 0
    int 0x80

    cmp eax, 0                   ; Check if open failed
    je input_file_error           ; Jump to error handling if open failed
    mov dword [Infile], eax       ; Save input file descriptor

    jmp next_argument             ; Jump to the next argument


open_output:
    add eax, 2                    ; Move to filename
    mov eax, 5
    mov ebx, edi
    mov ecx, 1101o
    int 0x80

    cmp eax, -1                   ; Check if open failed
    je output_file_error           ; Jump to error handling if open failed
    mov dword [Outfile], eax       ; Save output file descriptor

    jmp next_argument             ; Jump to the next argument

encode:
    ; Read character from stdin
read_char:
    mov eax, 3  ; Read syscall number
    mov ebx, dword [Infile] ; Input file descriptor
    mov esi, esp ; Buffer to store character
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

input_file_error:
    ; Print error message for input file
    push in_err_msg
    call printf
    add esp, 4
    jmp next_argument                    ; Jump to encode after error handling

output_file_error:
    ; Print error message for output file
    push out_err_msg
    call printf
    add esp, 4
    jmp next_argument                   ; Jump to encode after error handling

section .note.GNU-stack
    align 4
    dd 0x474e552d
    dd 0x1002
    dd 0x1