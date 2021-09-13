section .data
    msg: db "Hello, Infected File",10 ,0	; format string

section .text
global _start
global system_call
global code_start
global code_end
global infection
global infector
extern main
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
    push    ebp             ; Save caller state (saves the beginnning of the scope)
    mov     ebp, esp        ; saves the current position of the pointer to the stack in ebp
    sub     esp, 4          ; Leave space for local var on stack ("goes back" 4 steps in the stack)
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






code_start:
	
infection:
    push ebp                ; saves the original value of ebp
    mov ebp, esp            ; move ebp to piont to the current position in the stack
    pushad                  ; backup registers value

    mov eax,4              ; sys_write opcode is 4
    mov ebx,1              ; we want to write to stdout=1
    mov ecx, msg     ; ecx gets pointer to the beginning of the printed string
    mov edx, 20            ; edx gets the length of our msg
    int 0x80                ; call the kernel (to activate the func)
    popad                   ; restor original value of the registers
    mov esp, ebp            ; restor original value of esp in order to free the stack
    pop ebp                 ; restor original value of ebp
    ret                     ; returns from the function
code_end:

infector:
    push ebp                ; saves the original value of ebp
    mov ebp, esp            ; move ebp to piont to the current position in the stack
    pushad                  ; backup registers value
    
    ; opens file
    mov eax, 5              ; op code of open sys call is 5
    mov ebx, [ebp+8]        ; saves the 1st argumrnt in ebp
    mov ecx, 0x401          ; O_APPAND + O_WRITE (O_APPAND adds the new text to the end of the file)
    mov edx, 0777           ; set the new file permissions
    int 0x80                ; call the kernel (to activate the func)
    
    ; write to file
    mov ebx, eax
    mov eax, 4              ; sys_write opcode is 4
    mov ecx, code_start     ; saves a pointer to the first byte to write in ecx
    mov edx, code_end-code_start               ; saves the number of bytes to write in edx
    int 0x80                ; call the kernel (to activate the func)

    
    ; closes the file
    mov eax, 6              ; 6 for close (ebx already contains the file descriptor)
    mov ebx, esi
    int 0x80                ; call the kernel (to activate the func)
    popad                   ; restor original value of the registers
    mov esp, ebp            ; restor original value of esp in order to free the stack
    pop ebp                 ; restor original value of ebp
    ret                     ; returns from the function
