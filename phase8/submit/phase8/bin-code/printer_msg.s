.text
.global _start

_start:
	mov $msg, %eax
	sub $_start, %eax	
	add $0x40000000, %eax
	push %eax
	int $105

	pop %eax
	int $115




.data 
msg: .ascii "Hello, World!... ... ...\n\r"
