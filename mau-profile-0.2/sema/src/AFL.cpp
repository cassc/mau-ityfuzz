#include "AFL.h"
#include "config.h"

#include <unistd.h>

#include <array>
#include <vector>
#include <map>
#include <set>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/CFG.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

using namespace std;

namespace afl {

class AFLDeployerCov : public ModulePass {
  static char ID;
 public:
  AFLDeployerCov() : ModulePass(ID) {}
  virtual bool runOnModule(Module &M) override;
  using ModulePass::doFinalization;
  virtual bool doFinalization(Module &_module) override;
};

char AFLDeployerCov::ID = 1;

bool AFLDeployerCov::runOnModule(Module &M) {
  // errs() << "AFLDeployerCov is enabled\n";
  // LLVMContext &C = M.getContext();
  // IntegerType *Int8Ty = IntegerType::getInt8Ty(C);
  // IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  // llvm::GlobalVariable *CovMap = M.getNamedGlobal(COV_TB_REG);

  // llvm::Function *Func = M.getFunction(CUDA_DEPLOY_FUNC);

  // IRBuilder<> IRB(&(*Func->getEntryBlock().getFirstInsertionPt()));
  // // Value *AFLMapPtr = IRB.CreateGEP(CovMap, {IRB.getInt32(0), IRB.getInt32(0)});
  
  // llvm::Value *AFLPrevLoc = IRB.CreateAlloca(Int32Ty);
  // AFLPrevLoc->setName("__afl_prev_loc");
  // IRB.CreateStore(IRB.getInt32(0), AFLPrevLoc);
  
  /* Instrument all the things! */
  // for (auto &BB : *Func) {
  //   if (&BB == &Func->getEntryBlock() 
  //       || !BB.getTerminator()->hasMetadata(c_EVMBB)) 
  //     continue; 

  //   BasicBlock::iterator IP = BB.getFirstInsertionPt();
  //   IRBuilder<> IRB(&(*IP));

  //   /* Make up cur_loc */
  //   unsigned int cur_loc = (unsigned int)AFL_R(MAP_SIZE);
  //   ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);

  //   /* Load prev_loc */
  //   LoadInst *PrevLoc = IRB.CreateLoad(AFLPrevLoc);
  //   PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //   Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());
    
    // Value *MapPtrIdx = IRB.CreateGEP(AFLMapPtr, IRB.CreateXor(PrevLocCasted, CurLoc));
    // Value* CovPtr = IRB.CreateGEP(CovMap, {IRB.getInt32(0), IRB.CreateXor(PrevLocCasted, CurLoc)});
    /* Update covmap */
    // IRB.CreateStore(ConstantInt::get(Int8Ty, 1), CovPtr)
    //     ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

    /* Set prev_loc to cur_loc >> 1 */
    // StoreInst *Store =
    //     IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> 1), AFLPrevLoc);
    // Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  // }
  return true;
}

bool AFLDeployerCov::doFinalization(Module &) { return false; }

/* An LLVM PASS for AFL-style instrumentation*/
class AFLCoverage : public ModulePass {
  static char ID;
  
 public:
  AFLCoverage() : ModulePass(ID) {}
  virtual bool runOnModule(Module &M) override;
  virtual Function* fnHashWord(Module &M);
  using ModulePass::doFinalization;
  virtual bool doFinalization(Module &_module) override;
  protected:
    uint32_t start_cnt = 1;
    uint32_t delta = 1;
    double sigma = 1.0;
    uint32_t share_y;

