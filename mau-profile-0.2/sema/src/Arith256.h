#pragma once

#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>

#include <iomanip>
#include <iostream>

#include "Type.h"
// #include "preprocessor/llvm_includes_end.h"
// #include "preprocessor/llvm_includes_start.h"

#include "CompilerHelper.h"

namespace arith {

llvm::Function* getUDiv256Func(llvm::Module& _module);
llvm::Function* getURem256Func(llvm::Module& _module);
llvm::Function* getUDivRem256Func(llvm::Module& _module);
llvm::Function* getSDiv256Func(llvm::Module& _module);
llvm::Function* getSRem256Func(llvm::Module& _module);
llvm::Function* getSDivRem256Func(llvm::Module& _module);
llvm::Function* getMulFunc(llvm::Module& _module, uint64_t BitWidth,
                           const std::string funcName);
llvm::Function* getMul256Func(llvm::Module& _module);
llvm::Function* getMul128Func(llvm::Module& _module);
llvm::Function* getExpFunc(llvm::Module& _module);

}  // namespace arith
