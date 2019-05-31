/*
listDir.c
usage listDir .
*/

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/inotify.h>
#include <unistd.h>
#define BUF_LEN (1000 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void printInotifyEvent(struct inotify_event* event);

int level = 0;
int fd;
char wd[1000][10000];
void swapd(struct dirnet *a ,struct dirnet *b){
	struct dirent t;
	memcpy(&t,a,sizeof(t));
	memcpy(a,b,sizeof(t));
	memcpy(b,&t,sizeof(t));
}
void printName(char* type, char* name) {
    printf("%s", type);
    for (int i=0; i < level; i++)
        printf("  ");
    if (strcmp("d", type)==0)
        printf("+");
    else
        printf("|");
    printf("%s\n", name);
}

void listDir(char* pathName)
{
    level++;
    DIR* curDir = opendir(pathName);
    assert(curDir!=NULL);
    char* newPathName = (char*)malloc(PATH_MAX);
    struct dirent entry;
    struct dirent* result;
    struct dirent tmp[200];
    int ret;
    int cnt = 0;
    ret = readdir_r(curDir, &entry, &result);
    while(result != NULL) {
        if (strcmp(entry.d_name, ".") == 0 || strcmp(entry.d_name, "..") == 0) {
            ret = readdir_r(curDir, &entry, &result);
            assert(ret == 0);
            continue;
        }
        assert(ret == 0);
	tmp[cnt] = entry;
	cnt++;
        ret = readdir_r(curDir, &entry, &result);
        assert(ret == 0);        
    }

    for(int i=0; i<cnt-1; i++){
	for(int j=0; j<cnt-i-1; j++){
   	   if(tmp[j].d_type == DT_DIR && tmp[j+1].d_type == DT_DIR){
	   	if(strcmp(tmp[j].d_name,tmp[j+1].d_name) > 0){
			swapd(&tmp[j],&tmp[j+1]);
		}
	   }else if(tmp[j].d_type == DT_DIR || tmp[j+1].d_type == DT_DIR){
		if(tmp[j].d_type == DT_DIR){
			swapd(&tmp[j],&tmp[j+1]);
		}
	   }
	   else if(strcmp(tmp[j].d_name,tmp[j+1].d_name) > 0){
		swapd(&tmp[j],&tmp[j+1]);
	    }
	}
    }

    for(int i=0; i<cnt; i++){
        if (tmp[i].d_type == DT_LNK)
            printName("l", tmp[i].d_name);
        if (tmp[i].d_type == DT_REG)
            printName("f", tmp[i].d_name);
        if (tmp[i].d_type == DT_DIR) {
            printName("d", tmp[i].d_name);
            sprintf(newPathName, "%s/%s", pathName, tmp[i].d_name);
            printf("%s\n", newPathName);
            listDir(newPathName);
        }
    }
    closedir(curDir);
    level--;
}

int main(int argc, char** argv) {
 //   lisOtDir(".");
 listDir(argv[1]);

}

