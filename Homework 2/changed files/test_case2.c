#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

void trapTester(void) {

  /**
   * Print First Counts*
  */

  // pretty print all system call count
  printf(1, "\n\t**********************\n");
  printf(1, "\t* Print First Counts *\n");
  printf(1, "\t**********************\n");
  countTraps();

  /**
   * Test System Calls*
  */

  printf(1, "\n\t***********************\n");
  printf(1, "\t* Sample System Calls *\n");
  printf(1, "\t***********************\n");

  // mkdir test
  printf(1, "\n Calling mkdir()");
  mkdir("Bye-bye");
  printf(1, "\n - Directory 'Bye-bye' has been created\n");

  // write test
  int sz, fd;
  char *c = (char *) malloc(sizeof("Bye-bye"));
  printf(1, "\n Calling write()");

  fd = open("Bye-bye.txt", O_CREATE | O_WRONLY);
  write(fd, "Bye-bye, Miss American Pie", strlen("Bye-bye"));
  printf(1, "\n - File 'Bye-bye.txt' has been created");
  printf(1, "\n - 'Bye-bye, Miss American Pie' has been written to the file");
  printf(1, "\n - %d bytes were written\n", sizeof(strlen("Bye-bye, Miss American Pie")));
  close(fd);

  // pretty print all system call count
  printf(1, "\n\t****************************************\n");
  printf(1, "\t*            Call All Traps After      *\n");
  printf(1, "\t* [SYS_mkdir] and [SYS_write] syscalls *\n");
  printf(1, "\t****************************************\n");

  // get traps counts
  countTraps();

  fd = open("Bye-bye.txt", O_RDONLY);
  sz = read(fd, c, 10);
  printf(1, "\n '%s' was read from the file\n", c);
  printf(1, "\n '%d' the number of bytes were read\n", sz);
  close(fd);

  // pretty print all system call count
  printf(1, "\n\t*************************\n");
  printf(1, "\t* Call All Traps After *\n");
  printf(1, "\t*  [SYS_READ] syscall  *\n");
  printf(1, "\t************************\n");

  // get traps counts
  countTraps();
  exit();
}

int main(int argc, char *argv[]) {
  trapTester();
}
