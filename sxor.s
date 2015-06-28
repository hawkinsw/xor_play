; 
; This is a testing/prototype file for how to
; build canaries that are self-decoding.
; 
; The actual utility of this has been implemented
; as a transformation called fix_canaries in
; security_transforms
;
bits 64
section .data
; First four bytes are the key.
; Second four bytes are the randomness.
fs0x28: db 0x00, 0x34, 0x56, 0x78, 0xFF, 0xFF, 0xFF, 0xFF

; The imaginary text segment.
low: db 0x0
texts: db 0xbb,0xbb,0xbb,0xbb,0xaa,0xaa,0xaa,0xaa
;0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
high: db 0x0

section .text
global _start
_start:
setup:
	nop
	mov r14, low
	mov r15, high

breakpoint:
canary_push:
	; We think that it's faster to always
	; clamp down the pointer.
	mov rax, 0xFFFFFFFF0000001F ; the bottom of this is m_text_size
	and rax, [fs0x28]
	add rax, texts; again, m_text_base
	mov [fs0x28], rax ; it is unfortunate that we still need to do this
	mov rcx, rax
	mov rax, [eax]
	shr rax, 32
	shl rax, 32
	xor rax, [fs0x28]
	rol rax, 32       ; In transform, we will probably stop here.
	push rax          ; The push will be done by the code 
	                  ; that already exists.

inbetween:
	mov rax, [fs0x28]
	push rax
	pop rax

	; kill the registers
	mov rax, 0xdead
	mov rbx, 0xdead

	; twiddle the pointer;
	; aka, simulate the changing of the canary
	mov [fs0x28], dword 0x0

	; mess up the canary.
	pop rcx
	;rol rcx, 32
	;shr rcx, 18
	;shl rcx, 18
	;rol rcx, 32
	push rcx
	mov rcx, 0

canary_pop:
	mov rax, [rsp]
	rol rax, 32
	cmp eax, high ; This is goign to check to see if the
	jg fail       ; pointer value was tampered.
	cmp eax, low  ; 
	jl fail       ; 
	mov rax, [eax]
	xor rax, [fs0x28]
	shr rax, 32
	xor rax, [rsp]
	cmp eax, 0x0 ; In transform we will probably stop here.
	jne fail     ; The jump will be done by the code 
	             ; that already exists.
pass:
	mov rbx, 0
	jmp end
fail:
	mov rbx, 1
end:
	mov eax, 1
	int 0x80
