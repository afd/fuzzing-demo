#include "src/solver.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string input(reinterpret_cast<const char*>(Data), Size);
  Parse(input, [](const std::string&) {});
  return 0; // Non-zero return values are reserved for future use.
}











//#include <random>
//#include <sstream>
//#include <cstring>
//
// extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size,
//                                          size_t MaxSize, unsigned int Seed) {
//  std::string input(reinterpret_cast<const char *>(Data), Size);
//  std::optional<SatInstance> maybe_instance = Parse(input, [](const
//  std::string &) {}); if (!maybe_instance.has_value()) {
//    return 0;
//  }
//  int32_t new_var = maybe_instance->num_vars + 1;
//
//  std::mt19937 gen(Seed);
//  std::uniform_int_distribution<> distr(3, 20); // define the range
//
//  std::stringstream stringstream;
//  bool add_new_clause = (distr(gen) % 2) == 0;
//  stringstream << "p cnf " << new_var << " " << (add_new_clause ?
//  maybe_instance->clauses.size() + 1 : maybe_instance->clauses.size()) << " ";
//  for (auto& clause : maybe_instance->clauses) {
//    for (auto literal : clause) {
//      stringstream << literal << " ";
//    }
//    if ((distr(gen) % 2) == 0) {
//      if ((distr(gen) % 2) == 0) {
//        stringstream << "-";
//      }
//      stringstream << new_var << " ";
//    }
//    stringstream << "0 ";
//  }
//  if (add_new_clause) {
//    for (size_t i = 0; i < distr(gen); i++) {
//      if ((distr(gen) % 2) == 0) {
//        stringstream << "-";
//      }
//      stringstream << distr(gen) << " ";
//    }
//    stringstream << "0";
//  }
//  if (stringstream.str().size() > MaxSize) {
//    return 0;
//  }
//  std::memcpy(Data, stringstream.str().c_str(), stringstream.str().size());
//  return stringstream.str().size();
//}
