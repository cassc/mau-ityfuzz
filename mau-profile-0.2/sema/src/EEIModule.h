#pragma once

// #include "Type.h"
#include <llvm/IR/GlobalVariable.h>

#include "BasicBlock.h"
#include "CompilerHelper.h"
#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"

namespace dev {
namespace eth {
namespace trans {

 
class EEIModule {

private:
  llvm::Module *TheModule = nullptr;
  llvm::LLVMContext &VMContext;
  
 public:

  EEIModule(llvm::Module &_module);
  

  llvm::Function* fn_sync_warp();
  llvm::Function *fn_mem_p1_p0();
  llvm::Function *fn_mem_p4_p0();
  llvm::Function *fn_clear_bitmap();
  llvm::Function *fn_transfer();

  llvm::Function *fn_malloc();
  llvm::Function *fn_free();

  llvm::Function *fn_thread_id();

  llvm::Function *fn_update_bits();
  llvm::Function *fn_classify_counts();

  llvm::Function* getMstore();
  llvm::Function* getMload();

  llvm::Function* getMemcpy();
  llvm::Function* getMutateCalldata();
  llvm::Function* getCalldataCopy();
  llvm::Function* getCalldataLoad();
  llvm::Function* getMemcpyGtoH();
  llvm::Function* getHashFunc();
  llvm::Function* getStorageStore();
  llvm::Function* getStorageLoad();

};

}  // namespace trans
}  // namespace eth
}  // namespace dev