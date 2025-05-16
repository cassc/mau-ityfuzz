#pragma once

#include <llvm/ADT/APInt.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <chrono>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <cstdio>

#include "Arith256.h"
#include "BasicBlock.h"
#include "EEIModule.h"
#include "Instruction.h"
#include "Type.h"
#include "llvm/ADT/SmallString.h"
#include "Utils.h"  // for std:cerr
#include "config.h"
#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"

namespace dev {
namespace eth {
namespace trans {


uint32_t splitCode(const std::string& Bytecode);

void makeGlobalVar(llvm::Module& Module);
void makeGlobalBytecode(llvm::Module& Module, const std::string& Bytecode);

llvm::Function* wrapDeployer(llvm::Module& Module);
llvm::Function* wrapRuntimer(llvm::Module& Module, int64_t _txs_len);

class WASMCompiler {
 public:
  struct Options {
    /// Rewrite switch instructions to sequences of branches
    bool rewriteSwitchToBranches = true;
    /// Dump CFG as a .dot file for graphviz
    bool dumpCFG = false;
  };

  WASMCompiler(Options const &_options, llvm::Module& _module, EEIModule& _utilsM);
  llvm::Module* compileMain(const uint32_t EvmOffset, const std::string& Bytecode, const std::string& EntryName);
  void setBytecodeOffset(const uint64_t offset) { this->m_bytecodeOffset = offset; }
  uint64_t getBytecodeOffset() { return this->m_bytecodeOffset; }

 private:  
  uint64_t m_bytecodeOffset = 0;
  void dry_run_bb(BasicBlock const &bb, std::vector<uint64_t> &stack);
  void dry_run(std::vector<BasicBlock> &blocks);
  std::vector<BasicBlock> createBasicBlocks(code_iterator _begin,
                                            code_iterator _end);

  std::vector<llvm::ConstantInt *> compileBasicBlock(BasicBlock &_basicBlock);

  void resolveJumps(std::vector<BasicBlock> &blocks);
  void resolveGas(llvm::Function* Func);

  /// Compiler options
  Options const &m_options;

  /// Helper class for generating IR
  IRBuilder m_builder;
  
  /// Block with a jump table.
  llvm::BasicBlock *m_jumpTableBB = nullptr;

  /// Block with exit
  llvm::BasicBlock *m_exitBB = nullptr;

  /// Block with abort
  llvm::BasicBlock *m_abortBB = nullptr;

  /// Main program function
  llvm::Function *m_mainFunc = nullptr;

  // LLVM registers used in EVM memory
  llvm::Value *m_mem_ptr = nullptr;
  // LLVM registers used in EVM stack
  llvm::Value *m_stk_ref = nullptr;
  llvm::Value *m_stk_dep_ptr = nullptr;

  llvm::Value *m_targetPtr = nullptr;

  // llvm::Value *m_gasLeftPtr = nullptr;
  // llvm registers used in `msg`
  // llvm::Value *m_msg_sender = nullptr;

  // Runtime Env
  EEIModule& m_utilsM;
  GlobalStack *m_stack;
  llvm::Module& m_module;
  
  bool loadCaller(llvm::APInt &caller, const char hexdata[]) {
    uint32_t offset = 0;
    if (hexdata[0] == '0' && (hexdata[1] == 'x' || hexdata[1] == 'X')) offset = 2;
    std::cerr << "byte: ";
    uint8_t byte = 0;
    for (auto i = 0; i < 20; i++) {
      // std::sscanf(hexdata + offset + (19 - i) * 2, "%02hhx", &byte);
      std::sscanf(hexdata + offset + i * 2, "%02hhx", &byte);
      // std::cerr << byte << " ";
      printf("%02hhx", byte);
      caller <<= 8;
      caller |= byte;
    }
    return true;
  }


  inline llvm::Constant *createGlobalStringPtr(llvm::LLVMContext &Context,
                                               llvm::Module &Module,
                                               llvm::StringRef Str,
                                               const llvm::Twine &Name = "") {
    llvm::GlobalVariable *GV = createGlobalString(Context, Module, Str, Name);
    llvm::Constant *Zero =
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0);
    llvm::Constant *Indices[] = {Zero, Zero};
    return llvm::ConstantExpr::getInBoundsGetElementPtr(GV->getValueType(), GV,
                                                        Indices);
  }
  inline llvm::GlobalVariable *createGlobalString(
      llvm::LLVMContext &Context, llvm::Module &Module, llvm::StringRef Str,
      const llvm::Twine &Name = "") {
    llvm::Constant *StrConstant =
        llvm::ConstantDataArray::getString(Context, Str, false);
    auto *GV = new llvm::GlobalVariable(Module, StrConstant->getType(), true,
                                        llvm::GlobalValue::ExternalLinkage,
                                        StrConstant, Name);
    GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    GV->setAlignment(llvm::MaybeAlign(1));
    return GV;
  }
};

}  // namespace trans
}  // namespace eth
}  // namespace dev
