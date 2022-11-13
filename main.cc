#include "Executor.h"

using namespace shell;

int main(int argc, const char** argv) {
  dup2(1, 2);
  Executor shell{false};
  shell.Run();
  return 0;
}