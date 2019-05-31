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
 	ret = inotify_add_watch(fd,tmp[cnt].d_name,IN_ALL_EVENTS);
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
    closedir(curDir);
    level--;
}

int main(int argc, char** argv) {
 int num, i;
 char* p;
 char inotify_entity[BUF_LEN];
 fd = inotify_init();
 int ret = inotify_add_watch(fd,argv[1],IN_ALL_EVENTS);
 //   lisOtDir(".");
 listDir(argv[1]);
 while(1) {
                 num = read(fd, inotify_entity, BUF_LEN);
                 for (p = inotify_entity; p < inotify_entity + num; ) {
                         printInotifyEvent((struct inotify_event *) p);
                         p+=sizeof(struct inotify_event) + ((struct inotify_event *)p)->len;
                 }
 }

}
void printInotifyEvent(struct inotify_event* event) {
        char buf[4096]="";
        sprintf(buf, "[%s] ", wd[event->wd]);
        strncat(buf, "{", 4096);
        if (event->mask & IN_ACCESS)            strncat(buf, "ACCESS, ", 4096);
        if (event->mask & IN_ATTRIB)            strncat(buf,"ATTRIB, ", 4096);
        if (event->mask & IN_CLOSE_WRITE)       strncat(buf,"CLOSE_WRITE, ", 4096);
        if (event->mask & IN_CLOSE_NOWRITE)     strncat(buf,"CLOSE_NOWRITE, ", 4096);
        if (event->mask & IN_CREATE)            strncat(buf,"CREATE, ", 4096);
        if (event->mask & IN_DELETE)            strncat(buf,"DELETE, ", 4096);
        if (event->mask & IN_DELETE_SELF)       strncat(buf,"DELETE_SELF, ", 4096);
        if (event->mask & IN_MODIFY)            strncat(buf,"MODIFY, ", 4096);
        if (event->mask & IN_MOVE_SELF)         strncat(buf,"MOVE_SELF, ", 4096);
        if (event->mask & IN_MOVED_FROM)        strncat(buf,"MOVED_FROM, ", 4096);
        if (event->mask & IN_MOVED_TO)          strncat(buf,"MOVED_TO, ", 4096);
        if (event->mask & IN_OPEN)                      strncat(buf,"OPEN, ", 4096);
        if (event->mask & IN_IGNORED)           strncat(buf,"IGNORED, ", 4096);
        if (event->mask & IN_ISDIR)                     strncat(buf,"ISDIR, ", 4096);
        if (event->mask & IN_Q_OVERFLOW)        strncat(buf,"Q_OVERFLOW, ", 4096);
        buf[strlen(buf)-2]='\0';
        strncat(buf, "}", 4096);
        sprintf(buf, "%s cookie=%d", buf, event->cookie);
        if (event->len>0)
        sprintf(buf, "%s name = %s\n", buf, event->name);
        else
                sprintf(buf, "%s name = null\n", buf);
        printf("%s", buf);
}

