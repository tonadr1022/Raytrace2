#include "EAssert.hpp"

void HandleAssert(const char *msg, const char *condition, const char *filename,
                  uint64_t lineNumber) {
  std::cerr << "Assert Failed: " << msg << "\nCondition: " << condition << "\nFile: " << filename
            << "\nLine: " << lineNumber << '\n';
}
