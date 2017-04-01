#ifndef __SYSCALL_H__
#define __SYSCALL_H__

void syscall_handler(registers_t *r);
int argint(int n, int *p);
int argstr(int n, char **p);

#endif /* __SYSCALL_H__ */
