#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__x86_64__) || defined(_x86__)
	#include <sys/reg.h>
	#include <sys/user.h>
	#if defined(_x86__)
		#include "syscallents_x86.h"
	#else
		#include "syscallents_x86_64.h"
	#endif
#else
	#include <sys/user.h>
	#include <asm/ptrace.h>
	#include "syscallents_arm.h"
#endif

#define char_size sizeof(char)
#define MAX_REG_ENTRIES 30
#define MAX_SYS_REG_ENTRIES 8
#define DBG printf("In %s at %d", __FUNCTION__, __LINE__)
const int long_size = sizeof(long);
int wait_for_syscall(pid_t child);

int interpret_syscall(long *, pid_t, char *, unsigned int);
void interpret_write(long *, pid_t, char *,unsigned int);
void interpret_read(long *, pid_t, char *,unsigned int);
void interpret_open(long *, pid_t, char *,unsigned int);
int get_string_data_unbounded(pid_t, long, char *, int);
void get_string_data(pid_t, long, char *,int);
int get_regs(pid_t, long *);
int write_file(char *buf, int len) ;
int open_file(char *fname);
void print_syscall(pid_t , long* , char *, int );
void print_syscall_args(pid_t , long* , char *, int );
const char *syscall_name(int scn);
char *read_string(pid_t child, unsigned long addr);

#include "defs.h"
#include "syscall_interpret.h"

/* Interpret Syscall 
 * reg_array - contains all the regs needed to interpret syscall
 * child - tracee's PID
 * buf - Buffer
 * len - Length of the buffer
 * 
 * This func is responsible for interpreting syscalls . It checks 
 * syscall no and sends to appropriate handler
 */
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

/* Get_regs 
 * child - tracee's PID
 * buf - Buffer
 * 
 * This func is responsible for collecting all the regs used for 
 * syscalls. It creates a reg_array of size MAX_SYS_REG_ENTRIES
 * .The order of regs is orig_eax (syscall no), ebx, ecx, edx,
 * esi, ebp, eax (return Value) 
 */
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
		buf[1] = (long)my_regs.rdi;
		buf[2] = (long)my_regs.rsi;
		buf[3] = (long)my_regs.rdx;
		buf[4] = (long)my_regs.rcx;
		buf[5] = (long)my_regs.r8;
		buf[6] = (long)my_regs.r9;
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
void print_syscall(pid_t child, long* sys_regs, char *buf, 
	int slen) {
    int num;
    char *out_str = buf;
    num = sys_regs[0];

    snprintf(&out_str[strlen(out_str)], 
    	(slen-strlen(out_str)), "%s(", syscall_name(num));
    print_syscall_args(child, sys_regs, &out_str[strlen(out_str)], 
    	(slen-strlen(out_str)));
    snprintf(&out_str[strlen(out_str)], 
    	(slen-strlen(out_str)), ") = ");

}

