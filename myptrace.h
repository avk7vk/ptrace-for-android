#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/reg.h> /*ORIG_RAX, RAX*/
#include <string.h>
#include <stdlib.h>
/* Determine if 64/32 bit */
#ifdef ORIG_EAX
	const int const_orig_eax = ORIG_EAX;
	const int const_eax = EAX;
	const int const_ebx = EBX;
	const int const_ecx = ECX;
	const int const_edx = EDX;
#else
	const int const_orig_eax = ORIG_RAX;
	const int const_eax = RAX;
	const int const_ebx = RBX;
	const int const_ecx = RCX;
	const int const_edx = RDX;
#endif

#define MAX_STRING_LEN 50
const int long_size = sizeof(long);
int wait_for_syscall(pid_t child);
int interpret_syscall(int, pid_t, char *, unsigned int);
void interpret_write(const int, pid_t, char *,unsigned int);
void interpret_read(const int, pid_t, char *,unsigned int);
void interpret_open(const int, pid_t, char *,unsigned int);
int get_string_data_unbounded(pid_t, long, char *, int);
void get_string_data(pid_t, long, char *,int);

 int interpret_syscall(int syscall_no, pid_t child, char *buf,
 unsigned int len)
 {
 	switch(syscall_no) {
 		case SYS_write:
 			interpret_write(syscall_no, child, buf, len);
 			break; 
 		case SYS_read:
 			interpret_read(syscall_no, child, buf, len);
 			break;
 		case SYS_open:
 			interpret_open(syscall_no, child, buf, len);
 			break; 
 		default :
 			snprintf(buf ,len ,"sys call - %d",syscall_no);
 	}

 	return 0;
 } 

void interpret_write(const int syscall_no, pid_t child, char *buf,
	unsigned int len) 
{
	long ebx, ecx, edx;
	ebx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_ebx
		,0);
	ecx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_ecx
		,0);
	edx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_edx
		,0);
	snprintf(buf ,len ,"write( %u, %u, %d)",(unsigned int) ebx,
		(unsigned int) ecx, (int) edx);	
}

void interpret_read(const int syscall_no, pid_t child, char *buf,
	unsigned int len) 
{
	long ebx, ecx, edx;
	ebx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_ebx
		,0);
	ecx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_ecx
		,0);
	edx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_edx
		,0);
	snprintf(buf ,len ,"read( %u, %u, %d)",(unsigned int) ebx,
		(unsigned int) ecx, (int) edx);	
}

void interpret_open(const int syscall_no, pid_t child, char *buf,
	unsigned int len) 
{
	long ebx, ecx, edx;
	int err;
	char *fname = (char *) malloc(sizeof(char)*(MAX_STRING_LEN+1));
	ebx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_ebx
		,0);
	ecx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_ecx
		,0);
	edx = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * const_edx
		,0);

	err = get_string_data_unbounded(child, ebx, fname,
		 MAX_STRING_LEN);
	if (err) 
		fname[0] = '\0';
	snprintf(buf ,len ,"open( %s, %d, %d)",fname,
		(int) ecx, (int) edx);
	/*CLEANUP*/
	if(fname) free(fname);	
}

void get_string_data(pid_t child, long addr, char *str, int len)
{
	int i, j;
	char *laddr;
	union u {
		long val;
		char chars[long_size];
	}data;

	i = 0;
	j = len / long_size;
	laddr = str;
	while(i < j) {
		data.val = ptrace(PTRACE_PEEKDATA, child, 
			(i * long_size) + addr, NULL);
		memcpy(laddr, data.chars, long_size);
		laddr += long_size;
		i++;
	}
	j = len % long_size;
	if (j != 0) {
		data.val = ptrace(PTRACE_PEEKDATA, child, 
			(i * long_size) + addr, NULL);
		memcpy(laddr, data.chars, long_size);
		laddr += j;
	}
	laddr[len] = '\0';

}
int get_string_data_unbounded(pid_t child, long addr, char *str, 
	int len)
{
	int i, j, k;
	char *laddr;
	union u {
		long val;
		char chars[long_size];
	}data;

	i = 0;
	j = len / long_size;
	laddr = str;
	while(i < j) {
		data.val = ptrace(PTRACE_PEEKDATA, child, 
			(i * long_size) + addr, NULL);
		for (k = 0; k < long_size; k++) {
			//printf("%c", data.chars[k]); //For Debug
			if (data.chars[k] == '\0') {
				memcpy(laddr, data.chars, k+1);
				return 0;		
			}
		}
		memcpy(laddr, data.chars, long_size);
		laddr += long_size;
		i++;
	}
	j = len % long_size;
	if (j != 0) {
		data.val = ptrace(PTRACE_PEEKDATA, child, 
			(i * long_size) + addr, NULL);
		for (k = 0; k < j; k++) {
			if (data.chars[k] == '\0') {
				memcpy(laddr, data.chars, k+1);
				return 0;		
			}
		}
		memcpy(laddr, data.chars, long_size);
		laddr += j;
	}
	return -1; // No NULL char was found

}