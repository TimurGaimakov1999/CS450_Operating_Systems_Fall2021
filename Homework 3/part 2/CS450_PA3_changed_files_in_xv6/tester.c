#include "types.h"
#include "user.h"

int main() {
  void* region = GetSharedPage(5, 3);
  for(int i = 0; i < 10; i++) {
    printf(1, "%d ", ((char*)region)[i]);
  }
  printf(1, "\n");

  // write
  strcpy(region, "region");

  // read
  printf(1, "%s\n", region);

  // FreeSharedPage(0);
  exit();
}
