#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
//Global Variables
int i, err;
int pid;
char *a[128];
char *cmds[128];
char *input_redir_file, *output_redir_file;
char str[512];
char ch[] = "\n";
int input_redir = 0, output_redir = 0, pipe_flag = 0, last = 0, input = 0;	//flag
int input_fd, output_fd;

//FUNTIONS

static char* skip_space_before(char* s)
{
 // printf("BINSPACE: %s", s);
  while (isspace(*s)) ++s;
  //printf("INSPACE: %s", s);
  return s;
}
void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    fflush(stdout);
}
void command_tokenizer(char *str){
	char * p;
	p = strtok (str," ");
	i = 0;
	while (p != NULL){
		a[i] = p;
		i++;
		p = strtok (NULL, " ");
	}
	//here array is ready with all arguments
	a[i] = NULL;
	//printf("FILLED\n\n\n%s\n",a[0]);
}
int pipe_execution(char *cmd, int input, int first, int last){
		char *new_cmd;
		new_cmd = strdup(cmd);
		command_tokenizer(cmd);
		int pfd[2], ret;
		
		ret = pipe(pfd);
		if(ret == -1){
			perror("pipe");
			return -1;
		}
		pid = fork();
		if(pid == 0){
			if(first == 1 && last == 0 && input == 0){
				close(1);
				close(pfd[0]);
				dup(pfd[1]);
				close(pfd[1]);
			}
			else if(first == 0 && last == 0 && input != 0){
				close(0);
				close(pfd[0]);
				dup(input);
				close(input);
				close(1);
				dup(pfd[1]);
				close(pfd[1]);
			}
			else{
				close(0);
				close(pfd[0]);
				close(pfd[1]);
				dup(input);
				close(input);
				
			}
			err = execvp(a[0], a);
			if(err == -1)
				exit(0);
		}
		else
			wait(0);
		if(last == 1)
			close(pfd[0]);
		if(input != 0)
			close(input);
		close(pfd[1]);
		return pfd[0];
}

void pipeline(char* str){
	char *temp;
	char *p;
	int first;
	int x = 0, pipes_count = 0, z;
	temp = strdup(str);
	input = 0;
	first = 1;
	p = strtok(temp, "|");
	while(p != NULL){
		cmds[x] =  p;
		x++;
		p = strtok(NULL, "|");
	}
	cmds[x] = NULL;
	pipes_count = x-1;
	//input = pipe_execution(cmds[z], input, 1, 0);
	for(z = 0; z < x-1; z++){
		input = pipe_execution(cmds[z], input, first, 0);
		first = 0;
	}
	input = pipe_execution(cmds[z], input, first, 1);
	input = 0;
	return;
}

void output_redirection(char* str){
	char token[16][128];
	char *given_cmd;
	char *p;
	int m = 0;
	given_cmd = strdup(str);
	p = strtok(given_cmd, ">");
	while(p != NULL){
		strcpy(token[m], p);
		m++;
		p = strtok(NULL, ">");
	}
	strcpy(token[1], skip_space_before(token[1]));
	output_redir_file = strdup(token[1]);
	command_tokenizer(token[0]);
}

void input_redirection(char *str){
	char token[16][128];
	char *given_cmd;
	char *p;
	int m = 0;
	given_cmd = strdup(str);
	p = strtok(given_cmd, "<");
	while(p != NULL){
		strcpy(token[m], p);
		m++;
		p = strtok(NULL, "<");
	}
	strcpy(token[1], skip_space_before(token[1]));
	input_redir_file = strdup(token[1]);
	command_tokenizer(token[0]);
}

void prompt(){
	char cwd[128];
	getcwd(cwd, sizeof(cwd));
	printf("Abhijeet@%s$~ ",cwd);
	return;
}
//child process
int child(char *str){
	if(strcmp(str, ch) == 0)
		return 0;
	if(strchr(str, '<')){
		input_redir = 1;					//input redirection
		input_redirection(str);
	}
	else if(strchr(str, '>')){
		output_redir = 1;
		output_redirection(str);			//output redirection		
	}
	else
		command_tokenizer(str);
	//check for input redirection
	if(input_redir == 1){
		close(0);
		input_fd = open(input_redir_file, O_RDONLY);
		if (input_fd < 0){
			perror(input_redir_file);
			input_redir = 0;
			return(EXIT_FAILURE);
		}
		input_redir=0;
	}
	if(output_redir == 1){
		close(1);
		output_fd = open(output_redir_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		if(output_fd < 0){
			perror(output_redir_file);
			output_redir = 0;
			return(EXIT_FAILURE);
		}
		output_redir = 0;
	}
	err = execvp(a[0], a);
	if(err != 0){
		perror("Error: ");
		exit(0);
	}
	return 0;
}

// driver code
int main(){
	int exitflag = 0;
	int len;
	signal(SIGINT, sigintHandler);
	while(1){
		prompt();
		fgets(str, 1024, stdin);
		len = strlen(str);
		str[len - 1] = '\0';		//fgets takes an extra char \n in str
		if(strcmp(str, "exit") == 0){
			exitflag = 1;
			break;
		}
		if(strchr(str, '|')){
			pipe_flag = 1;
			pipeline(str);
		}
		else{
			pid = fork();
			if(pid == 0){
				child(str);
			}
			else
				wait(0);
			}
	}
	if(exitflag == 1){
		printf("THANK YOU..\n");
		exit(0);
	}
	return 0;
}
