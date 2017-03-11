[GLOBAL write]
write:
	mov eax, 0
	int 64
	ret

[GLOBAL fork]
fork:
	mov eax, 1
	int 64
	ret

[GLOBAL exec]
exec:
	mov eax, 2
	int 64
	ret
