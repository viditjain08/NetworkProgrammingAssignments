#include "functions.h"

int main (void)
{
	//signal(SIGINT,&handler);
	extern char **environ;
	char *path = getenv ("PATH");
	shell = (struct shell_info*) malloc(sizeof(struct shell_info));
	char buf[MAX_SIZE];
	int i;
    for (i = 0; i < NR_JOBS; i++) {
        shell->jobs[i] = NULL;
    }
	while(1){

		fflush (stdout);
		printf ("prompt> ");
		fflush (stdout);
		if (read (0, buf, MAX_SIZE) > 0){
			for (int i = 0; i < MAX_SIZE; i++)
						if(buf[i] == '\n')
					buf[i] = '\0';
			char temp[MAX_SIZE];
			strcpy(temp,buf);
			char *first=strtok(temp," ");
			if(strcmp(first,"daemonize")==0)
			{
				char *second=strtok(NULL," ");
				if(second!=NULL && strlen(second)>2 )//&& second[strlen(second)-1]=='c' && second[strlen(second-2)]=='.')
				{
					FILE *fp= NULL;
					pid_t process_id = 0;
					pid_t sid = 0;
					// Create child process
					process_id = fork();
					// Indication of fork() failure
					if (process_id < 0)
					{
					printf("fork failed!\n");
					// Return failure in exit status
					exit(1);
					}
					// PARENT PROCESS. Need to kill it.
					if (process_id > 0)
					{
					printf("process_id of child process %d \n", process_id);
					// return success in exit status
					exit(0);
					}
					//unmask the file mode
					umask(0);
					//set new session
					sid = setsid();
					if(sid < 0)
					{
					// Return failure
					exit(1);
					}
					// Change the current working directory to root.
					//chdir("/");
					// Close stdin. stdout and stderr
					close(STDIN_FILENO);
					close(STDOUT_FILENO);
					close(STDERR_FILENO);
					// Open a log file in write mode.
					//fp = fopen ("Log.txt", "w+");
					while (1)
					{
					//Dont block context switches, let the process sleep for some time
					sleep(1);
					//fprintf(fp, "Logging info...\n");
					//fflush(fp);
					// Implement and call some function that does core work for this daemon.

					char program[100];
					strcpy(program,"./");
					strcat(program,second);
					char *args[] = {"cc", "-o", "./a.out", program, NULL};
	   				execv("/usr/bin/cc", args);
					execl("./a.out", "a.out", NULL);
					}
					fclose(fp);
			}
			else
			{
				printf("Invalid Argument\n"); // accepting only c programs for daemonize
				continue;
			}
			}

			int nofargs=0;

			char *command = getcommand(buf,&nofargs); //nofargs updated too
			if(command==NULL){//if ENTER
				continue;
			}

			if(strcmp(command,"exit") == 0) {   //ability to exit from shell
				printf("Exiting prompt\n");
				exit(0);
			}
			char *p = getfile(path,command);
			bool back_pr = false;
			char **v = getargv(buf,&nofargs,&back_pr);

			// if (strcmp(v[0],"sc")==0) {
			// 	customcommands(v,nofargs);
			// 	continue;
			// }
			int ret = fork ();


			if (ret == 0){
				setpgid(0,0);//new process group for every command

				if (p == NULL) {
					// if(buf[0]=='b' && buf[1]=='g' && buf[2]==' ')
					// {
					// 	char tar[10];
					// 	int k=0;
					// 	int i=2;
					// 	while(i<MAX_SIZE && command[i]==' ')
					// 		i++;
					// 	if(i==MAX_SIZE)
					// 	{
					// 		printf ("Command %s not found\n", buf);
					// 		exit (0);
					// 	}
					// 	// else
					// 	// {
					// 	// 	if(buf[i])
					// 	// }
					//
					// else{
						printf ("Command %s not found\n", buf);
						exit (0);
					}

				job *j=(job *)malloc(sizeof(job));
				j->pid=getpid();

				insert_job(j);

				parsecommand(path,p,v,nofargs);
			}

			else {

				signal(SIGTTOU, SIG_IGN);
				int status;
				if(back_pr==false){ // if foreground process
					int s = tcsetpgrp(0,ret); //give child access to terminal input
					wait (&status);
					s = tcsetpgrp(0,getpgrp()); //take back the terminal input
					printf ("\n||PID = %d\n||Status = %d\n\n\n", ret, status);
				}
			}
		}
	}
}
