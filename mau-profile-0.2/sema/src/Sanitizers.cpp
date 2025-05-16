#include "Sanitizers.h"
#include <queue>
#include <set>

#include "config.h"

using namespace llvm;
namespace sema = dev::eth::trans;

namespace san {

static const std::string SigFn = "addBugSet";
static const std::string TidFn = "get_thread_id";

struct SigPass : public ModulePass {
  static char ID;
 public:
  SigPass () : ModulePass(ID) {}
  virtual bool runOnModule(Module& M) override {
    if (M.getFunction(SigFn)) 
      return false;

    llvm::Function::Create(
        llvm::FunctionType::get(sema::Type::Void, {sema::Type::Int32Ty, 
          sema::Type::Int8Ty, sema::Type::Int32Ty}, false),
        llvm::Function::ExternalLinkage, SigFn, M);
    return true;
  }
};

struct IBSan : public FunctionPass {
  static char ID;

 public:
  IBSan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    if (_func.getName() != CUDA_CONTRACT_FUNC) return false;
    llvm::errs() << "[+] IBSan is running.\n";
    bool modified = false;
    llvm::Module& Module = *_func.getParent();

    /* handle every basic block */
    std::vector<Instruction*> ips;
    for (auto& _bb : _func)
      for (auto it = _bb.begin(); it != _bb.end(); ++it)
        if ((*it).hasMetadata(c_intsan)) ips.push_back(&(*it));

    for (auto& IP : ips) {
      IRBuilder<> Builder(&(*IP));
      auto LHS = IP->getOperand(0);
      auto RHS = IP->getOperand(1);
      llvm::Value* flag = nullptr;

      auto N = IP->getMetadata(c_pc);
      auto PC = llvm::cast<llvm::ValueAsMetadata>(N->getOperand(0))->getValue();

      switch (IP->getOpcode()) {
        case Instruction::Add: {
          llvm::Value* Sum = Builder.CreateAdd(LHS, RHS);
          flag = Builder.CreateICmpULT(Sum, LHS, "SUM < LHS");
          break;
        }
        case Instruction::Sub: {
          flag = Builder.CreateICmpULT(LHS, RHS, "LHS < RHS");
          break;
        }
        case Instruction::Mul: {
          auto isLHSZ =
              Builder.CreateICmpEQ(LHS, Builder.getIntN(256, 0), "LHS==0");
          auto res = Builder.CreateMul(LHS, RHS);
          llvm::Function* func = arith::getUDiv256Func(Module);
          llvm::Value* pd = Builder.CreateAlloca(sema::Type::Word);
          Builder.CreateStore(res, pd);
          llvm::Value* pn = Builder.CreateAlloca(sema::Type::Word);
          Builder.CreateStore(LHS, pn);
          llvm::Value* presult = Builder.CreateAlloca(sema::Type::Word);

          Builder.CreateCall(func, {pd, pn, presult});
          llvm::Value* ver = Builder.CreateLoad(sema::Type::Word, presult);
          flag = Builder.CreateSelect(
              isLHSZ, Builder.getFalse(),
              Builder.CreateICmpNE(ver, RHS, "RES / LHS != RHS"));
          break;
        }
        case Instruction::Call: {
          // UDIV or SDIV
          // auto nPtr = IP->getOperand(2);
          // auto n = Builder.CreateLoad(nPtr, sema::Type::WordPtr);
          // flag = Builder.CreateICmpEQ(n, Builder.getIntN(256, 0), "n == 0");
          break;
        }
      }

      if (flag != nullptr) {
        modified = true;
        // auto constant = llvm::dyn_cast<llvm::ConstantInt>(PC);
        // llvm::errs() << "[+] IntSan => " << IP->getOpcodeName() << " at PC@"
        // << constant->getZExtValue() << "\n";
        auto Fault = Builder.CreateSelect(flag, Builder.getInt8(SIG_INT_SAN),
                                          Builder.getInt8(SIG_EXEC_NONE));
        llvm::Value* Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
        Builder.CreateCall(Module.getFunction(SigFn), {Thread, Fault, PC});
      }
    }
    return modified;
  }
};

struct RESan : public FunctionPass {
  static char ID;

 public:
  RESan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    if (_func.getName() != CUDA_CONTRACT_FUNC) return false;
    llvm::Module &Module = *_func.getParent();

    llvm::errs() << "[+] RenSan is running.\n";

    if (!_func.getParent()->getFunction("__device_sstore"))
      return false;

    llvm::Value *AFLState = nullptr;
    for (auto& inst : _func.getEntryBlock()) {
      if (inst.getName() == "__afl_state") {
        AFLState = &inst;
        break;
      }
    }

