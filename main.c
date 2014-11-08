#include "myptrace.h"

int main(int argc, char* argv[])
{
	pid_t childPid;
	long orig_eax, eax;
	int status;
	char **cmd_path;
	char *cmd_name;

	if(argc < 2) {
		cmd_name = "ls";
		cmd_path = (char *[]){"ls", NULL};
	}
	else {
		cmd_name = argv[1];
		cmd_path = &argv[1] ;
	}

	childPid = fork();
	if(childPid >= 0) {

		if(childPid == 0) { /*In Child Process */ 
			int err;
			printf("I am in child\n");
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);

		#if defined(__arm__)
			err = execvp(cmd_name,cmd_path);
		#else
			err = execvp(cmd_name, cmd_path);
		#endif

			if (err == -1) printf("Error @%d is %s\n",__LINE__, 
				strerror(errno));
		}
		else { /*In Parent Processs */
			printf("\n========Hi I am in Parent Process=======\n");
			printf("Child PID = %d\n", childPid);

			waitpid(childPid, &status, 0);
			if (!test_ptrace_setoptions_for_all())
				ptrace(PTRACE_SETOPTIONS, childPid, 0, 
					PTRACE_O_TRACESYSGOOD);
			
			while(1){
				int reg_err;
				long reg_array[MAX_SYS_REG_ENTRIES];
				
				if(wait_for_syscall(childPid) != 0) break;
				
				reg_err = get_regs(childPid, reg_array);
				orig_eax = reg_array[0];
				
				if(orig_eax >= 0) {
					const unsigned int len = 100; 
					char tmp[len];
					interpret_syscall(reg_array, 
							childPid, tmp, len);
					printf("%s", tmp);
				}
				else {
					printf("Sys Call Error : %ld", (long)reg_array[0]);
					printf("No : %d\n",errno);
				}
				if(wait_for_syscall(childPid) != 0 ) break;
				
				reg_err = get_regs(childPid, reg_array);
				eax = (long)reg_array[7];
				printf(" - Return Value = %ld\n", eax);
			}
		}
	}
	else {
		printf("Creation of Child Process failed\n");
		printf("Error Code = %d\n", childPid);
	}
	return 0;
}
/* Wait For SysCall
 * child - PID of the Tracee
 *
 * This functions waits for syscall events in the Tracee's process.
 * The Tracee process stops for each entry and exit of a syscall. 
 * This is checked by the WIFSTOPPED() and WSTOPSIG checks if it
 * is SYSCALL SIGNAL. WIFEXITED() checks if the child process has 
 * exited normally through Syscall exit. It indicates the end of 
 * the current Tracing session for the child
 */
int wait_for_syscall(pid_t child) {
	int status;
	while(1) {	
		ptrace(PTRACE_SYSCALL, child, NULL, NULL);	
		waitpid(child, &status, 0);
		if (WIFSTOPPED(status) && (WSTOPSIG(status) ==(SIGTRAP | 0x80))) {
			return 0;
		}
		if (WIFEXITED(status)) {
			printf("Child process exited Normally\n");
			return 1;
		}
	}
}