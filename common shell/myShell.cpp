#include "myShell.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>

int main() {
  myShell obj;
  obj.run_myShell();
  return 0;
}