    std::vector<Instruction*> EnterCalls;
    for (auto& _bb : _func) {
      for (auto& inst : _bb) {
        if (!inst.hasMetadata(c_rensan)) continue;

        // renentrancy is safe when gas < 2300 units
        auto gas = llvm::cast<llvm::ValueAsMetadata>(
            inst.getMetadata(c_rensan)->getOperand(0))->getValue();
        if (auto constant = llvm::dyn_cast<llvm::ConstantInt>(gas))
          if (constant->getZExtValue() <= 2300) continue;

        // renentrancy is safe when there is no raw between state variables
        auto U = &inst;
        while (!U->isTerminator()) {
          U = U->getNextNonDebugInstruction(); 
          // std::cerr << U->getOpcodeName() << "\n";
          if (!llvm::isa<llvm::CallInst>(U) && !llvm::isa<llvm::InvokeInst>(U))
            continue;
          auto *Call = llvm::cast<llvm::CallInst>(U);
          if (Call->getCalledFunction()->getName() == "__device_sstore") {
            EnterCalls.push_back(&inst);
            std::cerr << "[RenSan] found one nest call with RAW\n";
            break;
          }
        }

        std::set<BasicBlock *> visited {inst.getParent()};
        std::queue<llvm::BasicBlock *> DFS;
        for (uint32_t i = 0; i < U->getNumSuccessors(); i++)
          DFS.push(U->getSuccessor(i));
        
        while (!DFS.empty()) {
          llvm::BasicBlock *H = DFS.front();  
          DFS.pop();
          if (visited.find(H) != visited.end()) continue;
          else  visited.insert(H); 
          
          U = &H->front();
          while (!U->isTerminator()) {
            if (llvm::isa<llvm::CallInst>(U) || llvm::isa<llvm::InvokeInst>(U)) {
              auto *Call = llvm::cast<llvm::CallInst>(U);
              if (Call->getCalledFunction()->getName() == "__device_sstore") {
                EnterCalls.push_back(&inst);
                std::cerr << "[RenSan] found one call with RAW\n";
                break;
              }
            }
            U = U->getNextNonDebugInstruction(); 
          }
          // unfind in this basic block, then search its successors
          if (U->isTerminator() && !llvm::isa<llvm::SwitchInst>(U))
            for (uint32_t _i = 0; _i < U->getNumSuccessors(); _i++)
              DFS.push(U->getSuccessor(_i));
        } 
      }
    }

    // no cross-contract call
    if (EnterCalls.size() == 0 || AFLState == nullptr) return false;

    llvm::Function *fnRuntimer = Module.getFunction(CUDA_CONTRACT_FUNC);    
    Argument* mutex = fnRuntimer->getArg(7);

    IRBuilder<> Builder(Module.getContext());

    // auto renBB = BasicBlock::Create(Module.getContext(), "", &_func);
    // Builder.SetInsertPoint(&(*renBB));
      
    for (auto& IP : EnterCalls) {
      // export bug    
      auto PC = llvm::cast<llvm::ValueAsMetadata>(
          IP->getMetadata(c_pc)->getOperand(0))->getValue();
          
      auto exportBB = BasicBlock::Create(Module.getContext(), "", &_func);  
      Builder.SetInsertPoint(&(*exportBB));
      llvm::Value* State = Builder.CreateLoad(AFLState);
      llvm::Value* Changed =  Builder.CreateICmpNE(State, Builder.getInt32(0));
      llvm::Value* Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
      llvm::Value* Bug = Builder.CreateSelect(Changed, Builder.getInt8(SIG_REN_SAN), Builder.getInt8(SIG_EXEC_NONE));
      Builder.CreateCall(Module.getFunction(SigFn), {Thread, Bug, PC});
      Builder.CreateRet(Builder.getInt32(1)); 

      Builder.SetInsertPoint(&(*IP));
      // auto Gas = llvm::cast<llvm::ValueAsMetadata>(
          // IP->getMetadata(c_rensan)->getOperand(0))->getValue();
      // llvm::Value* GasSufficient = Builder.CreateICmpUGT(
          // gas, Builder.getInt64(2300));
      // llvm::Value* UnLocked = Builder.CreateICmpEQ(
          // mutex, Builder.getFalse(), "rensan.check");

      // re-enter 
      llvm::Value* Locked = Builder.CreateICmpEQ(
          mutex, Builder.getTrue(), "locked");
      CallInst* Call = Builder.CreateCall(
        fnRuntimer, {fnRuntimer->getArg(0), fnRuntimer->getArg(1),
        fnRuntimer->getArg(2), fnRuntimer->getArg(3),
        fnRuntimer->getArg(4), fnRuntimer->getArg(5),
        fnRuntimer->getArg(6), Builder.getTrue()});
        
      SplitBlockAndInsertIfThen(Locked, Call, false, nullptr, 
          (DomTreeUpdater *)nullptr, nullptr, exportBB);
      // IP->eraseFromParent(); // remove the @donothing()
    }

