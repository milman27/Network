.intel_syntax noprefix
.global _start
.section .text
_start:
	mov rbp, rsp
	sub rsp, 0xFFb0
socket:
	mov rdi, 2
	mov rsi, 1
	mov rdx, 0
	mov rax, 41
	syscall
bind:
	mov rdi, rax
	lea rsi, [rip+sockaddr]
	mov rdx, 0x10
	mov rax, 0x31
	syscall
listen:
	mov rsi, 0
	mov rax, 0x32
	syscall
 	mov [rbp-0x10], rdi
accept:
	mov rdi, [rbp-0x10]
	mov rsi, 0
	mov rdx, 0
	mov rax, 0x2B
	syscall
	mov rdi, rax
	mov rax, 0x39
	syscall
	cmp rax,0
	je close
	mov rax, 3
	syscall 
	jmp accept	
close:
	mov r10, rdi
	mov rdi, [rbp-0x10]
	mov rax, 3
	syscall
read:
	mov rdi, r10
	lea rsi, [rsp]
	mov rdx, 0xFF00 
	mov rax, 0
	syscall
	mov [rbp-0x50], r10
	mov [rbp-0x20], rax
getorpost:
	cmp BYTE PTR [rsp], 'G'
	je get
	mov QWORD PTR [rbp-0x30], 1
	jmp formatpath
get:
	mov  QWORD PTR [rbp-0x30], 0
formatpath:
	mov r10, [rbp-0x30]
	add r10, 3
	lea r10, [rsp+r10]
loop1:
	add r10, 1
	cmp BYTE PTR [r10], 0x20
	jne loop1
	mov BYTE PTR [r10], 0	
	mov r10, rdi
	cmp QWORD PTR [rbp-0x30], 1
	je POST
GET:
open:
	mov rdx, 0
	mov rsi, 0
	lea rdi, [rsp+4]
	mov rax, 2
	syscall
readfile:
	mov rdi, rax
	lea rsi, [rsp]
	mov rdx, 0xFF00
	mov rax, 0
	syscall
closefile:
	mov r8, rax
	mov rax, 3
	syscall
	
	mov rdi, r10
writehttp:
	lea rsi, [rip+httpok]
	mov rdx, 0x13
	mov rax, 1
	syscall
writefile:
	lea rsi, [rsp]
	lea rdx, [r8]
	mov rax, 1
	syscall
	jmp _exit
POST:
openwrite:
	mov rdx, 0777
	mov rsi, 65
	lea rdi, [rsp+5]
	mov rax, 2
	syscall
	xor r10, r10
	xor r11, r11
filesize:
	add r10, 1
	cmp BYTE PTR[rsp+r10], '\r'
	jne filesize
	cmp BYTE PTR[rsp+r10+1], '\n'
	jne filesize
	cmp BYTE PTR[rsp+r10+2], '\r'
	jne filesize
	cmp BYTE PTR[rsp+r10+3], '\n'
	jne filesize
	add r10, 4
	mov [rbp-0x40], r10 
writefile2:
	mov rdx, [rbp-0x20]
	sub rdx, r10
	lea rsi, [rsp+r10]
	mov rdi, rax
	mov rax, 1
	syscall
closefile2:
	mov rax, 3
	syscall

writehttp2:
	mov rdi, [rbp-0x50]
	lea rsi, [rip+httpok]
	mov rdx, 0x13
	mov rax, 1
	syscall

_exit:
	mov rdi, 0
	mov rax, 60
	syscall

.section .data
sockaddr:
	.2byte 2
	.2byte 0x5000
	.4byte 0
	.8byte 0
httpok:
	.ascii "HTTP/1.0 200 OK\r\n\r\n"
	.set http_size, .-httpok
