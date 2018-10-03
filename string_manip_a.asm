.CODE

; void string_store(const size_t size, volatile char* data);
string_store PROC
; rcx = size
; rdx = data
mov rax, 00101010101010101h ; 0x01 in each byte
mov r8, rdi					; save off rdi as we are about to clobber it and it's non-volatile (callee-saved)
mov rdi, rdx				; rdi = data
rep stosq					; store rax to rcx qwords, starting at rdi ( i.e. memset(data, 0x01, 8*size) )
mov rdi, r8					; restore rdi
ret
string_store ENDP

END