    return true;
  }
};

struct TOSan : public FunctionPass {
  static char ID;
 public:
  TOSan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    bool modified = false;
    llvm::Module &Module = *_func.getParent();
    if (_func.getName() != CUDA_CONTRACT_FUNC) return modified;

    llvm::errs() << "[+] OrgSan is running.\n";
    std::vector<Instruction*> TxOrginCalls;
    for (auto &BB : _func) {
      /* handle the tx.origin calls */
      for (auto it = BB.begin(); it != BB.end(); ++it) {
        Instruction* I = &(*it);
        if (I->hasMetadata(c_orgsan)) TxOrginCalls.push_back(I);         
      }
    }

    for (auto& IP : TxOrginCalls) {
      IRBuilder<> Builder(&(*IP));
      Builder.SetInsertPoint(IP);
      auto N = IP->getMetadata(c_pc);
      auto PC = llvm::cast<llvm::ValueAsMetadata>(N->getOperand(0))->getValue();
      auto Fault = Builder.getInt8(SIG_ORG_SAN);
      llvm::Value *Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
      Builder.CreateCall(Module.getFunction(SigFn), {Thread, Fault, PC});
    }
    return modified;
  }
};

struct SCSan : public FunctionPass {
  static char ID;
 public:
  SCSan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    bool modified = false;
    llvm::Module &Module = *_func.getParent();
    if (_func.getName() != CUDA_CONTRACT_FUNC) return modified;

    llvm::errs() << "[+] ScgSan is running.\n";
    std::vector<Instruction*> Calls;
    for (auto &BB : _func) {
      /* handle the tx.origin calls */
      for (auto it = BB.begin(); it != BB.end(); ++it) {
        Instruction* I = &(*it);
        if (I->hasMetadata("scsan")) Calls.push_back(I);         
      }
    }

    for (auto& IP : Calls) {
      IRBuilder<> Builder(&(*IP));
      Builder.SetInsertPoint(IP);
      auto N = IP->getMetadata(c_pc);
      auto PC = llvm::cast<llvm::ValueAsMetadata>(N->getOperand(0))->getValue();
      auto Fault = Builder.getInt8(SIG_SUC_SAN);
      llvm::Value *Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
      Builder.CreateCall(Module.getFunction(SigFn), {Thread, Fault, PC});
    }
    return modified;
  }
};

struct BDSan : public FunctionPass {
  static char ID;
 public:
  BDSan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    bool modified = false;
    llvm::Module &Module = *_func.getParent();
    if (_func.getName() != CUDA_CONTRACT_FUNC) return modified;

    llvm::errs() << "[+] BDSan is running.\n";
    std::vector<Instruction*> Calls;
    for (auto &BB : _func) {
      /* handle the tx.origin calls */
      for (auto it = BB.begin(); it != BB.end(); ++it) {
        Instruction* I = &(*it);
        if (I->hasMetadata(c_bsdsan)) Calls.push_back(I);         
      }
    }

    for (auto& IP : Calls) {
      IRBuilder<> Builder(&(*IP));
      Builder.SetInsertPoint(IP);
      auto N = IP->getMetadata(c_pc);
      auto PC = llvm::cast<llvm::ValueAsMetadata>(N->getOperand(0))->getValue();
      auto Fault = Builder.getInt8(SIG_BSD_SAN);
      llvm::Value *Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
      Builder.CreateCall(Module.getFunction(SigFn), {Thread, Fault, PC});
    }
    return modified;
  }
};

struct MESan : public FunctionPass {
  static char ID;
 public:
  MESan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    bool modified = false;
    llvm::Module &Module = *_func.getParent();
    if (_func.getName() != CUDA_CONTRACT_FUNC) return modified;

    llvm::errs() << "[+] MESan is running.\n";
    std::vector<Instruction*> Calls;
    for (auto &BB : _func) {
      for (auto it = BB.begin(); it != BB.end(); ++it) {
        Instruction* I = &(*it);
        if (!I->hasMetadata(c_msesan))
          continue;
        if (I->users().empty())
          Calls.push_back(I);         
      }
    }
    llvm::errs() << "[+] MeSan =>" << Calls.size() << " \n";

