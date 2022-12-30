#include "systemcalls.h"
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#define _XOPEN_SOURCE

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

	if (cmd == NULL) {
		return false; 
	}

	int ret; 
	ret = system(cmd); 
	
	if (ret < 0) {
		return false; 
	}

    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{

    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
    va_end(args);
    
    
    pid_t pid = fork();

	if (-1 == pid)
	{
		perror("Fork failed : ");
		return false;
	}
	
	else if (0 == pid)
	{
		execv(command[0], command);
		perror("execv() call failed!");
		
		//Note: Must EXIT here, not return false. why?
		
		exit(-1);
		return false; 

	}

	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
	
		if(WEXITSTATUS(status) == 0) {
			return true;
		}
		
		else {
			return false;
		}
	
	}
	return false;
    
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{

	
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
    va_end(args);

	
	int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	
	if (fd < 0) {
		perror("open");
		return false;
	}
	
	
    pid_t pid;
    pid = fork();
    
    if (pid == -1) { 
    	perror("fork");
	    return false;
	}
	
	if (pid == 0) {
			if (dup2(fd, 1) < 0) {
				perror("dup2");
				abort();
		}
		
		close(fd);
		execv(command[0], &command[0]);
		perror("execv");

		exit(-1);
		return false; 

	}
	

	
	int status;
    waitpid(pid, &status, 0);
   	close(fd);	
    
	if (WIFEXITED(status)) {
	
		if (WEXITSTATUS(status) == 0) {
			return true;
		} 
		
		else {
			return false;
		}
		
	}
	

    return true;
}
