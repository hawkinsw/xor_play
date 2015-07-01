; 
; This is a testing/prototype file for how to
; build canaries that are linked to one another.
;
; Each canary contains the distance to the previous
; canary and the fs:0x28 contains a pointer to the
; absolute address of the topmost canary.
; 
; The actual utility of this has been implemented
; as a transformation called fix_canaries in
; security_transforms
;
bits 64
section .data
fs0x28: db 0xff,0xff,0xab,0xcd,0xab,0xcd,0xef,0xab

section .text
global _start
_start:
setup:
canary_push:
	mov rax, [fs0x28]
	sub ax, sp
	; mov %rax, 0x8(%rsp)
	push rax
	mov rax, 0xffffffffffff0000
	and rax, [fs0x28]
	add ax, sp
	mov [fs0x28], sp 

screw_up:
	; mess up the canary.
	pop rcx
	;rol rcx, 32
	shr rcx, 18
	shl rcx, 18
	;rol rcx, 32
	push rcx
	mov rcx, 0

canary_pop:
	mov rax, [rsp]
	;add ax, sp
	add ax, [fs0x28]
	add ax, 0x8
	mov [fs0x28], ax
	xor rax, [fs0x28]
;breakpoint:
	jz pass
fail:
	mov rbx, 1
	jmp end
pass:
	mov rbx, 0
end:
	mov eax, 1
	int 0x80
