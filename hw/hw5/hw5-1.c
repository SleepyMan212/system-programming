#include<stdio.h>
#include<unistd.h>

int main(){
	pid_t pid;
	pid = fork();
	pid = fork();
	pid = fork();
	pid = fork();
	puts("A fork have been generated");
	return 0;
}
