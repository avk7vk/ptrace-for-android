#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/reg.h> /*ORIG_RAX, RAX*/

/* Determine if 64/32 bit */
#ifdef ORIG_EAX
	const int const_orig_eax = ORIG_EAX;
	const int const_eax = EAX;
#else
	 const int const_orig_eax = ORIG_RAX;
	 const int const_eax = RAX;
#endif

int wait_for_syscall(pid_t child);
int main()
{
	pid_t childPid;
	long orig_eax, eax;
	int status;

	childPid = fork();
	if(childPid >= 0) {

		if(childPid == 0) { /*In Child Process */ 
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);
			execl("/bin/ls","ls", NULL);
		}
		else { /*In Parent Processs */
			printf("\n========Hi I am in Parent Process=======\n");
			printf("Child PID = %d\n", childPid);

			waitpid(childPid, &status, 0);
			ptrace(PTRACE_SETOPTIONS, childPid, 0, 
				PTRACE_O_TRACESYSGOOD);
			
			while(1){
				if(wait_for_syscall(childPid) != 0) break;
				orig_eax = ptrace(PTRACE_PEEKUSER, childPid, 
						sizeof(long) * const_orig_eax, NULL);
				printf("System Call Number is %ld ", orig_eax);
				
				if(wait_for_syscall(childPid) != 0 ) break;
				eax = ptrace(PTRACE_PEEKUSER, childPid, 
					sizeof(long)*const_eax, NULL);
				printf("Return Value = %ld\n",eax);
			}
		}
	}
	else {
		printf("Creation of Child Process failed\n");
		printf("Error Code = %d\n", childPid);
	}

	return 0;
}

int wait_for_syscall(pid_t child) {
	int status;
	while(1) {	
		ptrace(PTRACE_SYSCALL, child, NULL, NULL);	
		waitpid(child, &status, 0);
		if (WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80)) {
			return 0;
		}
		if (WIFEXITED(status)) {
			printf("Child process exited Normally\n");
			return 1;
		}
	}
}