#include <sys/types.h>
#include <sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>

int main(){
	int i, pid, err;
	char a[16][16];
	char * pch;
	char str[32];
	  	
	//now a is filled with all arguments entered on single line and i contains num of args
	while(1){
		pid = fork();
		if(pid == 0){
			printf("prompt>");
			fgets(str, 32, stdin);
			pch = strtok (str," ");
  			i = 0;
  			while (pch != NULL)
  			{
			strcpy(a[i], pch);
			i++; 
	    	pch = strtok (NULL, " ");
			}
			//here array is ready with all arguments
			a[i-1][strlen(a[i-1])-1]= '\0';	// fgets also takes \n as its last character
			if(i == 2){
				err = execl(a[0], a[0], a[1], NULL);
			}
			else{
				err = execl(a[0], a[0], NULL);
			}
			if(err != 0)
				perror("Error: ");
		}
		else
			wait(0);
	}	
	return 0;
}
