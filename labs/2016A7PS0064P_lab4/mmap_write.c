#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

void wr()  {
    int fd = open("random_wr.dat", O_RDWR | O_CREAT, (mode_t)0600);
    char buffer[10000];

   for (int i = 0; i < 100000; ++i) {
      for (int j = 0; j < 10000; ++j) buffer[j] = rand();
      write(fd, buffer, sizeof(char)*10000);
   }
   close(fd);

}

void mm() {
    int fd = open("random_mm.dat", O_RDWR | O_CREAT, (mode_t)0600);
    int buffer[1000];
    size_t textsize = 1000000 * 1000;
    lseek(fd, textsize-1, SEEK_SET);
    write(fd, "", 1);
    char *map = mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for (size_t i = 0; i < textsize; i++)
    {
        map[i] = rand();
    }

    msync(map, textsize, MS_SYNC);
    munmap(map, textsize);
    close(fd);
}
int main() {
    int i, j;
    clock_t t;
    t = clock();
    wr();
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

    printf("Writing 1GB using write() took %f seconds to execute \n", time_taken);

    t = clock();
    mm();
    t = clock() - t;
    time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

    printf("Writing 1GB using mmap() took %f seconds to execute \n", time_taken);


  return 0;
}
