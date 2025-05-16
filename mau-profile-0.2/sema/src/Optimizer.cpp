#include "Optimizer.h"

#include <llvm/ADT/Statistic.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
// #include <llvm/IR/CallSite.h>


namespace dev {
namespace eth {
namespace trans {

namespace {

class LongJmpEliminationPass : public llvm::FunctionPass {
  static char ID;

 public:
  LongJmpEliminationPass() : llvm::FunctionPass(ID) {}
  virtual bool runOnFunction(llvm::Function& _func) override;
};
char LongJmpEliminationPass::ID = 0;
bool LongJmpEliminationPass::runOnFunction(llvm::Function& _func) {
  return false;
}
}  // namespace

bool optimize(llvm::Module& _module) {
  auto pm = llvm::legacy::PassManager{};
  // pm.add(llvm::createFunctionInliningPass(2, 2, false));
  pm.add(llvm::createCFGSimplificationPass());
  pm.add(llvm::createInstructionCombiningPass());
  pm.add(llvm::createAggressiveDCEPass());
  pm.add(llvm::createLowerSwitchPass());
  return pm.run(_module);
}


bool prepare(llvm::Module& _module) {
  auto pm = llvm::legacy::PassManager{};
 
  pm.add(llvm::createReassociatePass());

  pm.add(llvm::createCFGSimplificationPass());  // 1

  pm.add(llvm::createDeadCodeEliminationPass());

  pm.add(llvm::createReassociatePass());

  // pm.add(llvm::createFunctionInliningPass(2, 2, false));

  pm.add(llvm::createAggressiveDCEPass());

  pm.add(llvm::createPromoteMemoryToRegisterPass());  // 2

  return pm.run(_module);
}

}  // namespace trans
}  // namespace eth
}  // namespace dev