    map<BasicBlock *, array<int, 2>> Params;
    map<BasicBlock *, uint32_t> Keys, SingleHash;
    map<BasicBlock *, vector<BasicBlock *>> Preds;
    vector<BasicBlock *> SingleBBS, MultiBBS, Solv, Unsolv;
    set<uint32_t> Hashes;
    map<array<uint32_t, 2>, uint32_t> HashMap;
    bool IsJoint(set<uint32_t> &A, set<uint32_t> &B)
    {
      for(auto &a : A)
        for(auto &b : B)
          if (a == b)
            return true;
      return false;
    }
    void CalcFmul()
    {
      set<uint32_t> tmpHashSet;
      uint32_t cur, edgeHash;
      bool isFind;
      for (int y = 1; y < MAP_SIZE_POW2; y++)
      {
        Hashes.clear();
        Params.clear();
        Solv.clear();
        Solv.clear();
        for(auto &BB : MultiBBS)
        // search parameters for BB
        {
          isFind = false;
          for (int x = 1; x < MAP_SIZE_POW2; x++)
          {
            for (int z = 1; z < MAP_SIZE_POW2; z++)
            {
              tmpHashSet.clear();
              cur = Keys[BB];
              // hashes for all incoming edges via Equation 2
              for(auto &p : Preds[BB])
              {
                edgeHash = ((cur >> x) ^ (Keys[p] >> y)) + z;
                // errs() << "edgeHash:\t" << edgeHash
                //        << " cur:\t" << cur
                //        << " x:\t" << x
                //        << " Keys[p]:\t" << Keys[p]
                //        << " z:\t" << z
                //        << "\n";
                tmpHashSet.insert(edgeHash);
              }
              // Found a solution for BB if no collision
              if ((tmpHashSet.size() == Preds[BB].size()) && !IsJoint(tmpHashSet, Hashes))
              {
                isFind = true;
                Params[BB] = array<int, 2>{x, z};
                share_y = y;
                Hashes.insert(tmpHashSet.begin(), tmpHashSet.end());
              }
              if (isFind)
                break;
            }
            if (isFind)
              break;
          }
          if (isFind)
            Solv.push_back(BB);
          else
            Unsolv.push_back(BB);
        }
        if (Unsolv.size() < delta || ((double)Unsolv.size() / (Unsolv.size() + Solv.size())) < sigma)
          break;
      }
    }
    void CalcFhash()
    {
      uint32_t cur;
      bool isSatisfy;
      HashMap.clear();
      for(auto &BB : Unsolv)
      {
        cur = Keys[BB];
        isSatisfy = false;
        for(auto &p : Preds[BB])
        {
          for (int i = 1; i < MAP_SIZE; i++)
            if (!Hashes.count(i))
            {
              HashMap[array<uint32_t, 2>{cur, Keys[p]}] = i;
              Hashes.insert(i);
              isSatisfy = true;
              break;
            }
          if (!isSatisfy)
          {
            errs() << "ERROR!!!\n";
            exit(1);
          }
        }
      }
    }
    void CalcFsingle()
    {
      bool isSatisfy;
      SingleHash.clear();
      for(auto &BB : SingleBBS)
      {
        isSatisfy = false;
        for (int i = 1; i < MAP_SIZE; i++)
        {
          if (!Hashes.count(i))
          {
            isSatisfy = true;
            Hashes.insert(i);
            SingleHash[BB] = i;
            break;
          }
        }
        if (!isSatisfy)
        {
          errs() << "ERROR!!!\n";
          exit(1);
        }
      }
    }
};

char AFLCoverage::ID = 0;

bool AFLCoverage::runOnModule(Module &M) {
  LLVMContext &C = M.getContext();
  IntegerType *Int8Ty = IntegerType::getInt8Ty(C);
  IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  /* Show a banner */
  char be_quiet = 0;
  if (isatty(2) && !getenv("AFL_QUIET")) {
    errs() << "afl-llvm-pass modified from <lszekeres@google.com>\n";
  } else  be_quiet = 1;

  /* Decide instrumentation ratio */
  char *inst_ratio_str = getenv("AFL_INST_RATIO");
  unsigned int inst_ratio = 100;

  if (inst_ratio_str) {
    if (sscanf(inst_ratio_str, "%u", &inst_ratio) != 1 || !inst_ratio ||
        inst_ratio > 100)
      errs() << "Bad value of AFL_INST_RATIO (must be between 1 and 100)";
  }

  /* Get variables for the SHM region and the previous location. Note that
     __afl_prev_loc is thread-local. */
  llvm::Function *Func_main = M.getFunction(CUDA_CONTRACT_FUNC);
  llvm::Argument *AFLMapPtr = &(*(Func_main->getArg(4)));
  // AFLMapPtr->setName("__afl_area_ptr");

  /* Get SLOAD and SSTORE instructions */
  std::vector<Instruction *> SLoadCalls;
  std::map<Instruction *, Value *> SStoreMap;
  for (auto &BB : *Func_main) {
    for (auto it = BB.begin(); it != BB.end(); ++it) {
      Instruction* I = &(*it);
      if (auto call = dyn_cast<CallInst>(I)) {
        StringRef name = call->getCalledFunction()->getName();
        if (name == "__device_sload")  SLoadCalls.push_back(I);
        else if (name == "__device_sstore") SStoreMap[I] = nullptr;
      }
    }
  }

  IRBuilder<> IRB(&(*Func_main->getEntryBlock().getFirstInsertionPt()));  
  llvm::Value *AFLPrevLoc = IRB.CreateAlloca(Int32Ty);
  AFLPrevLoc->setName("__afl_prev_loc");
  IRB.CreateStore(IRB.getInt32(0), AFLPrevLoc);
  
  for(auto& [_, R] : SStoreMap) {
    assert(R == nullptr && "[!] The AFL register has been initiated accidentily.");
    llvm::Value *State = IRB.CreateAlloca(Int32Ty);
    IRB.CreateStore(IRB.getInt32(0), State);
    R = State;
  }

  /* Instrument all the things! */
  unsigned int inst_blocks = 0;

  // AFL mode
  for (auto &BB : *Func_main) {
    if (&BB == &Func_main->getEntryBlock() ||
        !BB.getTerminator()->hasMetadata(c_EVMBB)) 
      continue; 
    // skip the long jumps and direct jumps
    // &BB == &Func_main->getEntryBlock() ||
    // if (BB.getName().startswith("JumpTableCase") 
    // || BB.getTerminator()->getNumSuccessors() == 1) continue;

    BasicBlock::iterator IP = BB.getFirstInsertionPt();
    IRBuilder<> IRB(&(*IP));

    // if (AFL_R(100) >= inst_ratio) continue;

    /* Make up cur_loc */

    unsigned int cur_loc = (unsigned int)AFL_R(MAP_SIZE);

    ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);

    /* Load prev_loc */
    LoadInst *PrevLoc = IRB.CreateLoad(AFLPrevLoc);
    PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
    Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());
    
