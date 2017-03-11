[GLOBAL write]
write:
	mov eax, 1
	int 64
	ret

[GLOBAL fork]
fork:
	mov eax, 2
	int 64
	ret
