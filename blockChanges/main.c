#include "myptrace.h"

FILE* fd = NULL;

int main(int argc, char* argv[])
{
	pid_t childPid;
	long orig_eax, eax;
	int status;
	char *tmp = NULL;
	char **cmd_path;
	char *cmd_name;
	int ret = 0, trace_flag = 0;
	int block = 0;
	char *filename = NULL;

	if(argc < 2) {
		cmd_name = "ls";
		cmd_path = (char *[]){"ls", NULL};
	}
	else if (!strcmp(argv[1], "run_app") && argc == 4){
		char arg_buf[100];
		int i;
		snprintf(arg_buf, 100,"%s/.%s", argv[2], argv[3]); 
		cmd_name = "am";
		cmd_path = (char *[]){"am", "start", "-n", 
			arg_buf, NULL};
		printf("Run App Arguments : \n");
		i = 0;
		while (cmd_path[i] != NULL){
			printf("%s", cmd_path[i++]);
		}
	}
	else if (!strcmp(argv[1], "-p") && (argc == 3||argc == 5)) {
		trace_flag = 1;
		if (argc == 5 && !strcmp(argv[3], "-i")) {
			block = 1;
			filename = argv[4];
		}
		
	}
	else {	
		cmd_name = argv[1];
		cmd_path = &argv[1] ;
	}
	if (trace_flag == 1) {
		childPid = (pid_t)strtol(argv[2], NULL, 10);
		ptrace(PTRACE_ATTACH, childPid, NULL, NULL);	
		if (errno < 0) 
			printf("ATTACH Error is %s\n",strerror(errno));
		else printf("ATTACH Success\n");
	}
	else {
		childPid = fork();
	}
	if(childPid >= 0) {

		if(childPid == 0 && trace_flag != 1) { /*In Child Process */ 
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
			
			if(open_file("testing.txt")) {
				ret = errno;
				
				goto err_exit;
			}
			
			waitpid(childPid, &status, 0);
			
			//if (!test_ptrace_setoptions_for_all()) {
				ptrace(PTRACE_SETOPTIONS, childPid, 0, 
					PTRACE_O_TRACESYSGOOD);
			//	printf("PTRACE_SETOPTIONS Success\n");
			//}

						
			while(1){
				int reg_err;
				long reg_array[MAX_SYS_REG_ENTRIES];
				const unsigned int len = MAX_OUT_STRING_LEN;; 
				tmp = (char *) calloc(len, sizeof(char));
				
				if(wait_for_syscall(childPid) != 0) break;
				// sys call entry

				reg_err = get_regs(childPid, reg_array);
				orig_eax = reg_array[0];
				
				if(orig_eax >= 0) {
					if (block == 1) {
						if (check_blocklist(filename, orig_eax) == 1) {
							kill(childPid,SIGKILL);
							goto err_exit;
						}
					}
					print_syscall(childPid, reg_array, tmp, len);
					printf("%s", tmp);
					
					if (write_file(tmp, strlen(tmp)) < 0) {
						ret = errno;
						goto err_exit;
					}
				}
				else {
					printf("Sys Call Error : %ld", (long)reg_array[0]);
					printf("No : %d\n",errno);
				}
				if(wait_for_syscall(childPid) != 0 ) break;
				// sys call exit
				reg_err = get_regs(childPid, reg_array);
				eax = (long)reg_array[7];
				printf("%ld\n", eax);
				snprintf(tmp, len,"%ld\n", eax);
				
				if (write_file(tmp, strlen(tmp)) < 0) {
					ret = errno;
					goto err_exit;
				}
				
			}
			if (trace_flag == 1)
				ptrace(PTRACE_DETACH, childPid, NULL, NULL);
		}
	}
	else {
		printf("Creation of Child Process failed\n");
		printf("Error Code = %d\n", childPid);
	}

	err_exit:
	if(tmp) free(tmp);
	if(fd) fclose(fd);
	return ret;
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
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
			printf("Child process was KILL\n");
			return 1;
		}
	}
}

int open_file(char *fname) {
	fd = fopen(fname, "w");
	if(!fd) {
		printf("Error: %s", strerror(errno));
		return -1;
	}
	return 0;
}

int write_file(char *buf, int len) 
{
	int tmp = 0, wb = 0;
	tmp = fwrite(buf,char_size, len, fd);
	wb = tmp/char_size;
	if ( wb != len || tmp == EOF) {
		printf("Error: %s", strerror(errno));
		return -1;
	}
	fflush(fd);
	return tmp/char_size;
}
