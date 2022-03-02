#include <iostream>

int main(int argc, const char** argv) {
  int* A = new int[argc];
  for (int i = 0; i < argc; i++) {
    A[i] = 1000000;
  }
  unsigned temp = A[0];
  if (temp*temp > 1) {
    A[0] /= (argc);
  }
  std::cout << "Phew, made it to the end!" << std::endl;
  delete[] A;
  return 0;
}
