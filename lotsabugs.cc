#include <iostream>

int main(int argc, const char** argv) {
  int* A = new int[argc];
  for (int i = 0; i <= argc; i++) {
    A[i] = 1000000;
  }
  int temp = A[0];
  if (temp*temp < 0) {
    A[0] /= (argc - 1);
  }
  std::cout << "Phew, made it to the end!" << std::endl;
  return 0;
}
