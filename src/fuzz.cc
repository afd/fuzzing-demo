#include "src/solver.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string input(reinterpret_cast<const char *>(Data), Size);
  Parse(input, [](const std::string &) {});
  return 0; // Non-zero return values are reserved for future use.
}
