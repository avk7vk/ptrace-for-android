/* Syscall_Interpret.h
 *
 * This Header file contains the functions to interpret arguments of
 * supported System calls.
 */
#define MAX_STRING_LEN 30
#define MAX_OUT_STRING_LEN 250

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