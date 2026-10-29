#define _POSIX_C_SOURCE 200809L
#include "cupidconf.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(void) {
    char dir[] = "/tmp/cupidconf-XXXXXX";
    assert(mkdtemp(dir));
    assert(chdir(dir) == 0);
    assert(setenv("HOME", dir, 1) == 0);
    const char *name = "config $(touch EXECUTED) [literal].conf";
    FILE *fp = fopen(name, "w");
    assert(fp);
    fputs("# comment\r\npath = first\r\npath = second\ncache = ~/cache\n", fp);
    fputs("ignore = *.txt\nignore = build_*\nlong = ", fp);
    for (int i=0;i<12000;i++) fputc('x',fp);
    fputs("\nlast = yes\n = skipped\nmalformed\n",fp);
    assert(fclose(fp) == 0);
    cupidconf_t *conf = cupidconf_load(name);
    assert(conf);
    assert(access("EXECUTED", F_OK) != 0);
    assert(!strcmp(cupidconf_get(conf,"path"),"first"));
    assert(strlen(cupidconf_get(conf,"long")) == 12000);
    assert(!strcmp(cupidconf_get(conf,"last"),"yes"));
    char home[4096];
    snprintf(home,sizeof(home),"%s/cache",dir);
    assert(!strcmp(cupidconf_get(conf,"cache"),home));
    int count=99;
    char **list=cupidconf_get_list(conf,"pa*",&count);
    assert(count==2 && list);
    assert(!strcmp(list[0],"first") && !strcmp(list[1],"second"));
    free(list);
    assert(cupidconf_value_in_list(conf,"ignore","x.txt"));
    assert(cupidconf_value_in_list(conf,"ignore","build_test"));
    assert(!cupidconf_value_in_list(conf,"ignore","x.c"));
    count=99;
    assert(!cupidconf_get_list(NULL,"path",&count) && count==0);
    cupidconf_free(conf);
    assert(!cupidconf_load(NULL) && errno==EINVAL);
    assert(rename(name,"home file.conf")==0);
    conf=cupidconf_load("~/home file.conf");
    assert(conf);
    cupidconf_free(conf);
    assert(unsetenv("HOME")==0);
    fp=fopen("nul.conf","wb");
    assert(fp);
    assert(fwrite("key=ok\0hidden=bad\n",1,18,fp)==18);
    assert(fclose(fp)==0);
    assert(!cupidconf_load("nul.conf"));
    assert(!cupidconf_load("."));
    assert(unlink("nul.conf")==0 && unlink("home file.conf")==0);
    assert(chdir("/")==0 && rmdir(dir)==0);
    puts("CupidConf regressions passed");
}
