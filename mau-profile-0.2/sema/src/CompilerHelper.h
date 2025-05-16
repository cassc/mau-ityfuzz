#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/NoFolder.h>

#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"

namespace dev {
namespace eth {
namespace trans {

// using IRBuilder = llvm::IRBuilder<>; // enable the constant folder
using IRBuilder =
    llvm::IRBuilder<llvm::NoFolder>;  // disable the constant folder

struct InsertPointGuard {
  explicit InsertPointGuard(llvm::IRBuilderBase& _builder);
  ~InsertPointGuard() { m_builder.restoreIP(m_insertPoint); }

 private:
  llvm::IRBuilderBase& m_builder;
  llvm::IRBuilderBase::InsertPoint m_insertPoint;
};

}  // namespace trans
}  // namespace eth
}  // namespace dev