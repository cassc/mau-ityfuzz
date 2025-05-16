#pragma once
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <unistd.h>

#include "Utils.h"
#include "Arith256.h"
#include "Common.h"
#include "Type.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace llvm {
class Module;
}

namespace san {

void enhance(llvm::Module& _module, std::string FSanitize);

}  // namespace san
