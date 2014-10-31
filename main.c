#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h> /*errno*/
#include <sys/reg.h>

#define ORIG_EAX 11 /*Include a Header Instead*/
int main()
{
	pid_t childPid;
	int status;
	long orig_eax;

	childPid = fork();
	if(childPid >= 0) {
		if(childPid == 0) { // In Child Process
			printf("========Hi I am in Child Process=======\n");
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);
			execl("/bin/ls","ls", NULL);

		}
		else { // In Parent Process
			printf("\n========Hi I am in Parent Process=======\n");
			printf("CHild PID = %d\n", childPid);
			while(1){
				
				wait(&status);//Blocks till child terminates
				
				if(WIFEXITED(status)) {
					printf("Child Process Exited Normally\n");
					break;
				}
				else {
					printf("Child Process was stopped!\n");
				}

				//printf("Parent: Ahh! my wait is over!\n");
				orig_eax = ptrace(PTRACE_PEEKUSER, childPid, 
					4 * ORIG_EAX, NULL);
				printf("System Call Number is %ld\n", orig_eax);
				printf("Error is %d\n", errno);
				ptrace(PTRACE_SYSCALL, childPid, NULL, NULL);
			}
		}
	}
	else {
		printf("Creation of Child Process failed\n");
		printf("Error Code = %d\n", childPid);
	}

	return 0;
}