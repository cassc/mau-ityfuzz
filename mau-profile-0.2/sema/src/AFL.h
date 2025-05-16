#pragma once
#include <llvm/Support/raw_ostream.h>


#include "Utils.h"
#include "Common.h"
#include "Type.h"



#define AFL_R(x) (random() % (x))

namespace llvm {
class Module;
}

namespace afl {

bool instrument(llvm::Module& _module);

bool instrumentDeployer(llvm::Module& _module);

}  // namespace afl