void print_syscall_args(pid_t child, long* sys_regs, char * buf,int slen) {
    struct syscall_entry *ent = NULL;
    int nargs = SYSCALL_MAXARGS;
    int i;
    int num = sys_regs[0];

    if (num <= MAX_SYSCALL_NUM && syscalls[num].name) {
        ent = &syscalls[num];
        nargs = ent->nargs;
    }
    for (i = 0; i < nargs; i++) {
        long arg = sys_regs[i+1];
        int type = ent ? ent->args[i] : ARG_PTR;
        switch (type) {
        case ARG_INT:
            snprintf(&buf[strlen(buf)], slen -strlen(buf), 
            	"%ld", arg);
            break;
        case ARG_STR:
    		{
    			char *strval = read_string(child, arg);
            	snprintf(&buf[strlen(buf)], slen -strlen(buf), 
            		"\"%s\"", strval);
            	free(strval);
            }
            break;
         case ARG_SOCK:
         	char *addr = handle_sockaddr(child, arg);
         	snprintf(&buf[strlen(buf)], slen -strlen(buf), 
            		"\"%s\"", addr);
        default:
            snprintf(&buf[strlen(buf)], slen -strlen(buf), 
            	 "0x%lx", arg);
            break;
        }
        if (i != nargs - 1)
            snprintf(&buf[strlen(buf)], slen -strlen(buf),  ", ");
    }
}
const char *syscall_name(int scn) {
    struct syscall_entry *ent;
    static char buf[128];
    if (scn <= MAX_SYSCALL_NUM) {
        ent = &syscalls[scn];
        if (ent->name)
            return ent->name;
    }
    snprintf(buf, sizeof buf, "sys_%d", scn);
    return buf;
}
int check_blocklist(char* filename, unsigned long eax) {
	char *line = NULL;
	size_t len = 0;
	int ret = 0;
	ssize_t read;
	FILE *fp = NULL;
	if(!filename)
		goto out;
	fp = fopen(filename, "r");
	if (!fp)
		goto out;
	while ((read = getline(&line, &len, fp)) != -1) {
		line[(int)strlen(line)-1] = '\0';
		if (!strcmp(line, syscall_name(eax))) {
			printf("System call %s matched with entry in "
				" in block list\n", line);
			ret = 1;
		}
		//printf("%s %s\n",line, syscall_name(eax));
		if(line) {
			free(line);
			line = NULL;
		}
		if(ret == 1)
			break;
	}
	out:
	if (fp)
		fclose(fp);
	return ret;
}
char* handle_sockaddr(pid_t child, long arg) {
	const int sockaddr_sz = sizeof(struct sockaddr);
	struct sockaddr sk;
	struct sockaddr_in si;
	long tmp;
	int read = 0; 
	int itrs = sockaddr_sz / long_size;

	for (i = 0; i < itrs; i++) {
		tmp = ptrace(PTRACE_PEEKDATA, child, addr + read);
		memcpy(&sk + read, &tmp, sizeof tmp);
		read += sizeof tmp;
	}
	itrs = sockaddr_sz % long_size;
	if (itrs != 0) {
		tmp = ptrace(PTRACE_PEEKDATA, child, addr + read);
		memcpy(&sk + read, &tmp, itrs);
		read += itrs;
	}
	si = (struct sockaddr)sk;
	return inet_ntoa(si.sin_addr);
}
char *read_string(pid_t child, unsigned long addr) {
    
    int allocated = MAX_STRING_LEN, diff;
    char *val = calloc(allocated+1, sizeof(char));
    int read = 0;
    unsigned long tmp;
    while (1) {
        
        tmp = ptrace(PTRACE_PEEKDATA, child, addr + read);
        if(errno != 0) {
            snprintf(val, allocated, "%s",strerror(errno));
            break;
        }
        if (read + sizeof tmp > allocated) {
            diff = (allocated - read -1);
        	memcpy(val + read, &tmp, diff);
        	val[allocated] = '\0';
        	break;
        }
        memcpy(val + read, &tmp, sizeof tmp);
        if (memchr(&tmp, 0, sizeof tmp) != NULL)
            break;
        read += sizeof tmp;
    }
    return val;
}
/* Get_String_Data 
 * child - tracee's PID
 * addr - Memory Address where string resides
 * str - buffer to write the data
 * len - string length
 * 
 * This func is responsible for fetching the string data from 
 * a given memory location pointed by "addr. Use this func when 
 * when length of the memory is known. 
 */
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

/* Get_String_Data 
 * child - tracee's PID
 * addr - Memory Address where string resides
 * str - buffer to write the data
 * len - Max len to check read for the string data
 * 
 * This func is responsible for fetching the string data from 
 * a given memory location pointed by "addr. The length of the 
 * string data is not known in advance, so this function reads 
 * data till a MAX_LIMIT value and until it finds a String 
 * terminating character. 
 */
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
	laddr[len] = '\0';
	return 0; // No NULL char was found

}

/* Test Ptrace Set Options
 * 
 * This func checks if the option PTRACE_O_TRACESYSGOOD
 * works for this architecture or not.
 */
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