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

[GLOBAL execve]
execve:
	mov eax, 2
	int 64
	ret

[GLOBAL read]
read:
	mov eax, 3
	int 64
	ret

[GLOBAL exit]
exit:
	mov eax, 4
	int 64
	ret

[GLOBAL waitpid]
waitpid:
	mov eax, 5
	int 64
	ret