    for (auto& IP : Calls) {
      IRBuilder<> Builder(&(*IP)); 
      auto N = IP->getMetadata(c_pc);
      auto PC = llvm::cast<llvm::ValueAsMetadata>(N->getOperand(0))->getValue();
      llvm::Value *Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
      Builder.CreateCall(Module.getFunction(SigFn), {Thread, Builder.getInt8(SIG_MSE_SAN), PC});
      llvm::errs() << "[+] MeSan => " << IP->getOpcodeName() 
          << " at PC@" << llvm::dyn_cast<llvm::ConstantInt>(PC)->getZExtValue() << "\n";
    }
    return modified;
  }
};

struct ELSan : public FunctionPass {
  static char ID;
 public:
  ELSan() : FunctionPass(ID) {}
  virtual bool runOnFunction(Function& _func) override {
    bool modified = false;
    // llvm::Module &Module = *_func.getParent();
    // if (_func.getName() != CUDA_CONTRACT_FUNC) return modified;

    // llvm::errs() << "[+] ELSan is running.\n";
    // std::vector<Instruction*> Calls;
    // for (auto &BB : _func) {
    //   for (auto it = BB.begin(); it != BB.end(); ++it) {
    //     Instruction* I = &(*it);
    //     if (I->hasMetadata(c_elksan))
    //       Calls.push_back(I);         
    //   }
    // }
    // llvm::Value *Callvalue = Module.getFunction(CUDA_CONTRACT_FUNC)->getArg(2);
    // for (auto& IP : Calls) {
    //   IRBuilder<> Builder(&(*IP));
      
    //   auto PcN = IP->getMetadata(c_pc);
    //   auto PC = llvm::cast<llvm::ValueAsMetadata>(PcN->getOperand(0))->getValue();

    //   auto ElN = IP->getMetadata(c_elksan);
    //   auto Addr = llvm::cast<llvm::ValueAsMetadata>(ElN->getOperand(0))->getValue();
    //   auto Benefit = llvm::cast<llvm::ValueAsMetadata>(ElN->getOperand(1))->getValue();

    //   llvm::Value *Caller = Builder.CreateLoad(
    //       sema::Type::AddressPtrTy, Module.getNamedGlobal(CALLER_REF));
      
    //   auto Gain = Builder.CreateSelect(Builder.CreateICmpEQ(Addr, Caller), 
    //       Benefit, Builder.getInt64(0));
    //   auto Fault = Builder.CreateSelect(Builder.CreateICmpUGT(Gain, Callvalue), 
    //       Builder.getInt8(SIG_ELK_SAN), Builder.getInt8(SIG_EXEC_NONE));
     
    //   llvm::Value *Thread = Builder.CreateCall(Module.getFunction(TidFn), {});
    //   Builder.CreateCall(Module.getFunction(SigFn), {Thread, Fault, PC});      
    //   // llvm::errs() << "[+] ELSan => " << IP->getOpcodeName() 
    //       // << " at PC@" << llvm::dyn_cast<llvm::ConstantInt>(PC)->getZExtValue() << "\n";
    // }
    return modified;
  }
};

char SigPass::ID = 0;

char IBSan::ID = 1;
char RESan::ID = 2;
char TOSan::ID = 3;
char SCSan::ID = 4;
char BDSan::ID = 5;
char MESan::ID = 6;

void enhance(Module& _module, std::string FSanitize) {
  auto pm = legacy::PassManager{};
  pm.add(new SigPass{});

  if (FSanitize.find("ibsan") != std::string::npos) pm.add(new IBSan{});
  if (FSanitize.find("bdsan") != std::string::npos) pm.add(new BDSan{});
  if (FSanitize.find("mesan") != std::string::npos) pm.add(new MESan{});
  if (FSanitize.find("resan") != std::string::npos) pm.add(new RESan{}); 
  if (FSanitize.find("tosan") != std::string::npos) pm.add(new TOSan{});
  if (FSanitize.find("scsan") != std::string::npos) pm.add(new SCSan{});
  pm.run(_module);

  for (auto& _bb : *_module.getFunction(CUDA_CONTRACT_FUNC)) 
    for (auto it = _bb.begin(); it != _bb.end(); ++it) {
      llvm::Instruction *Inst = &(*it);
      if (Inst->hasMetadata(c_rensan)) Inst->setMetadata(c_rensan, NULL);
      if (Inst->hasMetadata(c_msesan)) Inst->setMetadata(c_msesan, NULL);
      if (Inst->hasMetadata(c_elksan)) Inst->setMetadata(c_elksan, NULL);
    }

  return;
}

}  // namespace san