#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>



int main(int argc, char **argv) {
    FILE** f = (FILE**)malloc(sizeof(FILE*)*argc);
    f[0] = stdout;
    for (int i = 1; i < argc; ++i)  {
        f[i] = fopen(argv[i],"w");
    }

    while(!feof(stdin)) {
        char str[1024];
        fgets(str,1024,stdin);

        for(int i=0;i<argc;i++) {
            fputs(str,f[i]);
        }
        for(int i=1;i<argc;i++) {
            fclose(f[i]);
        }
        for (int i=1;i<argc;i++)  {
            f[i] = fopen(argv[i],"a");
        }
    }
    for(int i=1;i<argc;i++) {
        fclose(f[i]);
    }
    return 0;
}
