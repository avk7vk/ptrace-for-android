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
#if defined(__x86_64__)
	#include <sys/user.h>
#else
	#include <sys/user.h>
	#include <asm/ptrace.h>
#endif
#include "defs.h"


/* Determine if 64/32 bit */
#ifdef ORIG_EAX
	//printf("****32 Bit System\n");
	const int const_orig_eax = ORIG_EAX;
	const int const_eax = EAX;
	const int const_ebx = EBX;
	const int const_ecx = ECX;
	const int const_edx = EDX;
#else
	//printf("****64 Bit System\n");
	const int const_orig_eax = ORIG_RAX;
	const int const_eax = RAX;
	const int const_ebx = RBX;
	const int const_ecx = RCX;
	const int const_edx = RDX;
#endif

#define MAX_STRING_LEN 50
#define MAX_REG_ENTRIES 30
#define MAX_SYS_REG_ENTRIES 8

const int long_size = sizeof(long);
int wait_for_syscall(pid_t child);

int interpret_syscall(long *, pid_t, char *, unsigned int);
void interpret_write(long *, pid_t, char *,unsigned int);
void interpret_read(long *, pid_t, char *,unsigned int);
void interpret_open(long *, pid_t, char *,unsigned int);
int get_string_data_unbounded(pid_t, long, char *, int);
void get_string_data(pid_t, long, char *,int);
int get_regs(pid_t, long *);
int get_regs_size();

int interpret_syscall(long *reg_array, pid_t child, 
	char *buf, unsigned int len)
 {
 	int syscall_no = reg_array[0];
 	switch(syscall_no) {
 		case SYS_write:
 			interpret_write(reg_array, child, buf, len);
 			break; 
 		case SYS_read:
 			interpret_read(reg_array, child, buf, len);
 			break;
 		case SYS_open:
 			interpret_open(reg_array, child, buf, len);
 			break; 
 		default :
 			snprintf(buf ,len ,"sys call - %d",syscall_no);
 	}

 	return 0;
 } 

void interpret_write(long *reg_array, pid_t child, char *buf,
	unsigned int len) 
{
	long ebx, ecx, edx;
	ebx = reg_array[1];
	ecx = reg_array[2];
	edx = reg_array[3];
	snprintf(buf ,len ,"write( %u, %u, %d)",(unsigned int) ebx,
		(unsigned int) ecx, (int) edx);	
}

void interpret_read(long *reg_array, pid_t child, char *buf,
	unsigned int len) 
{
	long ebx, ecx, edx;
	ebx = reg_array[1];
	ecx = reg_array[2];
	edx = reg_array[3];
	snprintf(buf ,len ,"read( %u, %u, %d)",(unsigned int) ebx,
		(unsigned int) ecx, (int) edx);	
}

void interpret_open(long *reg_array, pid_t child, char *buf,
	unsigned int len) 
{
	long ebx, ecx, edx;
	int err;
	char *fname = (char *) malloc(sizeof(char)*(MAX_STRING_LEN+1));
	ebx = reg_array[1];
	ecx = reg_array[2];
	edx = reg_array[3];
	err = get_string_data_unbounded(child, ebx, fname,
		 MAX_STRING_LEN);
	if (err) 
		fname[0] = '\0';
	snprintf(buf ,len ,"open( %s, %d, %d)",fname,
		(int) ecx, (int) edx);
	
	/*CLEANUP*/
	if(fname) free(fname);	
}
int get_regs_size() 
{
	#if defined(__i386__) || defined(__x86_64__)
		return sizeof(struct user_regs_struct);		
	#elif defined(__arm__)
		return sizeof(struct arm_pt_regs);
	#else
		return MAX_REG_ENTRIES;
	#endif
}
int get_regs(pid_t child, long *buf) {
	#if defined(__i386__) || defined(__x86_64__)
		struct user_regs_struct i386_regs;
		#define my_regs i386_regs
	#else
		struct arm_pt_regs arm_regs;
		#define my_regs arm_regs
	#endif
	int err;

	err = ptrace(PTRACE_GETREGS, child, 0, &my_regs);
	if (err < 0)
		return err;

	#if defined (__i386__)
		buf[0] = my_regs.orig_eax;
		buf[1] = my_regs.ebx;
		buf[2] = my_regs.ecx;
		buf[3] = my_regs.edx;
		buf[4] = my_regs.esi;
		buf[5] = my_regs.edi;
		buf[6] = my_regs.ebp;
		buf[7] = my_regs.eax;
	#elif defined(__x86_64__)
		buf[0] = (long)my_regs.orig_rax;
		buf[1] = (long)my_regs.rbx;
		buf[2] = (long)my_regs.rcx;
		buf[3] = (long)my_regs.rdx;
		buf[4] = (long)my_regs.rsi;
		buf[5] = (long)my_regs.rdi;
		buf[6] = (long)my_regs.rbp;
		buf[7] = (long)my_regs.rax;
	#else
		buf[0] = my_regs.ARM_r7;
		buf[1] = my_regs.ARM_r0;
		buf[2] = my_regs.ARM_r1;
		buf[3] = my_regs.ARM_r2;
		buf[4] = my_regs.ARM_r3;
		buf[5] = my_regs.ARM_r4;
		buf[6] = my_regs.ARM_r5;
		buf[7] = my_regs.ARM_r0;
	#endif

	return err;
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

static int test_ptrace_setoptions_for_all(void)
{
	const unsigned int test_options = PTRACE_O_TRACESYSGOOD |
					  PTRACE_O_TRACEEXEC;
	int pid;
	int it_worked = 0;

	pid = fork();
	if (pid < 0)
		printf("fork");

	if (pid == 0) {
		pid = getpid();
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0)
			/* Note: exits with exitcode 1 */
			printf("%s: PTRACE_TRACEME doesn't work",
					   __FUNCTION__);
		kill(pid, SIGSTOP);
		_exit(0); /* parent should see entry into this syscall */
	}

	while (1) {
		int status, tracee_pid;

		errno = 0;
		tracee_pid = wait(&status);
		if (tracee_pid <= 0) {
			if (errno == EINTR)
				continue;
			kill(pid, SIGKILL);
			printf("%s: unexpected wait result %d",
					   __FUNCTION__, tracee_pid);
		}
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0)
				break;
			printf("%s: unexpected exit status %u",
					  __FUNCTION__, WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			printf("%s: unexpected signal %u",
					  __FUNCTION__, WTERMSIG(status));
		}
		if (!WIFSTOPPED(status)) {
			kill(pid, SIGKILL);
			printf("%s: unexpected wait status %x",
					  __FUNCTION__, status);
		}
		if (WSTOPSIG(status) == SIGSTOP) {
			/*
			 * We don't check "options aren't accepted" error.
			 * If it happens, we'll never get (SIGTRAP | 0x80),
			 * and thus will decide to not use the option.
			 * IOW: the outcome of the test will be correct.
			 */
			if (ptrace(PTRACE_SETOPTIONS, pid, 0L, test_options) < 0
			    && errno != EINVAL && errno != EIO)
				printf("PTRACE_SETOPTIONS");
		}
		if (WSTOPSIG(status) == (SIGTRAP | 0x80)) {
			it_worked = 1;
		}
		if (ptrace(PTRACE_SYSCALL, pid, 0L, 0L) < 0) {
			kill(pid, SIGKILL);
			printf("PTRACE_SYSCALL doesn't work");
		}
	}

	if (it_worked) {
 //worked:
		return 0;
	}

	printf("Test for PTRACE_O_TRACESYSGOOD failed, "
		  "giving up using this feature.");
	return 1;
}