    Value *Val = IRB.CreateXor(PrevLocCasted, CurLoc);;
    Val = IRB.CreateURem(Val, IRB.getInt32(MAP_SIZE));
    Value *MapPtrIdx = IRB.CreateGEP(AFLMapPtr, Val);

    /* Update bitmap */
    LoadInst *Counter = IRB.CreateLoad(MapPtrIdx);
    Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
    /* 
      // hitcounts
      Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
      IRB.CreateStore(Incr, MapPtrIdx)
      ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
    */
    // plain edge coverage
    IRB.CreateStore(ConstantInt::get(Int8Ty, 1), MapPtrIdx)
      ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

    /* Set prev_loc to cur_loc >> 1 */
    StoreInst *Store =
        IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> 1), AFLPrevLoc);
    Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

    inst_blocks++;
  }

  // CollAFL
  // uint32_t counter = SLoadCalls.size() + start_cnt;
  // for (auto &b : *Func_main) {
  //   BasicBlock *BB = &b;
  //   if (BB->hasNPredecessors(0) && ((&(BB->getParent()->getEntryBlock())) != BB))
  //     continue;
  //   if (BB->hasNPredecessors(1))
  //     SingleBBS.push_back(BB);
  //   else
  //     MultiBBS.push_back(BB);
  //   for (const auto &pred : predecessors(BB))
  //     Preds[BB].push_back(pred);
  //   Keys[BB] = counter++;

  //   // determine the kind of instruction
  //   uint32_t memory_access_operation_sum = 0;
  //   for (BasicBlock::iterator inst = BB->begin(); inst != BB->end(); inst++)
  //   {
  //     if (inst->getOpcode() == Instruction::Alloca ||
  //         inst->getOpcode() == Instruction::Load ||
  //         inst->getOpcode() == Instruction::Store ||
  //         inst->getOpcode() == Instruction::GetElementPtr ||
  //         inst->getOpcode() == Instruction::Fence ||
  //         inst->getOpcode() == Instruction::AtomicCmpXchg ||
  //         inst->getOpcode() == Instruction::AtomicRMW)
  //     {
  //       memory_access_operation_sum++;
  //       // errs() << *inst << "\n";
  //     }
  //   }
  // }
  // errs() << "SingleBBS size:\t" << SingleBBS.size() << "\n";
  // errs() << "MultiBBS size:\t" << MultiBBS.size() << "\n";
  // errs() << "Keys size:\t" << Keys.size() << "\n";
  // CalcFmul();
  // CalcFhash();
  // CalcFsingle();
  // errs() << "After cal\n";
  // errs() << "Solv size:\t" << Solv.size() << "\n";
  // errs() << "Unsolv size:\t" << Unsolv.size() << "\n";
  // errs() << "Hashes size:\t" << Hashes.size() << "\n";
  // errs() << "Params size:\t" << Params.size() << "\n";
  // errs() << "HashMap size:\t" << HashMap.size() << "\n";
  // errs() << "SingleHash size:\t" << SingleHash.size() << "\n";
  // errs() << "share y:\t" << share_y << "\n";

  // errs() << "Param:\n";
  // for(auto &sov:Solv)
  //   errs() <<Keys[sov] <<":\t"<<Params[sov][0]<<" "<<Params[sov][1]<< "\n";
  // for (auto &BB : *Func_main) {
  //   if (&BB == &Func_main->getEntryBlock()) 
  //     continue; 

  //   BasicBlock::iterator IP = BB.getFirstInsertionPt();
  //   IRBuilder<> IRB(&(*IP));

  //   /* Make up cur_loc */

  //   // uint32_t cur_loc = AFL_R(MAP_SIZE);
  //   BasicBlock *bb = &BB;
  //   uint32_t cur_loc = Keys[bb];

  //   ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);

  //   /* Load prev_loc */

  //   LoadInst *PrevLoc = IRB.CreateLoad(AFLPrevLoc);
  //   PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //   Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());

  //   /* Load SHM pointer */

  //   Value *MapPtrIdx;
  //   if(Params.count(bb)) {
  //     //InstrumentFmul
  //     array<int ,2> arr=Params[bb];
  //     CurLoc = ConstantInt::get(Int32Ty, cur_loc>>arr[0]);
  //     Value *h = IRB.CreateXor(PrevLocCasted, CurLoc);
  //     MapPtrIdx = IRB.CreateGEP(AFLMapPtr, IRB.CreateAdd(h, ConstantInt::get(Int32Ty, arr[1])));
  //   } else if (SingleHash.count(bb)) {
  //     //InstrumentFsingle 
  //     MapPtrIdx = IRB.CreateGEP(AFLMapPtr, ConstantInt::get(Int32Ty, SingleHash[bb]));
  //   } else {
  //     continue;
  //   }

  //   /* Update bitmap */

  //   LoadInst *Counter = IRB.CreateLoad(MapPtrIdx);
  //   Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //   Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
  //   IRB.CreateStore(Incr, MapPtrIdx)
  //       ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

  //   /* Set prev_loc to cur_loc >> share_y */
  //   StoreInst *Store =
  //       IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> share_y), AFLPrevLoc);
  //   Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

  //   inst_blocks++;
  // }

  // for(auto &bb : Unsolv)
  // {
  //   // if (bb == &Func_main->getEntryBlock()) 
  //   //   continue; 

  //   if (AFL_R(100) >= inst_ratio)
  //     continue;

  //   /* Make up cur_loc */

  //   uint32_t cur_loc = Keys[bb];
  //   for(auto &prevbb : Preds[bb])
  //   {
  //     uint32_t pre_loc = Keys[prevbb];
  //     if (HashMap.count(array<uint32_t, 2>{cur_loc, pre_loc}))
  //     {
  //       BasicBlock *dummyBB = SplitEdge(prevbb, bb);
  //       Keys[dummyBB] = counter++;
  //       HashMap[array<uint32_t,2>({Keys[dummyBB], pre_loc})]=HashMap[array<uint32_t,2>({cur_loc, pre_loc})];
  //       // for (int i = 1; i < MAP_SIZE; i++)
  //       // {
  //       //   if (!Hashes.count(i))
  //       //   {
  //       //     Hashes.insert(i);
  //       //     SingleHash[dummyBB] = i;
  //       //     break;
  //       //   }
  //       // }
  //       BasicBlock::iterator IP = dummyBB->getFirstInsertionPt();
  //       IRBuilder<> IRB(&(*IP));
  //       ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);

  //       /* Load prev_loc */

  //       LoadInst *PrevLoc = IRB.CreateLoad(AFLPrevLoc);
  //       PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //       Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());

  //       /* Load SHM pointer */

  //       // LoadInst *MapPtr = IRB.CreateLoad(AFLMapPtr);
  //       // MapPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //       Value *MapPtrIdx = IRB.CreateGEP(AFLMapPtr, ConstantInt::get(Int32Ty, HashMap[array<uint32_t,2>({cur_loc, pre_loc})]));

  //       /* Update bitmap */

  //       LoadInst *Counter = IRB.CreateLoad(MapPtrIdx);
  //       Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //       Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
  //       IRB.CreateStore(Incr, MapPtrIdx)
  //           ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

  //       /* Set prev_loc to cur_loc >> 1 */

  //       // StoreInst *Store =
  //       //     IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> 1), AFLPrevLoc);
  //       StoreInst *Store =
  //           IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> share_y), AFLPrevLoc);
  //       Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

  //       inst_blocks++;

  //       // LoadInst *MapMyPtr = IRB.CreateLoad(AFLMyMapPtr);
  //       // MapMyPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //       // Value *MapMyPtrIdx = IRB.CreateGEP(MapMyPtr, ConstantInt::get(Int32Ty, Keys[dummyBB]));
  //       // IRB.CreateStore(ConstantInt::get(Int8Ty, 1), MapMyPtrIdx)
  //       //     ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  //     }
  //   }
  // }

  /* state instrumention */
  for(auto const& [IP, State] : SStoreMap) {
    // insert code after sload()
    IRB.SetInsertPoint(IP->getNextNonDebugInstruction());     
    Value *SKeyPtr = IP->getOperand(0);
    Value *SKeyHash = IRB.CreateCall(fnHashWord(M), {SKeyPtr});
    // __state = key
    StoreInst *Store = IRB.CreateStore(SKeyHash, State);
    Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
  }

  uint32_t state_counter = 0;
  for (auto const& IP : SLoadCalls) {
    // insert code after sload()
    IRB.SetInsertPoint(IP->getNextNonDebugInstruction()); 
    Value *SKeyPtr = IP->getOperand(0);
    Value *SKeyHash = IRB.CreateCall(fnHashWord(M), {SKeyPtr});

    // ri = hashkey eq state
    // _state = r1 | r2 | r3 ... | rn
    Value *State = IRB.getFalse();
    for(auto const& [_, r] : SStoreMap) {
      Value *h = IRB.CreateLoad(r);
      Value *t = IRB.CreateICmpEQ(SKeyHash, h);
      State = IRB.CreateOr(State, t);
    }

    Value *MapPtrIdx = IRB.CreateGEP(AFLMapPtr, IRB.getInt32((unsigned int)state_counter));
    StoreInst *Store = IRB.CreateStore(IRB.CreateZExt(State, IRB.getInt8Ty()), MapPtrIdx);
    Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
    state_counter++;
  }

  /* Say something nice. */
  if (!be_quiet) {
    if (!inst_blocks)
      errs() << "No instrumentation targets found.";
    else
      errs() << "Instrumented " << inst_blocks << " locations ("
             << (getenv("AFL_HARDEN") ? "hardened"
          : ((getenv("AFL_USE_ASAN") || getenv("AFL_USE_MSAN"))
                 ? "ASAN/MSAN" : "non-hardened"))
                << " mode, ratio " << inst_ratio << "%). States cover "
                << SLoadCalls.size() << " SLOAD insts.\n";
  }
  return true;
}

Function* AFLCoverage::fnHashWord(Module &M) {
  static const auto funcName = "__hashword";
  llvm::Function *func = M.getFunction(funcName);
  if (func != nullptr) return func;

  LLVMContext &C = M.getContext();
  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::getInt32Ty(C), { Type::getIntNPtrTy(C, 256) }, false),
      llvm::Function::ExternalLinkage, funcName, M);
  return func;
}

bool AFLCoverage::doFinalization(Module &) { return false; }

bool instrument(Module &_module) {
  auto pm = legacy::PassManager{};

  pm.add(new AFLCoverage{});
  return pm.run(_module);
}

bool instrumentDeployer(Module &_module) {
  auto pm = legacy::PassManager{};

  pm.add(new AFLDeployerCov{});
  return pm.run(_module);
}

}  // namespace afl