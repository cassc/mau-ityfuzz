#pragma once

#include <llvm/Support/raw_ostream.h>

#include <cstdint>
#include <iostream>

namespace dev {
namespace evmtrans {

using byte = uint8_t;
using code_iterator = byte const*;

#define UNTESTED assert(false)

}  // namespace evmtrans
}  // namespace dev

// using namespace llvm;
// using namespace std;

enum Verbosity { v0 = 0, v1, v2, v3 };

// ------------------------------------------------------------------------------------------------
// Display various information to the user.
//
class Info {
 public:
  /* types of information */
  enum InfoType { STATUS = 0, REMARK, EMPH, WARNING, FATAL };

  static int verbosityLevel;

  Info(int type, int level) : type(type), currLevel(level) {}
  ~Info() {}

  /* overload stream operator to prepend the appropriate label */
  friend llvm::raw_ostream& operator<<(llvm::raw_ostream& strm, const Info& s) {
    switch (s.type) {
      // ----------------------------------------------------------------------------
      case STATUS:
        if (s.currLevel >
            verbosityLevel) {    // if verbosity level exceeds current level,
          return llvm::nulls();  // don't print anything
        }

        llvm::outs().changeColor(llvm::raw_ostream::GREEN, /* bold */ true,
                                 /* bg */ false);
        llvm::outs() << "[STATUS]";
        break;

      // ----------------------------------------------------------------------------
      case REMARK:
        if (s.currLevel > verbosityLevel) {
          return llvm::nulls();
        }

        llvm::outs().changeColor(llvm::raw_ostream::BLUE, true, false);
        llvm::outs() << "[REMARK]";
        break;

      // ----------------------------------------------------------------------------
      case EMPH:
        if (s.currLevel > verbosityLevel) {
          return llvm::nulls();
        }

        llvm::outs().changeColor(llvm::raw_ostream::MAGENTA, true, false);
        llvm::outs() << "[STATUS] ";

        /* keep the color changed for the rest of the text */
        llvm::outs().changeColor(llvm::raw_ostream::GREEN, true, false);

        return strm;

      // ----------------------------------------------------------------------------
      case WARNING:
        llvm::errs().changeColor(llvm::raw_ostream::YELLOW, true, false);
        llvm::errs() << "[WARNING]";
        break;

      // ----------------------------------------------------------------------------
      case FATAL:
        llvm::errs().changeColor(llvm::raw_ostream::RED, true, false);
        llvm::errs() << "[ERROR]";
    }

    llvm::errs().resetColor();
    llvm::errs() << " ";

    return strm;
  }

 private:
  int type;
  int currLevel;
};

// ------------------------------------------------------------------------------------------------
/* that's definitely not the best way to do it, but it works */
// #define info(level) llvm::outs() << Info(Info::STATUS, Verbosity::level)
// #define remark(level) llvm::outs() << Info(Info::REMARK, Verbosity::level)
// #define emph(level) llvm::outs() << Info(Info::EMPH, Verbosity::level)
// #define warning() llvm::errs() << Info(Info::WARNING, v0)
// #define fatal() llvm::errs() << Info(Info::FATAL, v0)

// ------------------------------------------------------------------------------------------------