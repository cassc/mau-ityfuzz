#include "WASMCompiler.h"

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#if __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
#define FALLTHROUGH [[clang::fallthrough]]
#elif __has_cpp_attribute(gnu::fallthrough)
#define FALLTHROUGH [[gnu::fallthrough]]
#else
#define FALLTHROUGH
#endif

namespace dev {
namespace eth {
namespace trans {


uint32_t splitCode(const std::string& Bytecode) {
  bool existCodeCopy = false;
  bool existReturn = false;

  code_iterator begin = (uint8_t *)Bytecode.c_str();  // begin of current block
  code_iterator _end = begin + Bytecode.size();
  // std::cerr << "------------------------------\n";
  for (code_iterator curr = begin, next = begin; curr != _end; curr = next) {
    next = skipPushDataAndGetNext(curr, _end);
    // std::cerr << std::hex << (int)(*curr) << " ";
    switch (Instruction(*curr)) {
      case Instruction::CODECOPY:
        existCodeCopy = true;
        // std::cerr << "CodeCopy\n";
        break;

      case Instruction::RETURN:
        if (existCodeCopy) {
          existReturn = true;
          // std::cerr << "RETURN\n";
          // RETURNPC = curr - begin;
        }
        break;

      case Instruction::PUSH1:
        if (existReturn) {
          return (uint32_t)(curr - begin);  // success split the runtime code=code[curr:];
                                // deploy code=code[:curr]
        }
        break;
      default:
        break;
        // do nothing
    }
  }
  return 0;
}

void makeGlobalVar(llvm::Module& Module) {
  llvm::LLVMContext &Ctx = Module.getContext();  

  // runtime storage vector for all threads
  // llvm::Type *SnapLenTy = llvm::ArrayType::get(Type::Int8Ty, NJOBS);
  // llvm::GlobalVariable *L2SnapLenPtr = new llvm::GlobalVariable(
  //     Module, SnapLenTy, false, llvm::GlobalVariable::ExternalLinkage,
  //     nullptr, L2SNAP_LENS, nullptr,
  //     llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);
  // L2SnapLenPtr->setAlignment(1);

  // llvm::Type *SnapTy = llvm::ArrayType::get(Type::StorageTy, NJOBS);
  // llvm::GlobalVariable *L2SnapPtr = new llvm::GlobalVariable(
  //     Module, SnapTy, false, llvm::GlobalVariable::ExternalLinkage,
  //     nullptr, L2SNAPS, nullptr, 
  //     llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);
  // L2SnapPtr->setAlignment(32);

  // root storage for deployer only
  // llvm::GlobalVariable *RootSnapLenPtr = new llvm::GlobalVariable(
  //     Module, Type::Int8Ty, false, llvm::GlobalVariable::ExternalLinkage,
  //     nullptr, L3SNAP_LEN, nullptr, llvm::GlobalValue::NotThreadLocal,
  //     AS::GLOBAL, false);
  // RootSnapLenPtr->setAlignment(1);
 
  // llvm::GlobalVariable *RootSnapPtr = new llvm::GlobalVariable(
  //     Module, Type::StorageTy, false, llvm::GlobalVariable::ExternalLinkage,
  //     nullptr, L3SNAP, nullptr, llvm::GlobalValue::NotThreadLocal, 
  //     AS::GLOBAL, false);
  // RootSnapPtr->setAlignment(32);

  /* 
   * declear a pointer array representing all bitmaps, i.e. uint8_t *__bitmaps[NJOBS]; . 
   * In CPU end, we allocate a MAP_SIZE array to the address of __bitmaps[i]
   ,representing the bitmap space of the thread i.
  */

  // llvm::GlobalVariable *SignalsPtr = new llvm::GlobalVariable(
  //     Module, llvm::ArrayType::get(Type::Int64Ty, SIGNAL_VEC_LEN), false, 
  //     llvm::GlobalVariable::ExternalLinkage, nullptr, SIGNALS_REG, nullptr,
  //     llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);
  // SignalsPtr->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // SignalsPtr->setAlignment(8);

  // declare this.address, a 160 bits constant
  llvm::GlobalVariable *SelfAddressVar = new llvm::GlobalVariable(
      Module, Type::AddressTy, false, llvm::GlobalVariable::PrivateLinkage,
      llvm::ConstantInt::get(Type::AddressTy, 0), SELFADDR_REF, nullptr,
      llvm::GlobalValue::NotThreadLocal, AS::CONST, false);  // constant
  SelfAddressVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  SelfAddressVar->setAlignment(llvm::MaybeAlign(32));

  // declare caller, a 160 bits constant
  // llvm::GlobalVariable *CallerVar = new llvm::GlobalVariable(
  //     Module, Type::AddressTy, false, llvm::GlobalVariable::PrivateLinkage,
  //     llvm::ConstantInt::get(Type::AddressTy, 0), CALLER_REF, nullptr,
  //     llvm::GlobalValue::NotThreadLocal, AS::CONST, false);  // constant
  // CallerVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // CallerVar->setAlignment(32);

  // declare tx.origin, a 160 bits constant
  llvm::GlobalVariable *OriginVar = new llvm::GlobalVariable(
      Module, Type::AddressTy, false, llvm::GlobalVariable::PrivateLinkage,
      llvm::ConstantInt::get(Type::AddressTy, 0), ORIGIN_REF, nullptr,
      llvm::GlobalValue::NotThreadLocal, AS::CONST, false);  // constant
  OriginVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  OriginVar->setAlignment(llvm::MaybeAlign(32));

  // tx.timestamp
  llvm::GlobalVariable *TimestampVar = new llvm::GlobalVariable(
    Module, Type::Int256Ty, false, llvm::GlobalVariable::PrivateLinkage,
    llvm::ConstantInt::get(Type::Int256Ty, 0), TIMESTAMP_REF, nullptr,
    llvm::GlobalValue::NotThreadLocal, AS::CONST, false);  // constant
  TimestampVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // TimestampVar->setAlignment(32);

  // tx.number
  llvm::GlobalVariable *BlockNumVar = new llvm::GlobalVariable(
    Module, Type::Int256Ty, false, llvm::GlobalVariable::PrivateLinkage,
    llvm::ConstantInt::get(Type::Int256Ty, 0), BLOCKNUM_REF, nullptr,
    llvm::GlobalValue::NotThreadLocal, AS::CONST, false);  // constant
  BlockNumVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // BlockNumVar->setAlignment(32); 

  llvm::GlobalVariable *BalanceCallerVar = new llvm::GlobalVariable(
    Module, Type::Int256Ty, false, llvm::GlobalVariable::PrivateLinkage,
    Constant::get(getIntialBalance()), BALANCECALLER_REF, nullptr,
    llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);  // 10 ether
  BalanceCallerVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // BalanceCallerVar->setAlignment(32);

  // msg.value
  // llvm::GlobalVariable *ValueVar = new llvm::GlobalVariable(
  //   Module, Type::Int256Ty, false, llvm::GlobalVariable::PrivateLinkage,
  //   llvm::ConstantInt::get(Type::Int256Ty, 0), MSGVALUE_REF, nullptr,
  //   llvm::GlobalValue::NotThreadLocal, AS::CONST, false);  // constant
  // ValueVar->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // ValueVar->setAlignment(32); 

  // llvm::GlobalVariable *CovMap = new llvm::GlobalVariable(
  //     Module, llvm::ArrayType::get(Type::Int8Ty, MAP_SIZE), false, 
  //     llvm::GlobalVariable::ExternalLinkage,  nullptr, COV_TB_REG, nullptr,
  //     llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);
  // CovMap->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  // CovMap->setAlignment(1);
}


void makeGlobalBytecode(llvm::Module& Module, const std::string& Bytecode) {
  llvm::LLVMContext &Ctx = Module.getContext();  
  std::string EvmCode(Bytecode);
  EvmCode.resize(EVMCODE_SIZE, 0);

  llvm::Constant *BytecodeConst = llvm::ConstantDataArray::getString(Ctx, 
      llvm::StringRef(EvmCode));
  auto *GBytecode = new llvm::GlobalVariable(Module, BytecodeConst->getType(), false,
      llvm::GlobalValue::ExternalLinkage, BytecodeConst, EVMCODE_REG, nullptr, 
      llvm::GlobalVariable::NotThreadLocal, AS::CONST);
  GBytecode->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
  // GBytecode->setAlignment(1);

  llvm::GlobalVariable *CodeSizePtr = new llvm::GlobalVariable(
      Module, Type::Int32Ty, false, llvm::GlobalVariable::ExternalLinkage,
      llvm::ConstantInt::get(Type::Int32Ty, Bytecode.size()), EVMCODESIZE_REG, 
      nullptr, llvm::GlobalValue::NotThreadLocal,
      AS::CONST, false);
  CodeSizePtr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
  // CodeSizePtr->setAlignment(4);
}


WASMCompiler::WASMCompiler(const Options& _options, llvm::Module& _module, 
    EEIModule& _utilsM): m_options(_options), m_module(_module), 
      m_builder(_module.getContext()), m_utilsM(_utilsM) {}

std::vector<BasicBlock> WASMCompiler::createBasicBlocks(
    code_iterator _codeBegin, code_iterator _codeEnd) {
  /* 
    Helper function that skips push data and 
    finds next iterator (can be the end)
  */
  auto skipPushDataAndGetNext = [](code_iterator _curr, code_iterator _end) {
    static const auto push1 = static_cast<size_t>(Instruction::PUSH1);
    static const auto push32 = static_cast<size_t>(Instruction::PUSH32);
    size_t offset = 1;
    if (*_curr >= push1 && *_curr <= push32)
      offset += std::min<size_t>(*_curr - push1 + 1, (_end - _curr) - 1);
    return _curr + offset;
  };

  uint32_t edgeCnt = 0;

  std::vector<BasicBlock> blocks;

  bool isDead = false;
  auto begin = _codeBegin;  // begin of current block

  for (auto curr = begin, next = begin; curr != _codeEnd; curr = next) {
    next = skipPushDataAndGetNext(curr, _codeEnd);

    if (isDead) {
      if (Instruction(*curr) == Instruction::JUMPDEST) {
        isDead = false;
        begin = curr;
      } else
        continue;
    }
    // measure
    if (Instruction(*curr) == Instruction::JUMP)  edgeCnt += 1;
    else if (Instruction(*curr) == Instruction::JUMPI) edgeCnt += 2;

    bool isEnd = false;
    switch (Instruction(*curr)) {
      case Instruction::JUMP:
      case Instruction::RETURN:
      case Instruction::REVERT:
      case Instruction::STOP:
      // case Instruction::INVALID:
      case Instruction::SUICIDE:
        isDead = true;
        FALLTHROUGH;
      case Instruction::JUMPI:
        isEnd = true;
        break;

      default:
        break;
    }

    assert(next <= _codeEnd);
    if (next == _codeEnd || Instruction(*next) == Instruction::JUMPDEST)
      isEnd = true;

    if (isEnd) {
      auto beginIdx = begin - _codeBegin;
      blocks.emplace_back(beginIdx, begin, next, m_mainFunc);
      begin = next;
    }
  }
  std::cerr << "At least: " << edgeCnt << " edges in bytecode.\n";
  return blocks;
}

void WASMCompiler::resolveGas(llvm::Function* Func) {  
  assert(m_abortBB != nullptr && "Create the abortBB first.");
  llvm::IRBuilder<> IRB(&(*Func->getEntryBlock().getFirstInsertionPt()));
  auto GasLeftPtr = IRB.CreateAlloca(Type::Int64Ty, nullptr, "remaing_gas");
  IRB.CreateStore(IRB.getInt64(GASLIMIT_MAX), GasLeftPtr);

  std::vector<llvm::Instruction*> IPs;
  for (auto &BB : *Func) {
    if (&BB == &Func->getEntryBlock() || !BB.getTerminator()->hasMetadata(c_EVMBB))
      continue; 
    llvm::BasicBlock::iterator IP = BB.getFirstInsertionPt();
    IPs.push_back(&(*IP));
  }

  for (llvm::Instruction *IP : IPs) {  
    IRB.SetInsertPoint(&(*IP));
    auto GasLeftBegin = IRB.CreateLoad(GasLeftPtr);
    auto UsedGas = IRB.getInt64(8 * IP->getParent()->getInstList().size());
    auto InSufficient = IRB.CreateICmpUGT(UsedGas, GasLeftBegin);
    auto GasLeftAfter = IRB.CreateSub(GasLeftBegin, UsedGas);
    IRB.CreateStore(GasLeftAfter, GasLeftPtr);

    SplitBlockAndInsertIfThen(InSufficient, llvm::cast<llvm::Instruction>(GasLeftAfter), 
                              false, nullptr, (llvm::DomTreeUpdater *)nullptr, nullptr, m_abortBB);
  }
}


void WASMCompiler::resolveJumps(std::vector<BasicBlock> &blocks) {
  std::map<uint64_t, BasicBlock *> idxMap;
  for (size_t i = 0; i < blocks.size(); i++) {
    idxMap[blocks[i].firstInstrIdx()] = &blocks[i];
  }

  auto jumpTable = llvm::cast<llvm::SwitchInst>(m_jumpTableBB->getTerminator());

  for (auto it = std::next(m_mainFunc->begin()),
            end = std::prev(m_mainFunc->end(), 3);
       it != end; ++it) {
    auto nextBlockIter = it;
    ++nextBlockIter;  // If the last code block, that will be "stop" block.
    auto currentBlockPtr = &(*it);
    auto nextBlockPtr = &(*nextBlockIter);

    llvm::Instruction *term = it->getTerminator();
    llvm::BranchInst *jump = nullptr;

    if (!term) {
      // Block may have no terminator if the next instruction is a JUMPDEST
      IRBuilder{currentBlockPtr}.CreateBr(nextBlockPtr);
    } 
    else if ((jump = llvm::dyn_cast<llvm::BranchInst>(term)) &&
              jump->getSuccessor(0) == m_jumpTableBB) {
      auto destIdx = llvm::cast<llvm::ValueAsMetadata>(
          jump->getMetadata(c_destIdxLabel)->getOperand(0))->getValue();
      if (auto constant = llvm::dyn_cast<llvm::ConstantInt>(destIdx)) {
        int64_t concrete = constant->getSExtValue();
        // If destination is a constant, we use a direct jump. 
        // std::cerr << "[resolveJumps] Static Jump target. at " << concrete << "\n";
        auto constant64 = llvm::ConstantInt::get(Type::Int64Ty, concrete);
        auto bb = jumpTable->findCaseValue(constant64)->getCaseSuccessor();
        jump->setSuccessor(0, bb);
        // std::cerr << bb->getName().str() << "\n";
    
        llvm::Instruction *prev_ins = jump;
        while (prev_ins) {
          prev_ins = prev_ins->getPrevNonDebugInstruction();
          if (auto store = llvm::dyn_cast<llvm::StoreInst>(prev_ins)) {
            if (store->getOperand(1)->getName() == JMPTARGET_PTR_REG) {
              store->eraseFromParent();
              break;
            }
          }
        }
      }
      jump->setMetadata(c_destIdxLabel, NULL);
      // Set next block for conditional jumps
      if (jump->isConditional())
        jump->setSuccessor(1, &(*nextBlockIter));
    }
  }

  // replace the switch jumps to inline jumps
  // m_builder.SetInsertPoint(m_jumpTableBB->getTerminator());
  // llvm::Value *load_target_ins = m_builder.CreateTrunc(
  //     m_jumpTableBB->getFirstNonPHIOrDbg(), Type::Int32Ty);

  // std::vector<llvm::BasicBlock *> inline_bbs;
  // for (auto Case : jumpTable->cases()) {
  //   llvm::BasicBlock *Dst = Case.getCaseSuccessor();
  //   llvm::ConstantInt *C = Case.getCaseValue();
  //   llvm::BasicBlock *new_BB = llvm::BasicBlock::Create(
  //       m_builder.getContext(), "JumpTableCase", m_mainFunc);
  //   inline_bbs.push_back(new_BB);

  //   m_builder.SetInsertPoint(new_BB);
  //   llvm::Value *cond = m_builder.CreateICmpEQ(
  //       llvm::ConstantInt::get(Type::Int32Ty, C->getSExtValue()),
  //       load_target_ins);
  //   m_builder.CreateCondBr(cond, Dst, nullptr);
  // }
  // inline_bbs.push_back(jumpTable->getDefaultDest());
  // // llvm::Value *cond = m_builder.CreateICmpEQ(C, load_target_ins);
  // // m_builder.CreateCondBr()

  // m_builder.SetInsertPoint(m_jumpTableBB->getTerminator());
  // m_builder.CreateBr(inline_bbs[0]);
  // jumpTable->eraseFromParent();

  // for (uint idx = 0; idx < inline_bbs.size() - 1; ++idx) {
  //   auto jump =
  //       llvm::dyn_cast<llvm::BranchInst>(inline_bbs[idx]->getTerminator());
  //   jump->setSuccessor(1, inline_bbs[idx + 1]);
  // }
}

/// @brief wrap the function contract() to execute multiply txs
/// @param _txs_len  the lenght of transactions
/// @return LLVM module
llvm::Function* wrapRuntimer(llvm::Module& Module, int64_t _txs_len) {
  /* 
   create the below kernel funtion 
  */
  auto *FuncTy = llvm::FunctionType::get(
      Type::Void, {Type::Int8Ptr1Ty,  /* dataVec */
                   Type::Int32Ty,     /* calldatasize */
                   },   
                   false);
  llvm::Function *kernelFunc = llvm::Function::Create(
      FuncTy, llvm::Function::ExternalLinkage, CUDA_KERNEL_FUNC, Module);
  
  llvm::LLVMContext &Context = Module.getContext();  
  // set metadata for nvvc
  llvm::NamedMDNode *annotation =
      Module.getOrInsertNamedMetadata("nvvm.annotations");
  auto _content = llvm::MDTuple::get(
      Context, 
      {
        llvm::ValueAsMetadata::get(kernelFunc),
        llvm::MDString::get(Context, "kernel"),
        llvm::ValueAsMetadata::getConstant(
          llvm::ConstantInt::get(Type::Int32Ty, 1))
      }
  );
  annotation->addOperand(_content);

  // /* Global Variables */  
  // llvm::GlobalVariable *L2SnapLenPtr = Module.getNamedGlobal(L2SNAP_LENS);
  // llvm::GlobalVariable *L2SnapPtr = Module.getNamedGlobal(L2SNAPS);

  // llvm::BasicBlock *BB = llvm::BasicBlock::Create(Context, "entry", kernelFunc);
  // llvm::IRBuilder<> IRB(BB);

  // llvm::ConstantInt *Zero32 = IRB.getInt32(0);
  // llvm::ConstantInt *TcSize32 = IRB.getInt32(TRANSACTION_SIZE);

  // /* Thread ID for one running thread, denoted as `i`*/
  // auto FnCuThread = Module.getFunction(CUTHREAD_FUNC);
  // llvm::Value *LThreadId = IRB.CreateCall(FnCuThread, {});
  // llvm::Argument *ThreadOffset = kernelFunc->getArg(3);
  // llvm::Value *GThreadId = IRB.CreateAdd(ThreadOffset, LThreadId);

  // auto BitmapPtr = IRB.CreateLoad(Type::Byte->getPointerTo(AS::GLOBAL),
  //     IRB.CreateGEP(Module.getNamedGlobal(BITMAPS_REG), 
  //     {Zero32, GThreadId}));

  // /* snapshot for the thread N_i <--- N[i] */
  // auto SnapLenPtr = IRB.CreateGEP(L2SnapLenPtr, {Zero32, GThreadId});
  // auto SnapPtr = IRB.CreateGEP(L2SnapPtr, {Zero32, GThreadId, Zero32});

  // /* FIXME: set the signal if crashed or the seed is interesting */
  // llvm::Value *SigPtr = BitmapPtr;

  // /* calldata for the thread */
  // // llvm::Value *TcPtr = IRB.CreateGEP(kernelFunc->getArg(0), 
  // //     {IRB.CreateMul(LThreadId, TcSize32)});
  // llvm::Value *TcPtr = IRB.CreateGEP(kernelFunc->getArg(0), {Zero32});

  // llvm::Value *CallvaluePtr = IRB.CreateBitCast(
  //     IRB.CreateGEP(TcPtr, {IRB.getInt32(0)}), 
  //     Type::Int64Ty->getPointerTo(AS::GLOBAL));
  // llvm::Value *Callvalue = IRB.CreateLoad(CallvaluePtr);

  // llvm::Value *CalldataSizePtr = IRB.CreateBitCast(
  //     IRB.CreateGEP(TcPtr, {IRB.getInt32(8)}), 
  //     Type::Int32Ty->getPointerTo(AS::GLOBAL));
  // llvm::Value *CalldataSize = IRB.CreateLoad(CalldataSizePtr);

  // // auto CalldataPtr = IRB.CreateCall(Module.getFunction("malloc"), 
  // //     {IRB.getInt64(TRANSACTION_SIZE)}, "calldata");
  // auto CalldataType = llvm::ArrayType::get(Type::Int8Ty, TRANSACTION_SIZE);
  // auto CalldataArray = IRB.CreateAlloca(CalldataType);
  // auto CalldataPtr = IRB.CreateGEP(CalldataArray, {Zero32, Zero32}, "calldata");
  // // llvm::Value *CalldataPtr = IRB.CreateBitCast(
  //     // IRB.CreateGEP(TcPtr, {IRB.getInt32(12)}), Type::Int8Ty->getPointerTo(AS::GLOBAL));

  // IRB.CreateMemCpy(CalldataPtr, llvm::MaybeAlign(1), 
  //     IRB.CreateGEP(TcPtr, {IRB.getInt32(12)}), llvm::MaybeAlign(1), 
  //     IRB.getInt32(TRANSACTION_SIZE - 12));  
 
  // // mutate the calldata except the first SIMD thread
  // IRB.CreateCall(Module.getFunction(FN_CALLDATAMUTATOR), {CalldataPtr, GThreadId});
  // llvm::Function *ContractFn = Module.getFunction(CUDA_CONTRACT_FUNC);
  // IRB.CreateCall(ContractFn, {SnapPtr, SnapLenPtr, Callvalue, CalldataSize,
  //                CalldataPtr, BitmapPtr, SigPtr, IRB.getFalse()});
  // // IRB.CreateCall(Module.getFunction("free"), {CalldataPtr});

  // IRB.CreateRetVoid();
  /* end of @contract()*/

  return kernelFunc;
}

llvm::Function* wrapDeployer(llvm::Module& Module) {
  auto *FuncTy = llvm::FunctionType::get(Type::Void, 
      {/*transaction*/Type::Int8Ptr1Ty, /*debug*/Type::Int8Ptr1Ty},
      false);
  llvm::Function *kernelFunc = llvm::Function::Create(
      FuncTy, llvm::Function::ExternalLinkage, CUDA_KERNEL_DEPLOYER_FUNC, Module);
  
  // set metadata for nvvc
  llvm::NamedMDNode *annotation =
    Module.getOrInsertNamedMetadata("nvvm.annotations");
  llvm::LLVMContext &Context = Module.getContext();
  auto _content = llvm::MDTuple::get(Context, {
      llvm::ValueAsMetadata::get(kernelFunc),
      llvm::MDString::get(Context, "kernel"),
      llvm::ValueAsMetadata::getConstant(
        llvm::ConstantInt::get(Type::Int32Ty, 1))});
  annotation->addOperand(_content);

  // /* Global Variables */  
  // llvm::GlobalVariable *L2SnapLenPtr = Module.getNamedGlobal(L2SNAP_LENS);
  // llvm::GlobalVariable *L2SnapPtr = Module.getNamedGlobal(L2SNAPS);
  
  // llvm::BasicBlock *BB = llvm::BasicBlock::Create(Context, "entry", kernelFunc);
  // llvm::IRBuilder<> IRB(BB);
  // llvm::ConstantInt *Zero32 = IRB.getInt32(0);

  // auto BitmapPtr = IRB.CreateLoad(Type::Int8Ty->getPointerTo(AS::GLOBAL),
  //     IRB.CreateGEP(Module.getNamedGlobal(BITMAPS_REG), {Zero32, Zero32}));

  // llvm::Value *SigPtr = BitmapPtr;

  // llvm::Value *TcPtr = IRB.CreateGEP(kernelFunc->getArg(0), {Zero32});
  // llvm::Value *CallvaluePtr = IRB.CreateBitCast(
  //     IRB.CreateGEP(TcPtr, {IRB.getInt32(0)}), 
  //     Type::Int64Ty->getPointerTo(AS::GLOBAL));
  // llvm::Value *Callvalue = IRB.CreateLoad(CallvaluePtr);  
  // llvm::Value *CalldataSizePtr = IRB.CreateBitCast(
  //     IRB.CreateGEP(TcPtr, {IRB.getInt32(8)}), 
  //     Type::Int32Ty->getPointerTo(AS::GLOBAL));
  // llvm::Value *CalldataSize = IRB.CreateLoad(CalldataSizePtr);
  // // llvm::Value *CalldataPtr = IRB.CreateBitCast(
  // //     IRB.CreateGEP(TcPtr, {IRB.getInt32(12)}),
  // //     Type::Int8Ty->getPointerTo(AS::GLOBAL));
  // auto CalldataType = llvm::ArrayType::get(Type::Int8Ty, TRANSACTION_SIZE);
  // auto CalldataArray = IRB.CreateAlloca(CalldataType);
  // auto CalldataPtr = IRB.CreateGEP(CalldataArray, {Zero32, Zero32}, "calldata");
  // IRB.CreateMemCpy(CalldataPtr, llvm::MaybeAlign(1), 
  //     IRB.CreateGEP(TcPtr, {IRB.getInt32(12)}), llvm::MaybeAlign(1), 
  //     IRB.getInt32(TRANSACTION_SIZE)); 

  // // L2SnapLen[0] = 0
  // // L1SnapLen[0] = 0
  // auto SnapPtr = IRB.CreateGEP(L2SnapPtr, {Zero32, Zero32, Zero32});
  // auto SnapLenPtr = IRB.CreateGEP(L2SnapLenPtr , {Zero32, Zero32});
  // IRB.CreateStore(IRB.getInt8(0), SnapLenPtr);
  // auto RootSnapLenPtr = Module.getNamedGlobal(L3SNAP_LEN);
  // IRB.CreateStore(IRB.getInt8(0), RootSnapLenPtr);
  
  // // execute the deployer 
  // llvm::Function *DeployerFn = Module.getFunction(CUDA_DEPLOY_FUNC);
  // IRB.CreateCall(DeployerFn, {SnapPtr, SnapLenPtr, Callvalue, CalldataSize,
  //                CalldataPtr, BitmapPtr, SigPtr, IRB.getFalse()});
  
  // // move the storage content to the root one
  // auto SlotLen = IRB.CreateLoad(SnapLenPtr);
  // IRB.CreateStore(SlotLen, RootSnapLenPtr);
  // IRB.CreateStore(IRB.getInt8(0), SnapLenPtr);
  
  // auto RootSnapPtr = IRB.CreateGEP(Module.getNamedGlobal(L3SNAP), {Zero32, Zero32});
  // auto Bytes = IRB.CreateMul(IRB.CreateZExt(SlotLen, Type::Int32Ty), IRB.getInt32(64));
  // IRB.CreateMemCpy(RootSnapPtr, llvm::MaybeAlign(32), SnapPtr, llvm::MaybeAlign(32), Bytes);
  
  // IRB.CreateRetVoid();
  // /* end of @deployer */

  return kernelFunc;
}

/// @brief lift the given EVM bytecode to LLVM IR in a device function named
/// contrat()
/// @param _begin the begin of the EVM bytecode
/// @param _end   the end of the EVM bytecode
/// @param _id    contact name
/// @return  an LLVM module
llvm::Module* WASMCompiler::compileMain(const uint32_t EvmOffset, 
    const std::string& Bytecode, const std::string& EntryName) {
  this->setBytecodeOffset(EvmOffset);

  // create the entry function
  llvm::LLVMContext &context = m_module.getContext();
  auto *FuncTy = llvm::FunctionType::get(Type::Int32Ty, {
        Type::Int256PtrTy,                      /* caller */
        Type::Int256PtrTy,                      /* callvalue */
        Type::Int8PtrTy,                        /* calldata pointer */
        Type::Int32Ty,                          /* calldata size */
        Type::Int8Ptr1Ty,                       /* shm */
      }, false);

  m_mainFunc = llvm::Function::Create(FuncTy, llvm::Function::ExternalLinkage,
                                      EntryName, m_module);
  llvm::BasicBlock *mainBB = llvm::BasicBlock::Create(context, "Entry", m_mainFunc);
  // create llvm basic blocks from EVM bytecode
  std::vector<BasicBlock> blocks = createBasicBlocks(
      (uint8_t *)Bytecode.c_str(), (uint8_t *)Bytecode.c_str() + Bytecode.length());
  m_jumpTableBB = llvm::BasicBlock::Create(context, "JumpTable", m_mainFunc);
  m_abortBB = llvm::BasicBlock::Create(context, "Abort", m_mainFunc);
  m_exitBB = llvm::BasicBlock::Create(context, "Exit", m_mainFunc);

  // Build entry block
  m_builder.SetInsertPoint(mainBB);
  // @memory
  m_mem_ptr = m_builder.CreateCall(m_utilsM.fn_malloc(), 
      {m_builder.getInt64(EVM_MEM_SIZE)}, MEM_REF_REG);
  // @stack
  auto _stk_i8_ref = m_builder.CreateCall(m_utilsM.fn_malloc(), 
      {m_builder.getInt64(EVM_STK_SIZE)});
  m_stk_ref = m_builder.CreateBitCast(_stk_i8_ref, Type::WordPtr, STK_REF_REG);
  // declare @stkPtr
  m_stk_dep_ptr = m_builder.CreateAlloca(Type::Int64Ty, nullptr, STK_DEP_PTR_REG);
  m_builder.CreateStore(m_builder.getInt64(0), m_stk_dep_ptr);
  // declare @jmp_target_ptr
  m_targetPtr = m_builder.CreateAlloca(Type::Int64Ty, nullptr, JMPTARGET_PTR_REG);
  m_builder.CreateStore(m_builder.getInt64(0), m_targetPtr);
  m_builder.CreateBr(blocks[0].llvm());

  // Build jump table
  // Must be before basic blocks compilation
  m_builder.SetInsertPoint(m_jumpTableBB);  
  m_builder.CreateSwitch(m_builder.CreateLoad(Type::Int64Ty, m_targetPtr), m_abortBB);

  dry_run(blocks);

  // lift the bytecode to IR
  GlobalStack _stack(&m_builder, m_stk_ref, m_stk_dep_ptr);
  m_stack = &_stack;

  std::set<BasicBlock *> visited;
  std::queue<std::pair<BasicBlock *, std::vector<llvm::Value *> *>>
      waitingBlocks;
  std::vector<llvm::Value *> *root = new std::vector<llvm::Value *>;
  waitingBlocks.push(std::make_pair(&blocks[0], root));

  while (!waitingBlocks.empty()) {
    // std::cerr << "going to analyze one block\n";
    BasicBlock *pbb = waitingBlocks.front().first;
    std::vector<llvm::Value *> *pstack = waitingBlocks.front().second;
    waitingBlocks.pop();

    // jump to an invalid block, such as INVALID
    if (pbb == nullptr) {
      continue;
    }

    if (visited.find(pbb) != visited.end()) {
      delete pstack;
      continue;
    } else
      visited.insert(pbb);

    std::vector<llvm::Value *> depStk;
    m_builder.SetInsertPoint(pbb->llvm());
    if ((Instruction(*pbb->begin()) != Instruction::JUMPDEST ||
         (pbb->bbs_from.size() == 1 &&
          Instruction(*pbb->bbs_from[0]->begin()) == Instruction::JUMPI)) &&
        pstack->size() > 0) {
      // Starting with a non-JUMPDEST
      // fallout block
      for (auto itr = pstack->rbegin();
           itr != pstack->rend() && depStk.size() < pbb->get_stk_in(); ++itr) {
        depStk.push_back(*itr);
      }
      // std::cerr << "seting fallout edges. depStk.size=" << depStk.size() <<
      // "in="<< pbb->get_stk_in() << "\n"; set up GEP index
      llvm::Value *tsp = m_builder.CreateLoad(m_stack->get_sp());
      m_builder.CreateStore(
          m_builder.CreateSub(
              tsp, llvm::ConstantInt::get(Type::Int64Ty, depStk.size())),
          m_stack->get_sp());
    }

    m_stack->enableRegs = false;
    if (depStk.size() < pbb->get_stk_in()) {
      // not enough stack elements
      uint32_t threshold = pbb->get_stk_in() - (uint32_t)depStk.size();
      for (uint32_t _ = 0; _ < threshold; ++_) {
        depStk.push_back(m_stack->pop());  // get item from GEP
      }
    }

    // std::cerr << "generate local stack\n";
    m_stack->enableRegs = true;
    for (auto itr = depStk.rbegin(); itr != depStk.rend(); itr++) {
      m_stack->push(*itr);  // push into the local stack
    }

    m_stack->enableRegs = true;
    compileBasicBlock(*pbb);
    m_stack->enableRegs = false;
    // std::cerr << "compiled one basic block\n";

    // maintain the LLVM registers produced from this basic block
    std::vector<llvm::Value *> *vs = new std::vector<llvm::Value *>;
    for (auto value : *m_stack->view()) vs->push_back(value);

    // std::cerr << "pbb->bbs_to.size() = " << pbb->bbs_to.size() << "\n";
    for (auto _pbb : pbb->bbs_to) {
      // analyze.
      std::vector<llvm::Value *> *vs = new std::vector<llvm::Value *>;
      for (auto value : *m_stack->view()) vs->push_back(value);
      waitingBlocks.push(std::make_pair(_pbb, vs));
    }
    // std::cerr << "copied the _stack\n";

    m_stack->enableRegs = false;
    auto now_point = m_builder.saveIP();
    if (auto term = pbb->llvm()->getTerminator())
      m_builder.SetInsertPoint(term);  // Insert before terminator

    for (auto value : *m_stack->view()) m_stack->push(value);  // store with GEP
    m_builder.restoreIP(now_point);

    m_stack->clearLocals();
    delete pstack;
  }

  for (uint32_t idx = 0; idx < blocks.size(); ++idx) {
    BasicBlock bb = blocks[idx];
    if (visited.find(&blocks[idx]) != visited.end()) continue;
    visited.insert(&bb);

    std::vector<llvm::Value *> depStk;
    m_stack->enableRegs = false;

    m_builder.SetInsertPoint(bb.llvm());
    for (uint32_t _ = 0; _ < bb.get_stk_in(); ++_) {
      depStk.push_back(m_stack->pop());  // get item from GEP
    }

    m_stack->enableRegs = true;
    for (auto itr = depStk.rbegin(); itr != depStk.rend(); itr++) {
      m_stack->push(*itr);  // push into the local stack
    }

    m_stack->enableRegs = true;  // use regs only for optimization
    compileBasicBlock(bb);
    m_stack->enableRegs = false;

    auto now_point = m_builder.saveIP();
    if (auto term = bb.llvm()->getTerminator())
      m_builder.SetInsertPoint(term);  // Insert before terminator

    for (auto value : *m_stack->view()) m_stack->push(value);  // store with GEP
    m_builder.restoreIP(now_point);

    m_stack->clearLocals();
  }

  auto func_mfree = m_utilsM.fn_free();
  
  m_builder.SetInsertPoint(m_exitBB);
  // free memory * stack
  m_builder.CreateCall(func_mfree, {m_mem_ptr});
  m_builder.CreateCall(func_mfree, {_stk_i8_ref});
  m_builder.CreateRet(m_builder.getInt32(1));  // return success

  m_builder.SetInsertPoint(m_abortBB);
  // free memory * stack
  m_builder.CreateCall(func_mfree, {m_mem_ptr});
  m_builder.CreateCall(func_mfree, {_stk_i8_ref});
  m_builder.CreateRet(m_builder.getInt32(0));  // return failure

  resolveJumps(blocks);
  resolveGas(m_mainFunc);
  return &m_module;
}

void WASMCompiler::dry_run_bb(BasicBlock const &bb,
                              std::vector<uint64_t> &stack) {
  auto balanced_stk = [](std::vector<uint64_t> *stk, uint64_t out,
                         uint64_t in) {
    uint8_t _;
    for (_ = 0; _ < out; ++_) stk->pop_back();

    for (_ = 0; _ < in; ++_) stk->push_back(0);
  };

  // for each instruction
  for (code_iterator it = bb.begin(); it != bb.end(); ++it) {
    Instruction ins = Instruction(*it);
    uint32_t opcode = static_cast<uint32_t>(ins);

    uint8_t stk_out = std::get<1>(InstrMap[opcode]);
    uint8_t stk_in = std::get<2>(InstrMap[opcode]);
    // std::cerr << "Opcode@ 0x" << std::hex << (uint64_t)opcode << "\n";

    // Pushes first because they are very frequent
    if (0x60 <= opcode && opcode <= 0x7f) {
      // std::cerr << "PUSHn\n";
      llvm::APInt value = readPushData(it, bb.end());
      if (value.getActiveBits() <= 64)
        stack.push_back(value.getZExtValue());
      else
        stack.push_back(0);
      // std::cerr << stack[stack.size() - 1] << "\n";
    } else if (opcode < 0x10) {
      // Arithmetic
      // std::cerr << "arithmetic\n";
      if (ins == Instruction::STOP)
        return;
      else
        balanced_stk(&stack, stk_out, stk_in);
    }

    else if (opcode < 0x20) {
      // Comparisons
      // std::cerr << "compare\n";
      balanced_stk(&stack, stk_out, stk_in);
    } else if (opcode < 0x40) {
      // SHA3 and environment info
      // std::cerr << "sha3&environment\n";
      balanced_stk(&stack, stk_out, stk_in);
    } else if (opcode < 0x50) {
      // Block info
      // std::cerr << "blockinfo\n";
      balanced_stk(&stack, stk_out, stk_in);
    } else if (opcode < 0x60) {
      // VM state manipulations
      // std::cerr << " activated \n";
      if (ins == Instruction::POP) {
        stack.pop_back();
        // std::cerr << " pop \n";
      } else if (ins == Instruction::JUMP || ins == Instruction::JUMPI) {
        return;
      } else {
        // std::cerr << "-> " << stack[0] << " " << stack[1] << " " << "\n";
        balanced_stk(&stack, stk_out, stk_in);
        // std::cerr << stack.size() << "\n";
      }
    }
    // 0x60-0x7f => PUSH_ANY

    // DUPn (eg. DUP1: a b c -> a b c c, DUP3: a b c -> a b c a)
    else if (Instruction::DUP1 <= ins && ins <= Instruction::DUP16) {
      stack.push_back(stack[stack.size() + 0x7f -
                            opcode]);  // 0x7f - opcode is a negative number, -1
                                       // for 0x80 ... -16 for 0x8f
    } else if (Instruction::SWAP1 <= ins && ins <= Instruction::SWAP16) {
      // SWAPn (eg. SWAP1: a b c d -> a b d c, SWAP3: a b c d -> d b c a)
      // 0x8e - opcode is a negative number, -2 for 0x90 ... -17 for 0x9f
      uint64_t tmp = stack[stack.size() - 1];
      stack[stack.size() - 1] = stack[stack.size() + 0x8e - opcode];
      stack[stack.size() + 0x8e - opcode] = tmp;
    } else if (Instruction::LOG0 <= ins && ins <= Instruction::LOG4) {
      /*
      0xa0 ... 0xa4, 32/64/96/128/160 + len(data) gas
      a. Opcodes LOG0...LOG4 are added, takes 2-6 stack arguments
                      MEMSTART MEMSZ (TOPIC1) (TOPIC2) (TOPIC3) (TOPIC4)
      b. Logs are kept track of during tx execution exactly the same way as
      selfdestructs (except as an ordered list, not a set). Each log is in the
      form [address, [topic1, ... ], data] where:
      * address is what the ADDRESS opcode would output
      * data is mem[MEMSTART: MEMSTART + MEMSZ]
      * topics are as provided by the opcode
      c. The ordered list of logs in the transaction are expressed as [log0,
      log1, ..., logN].
      */
      balanced_stk(&stack, stk_out, stk_in);
    }
    // CREATE
    else if (ins == Instruction::CREATE || ins == Instruction::CALL ||
             ins == Instruction::CALLCODE || ins == Instruction::DELEGATECALL ||
             ins == Instruction::CREATE2 || ins == Instruction::STATICCALL) {
      balanced_stk(&stack, stk_out, stk_in);
    }

    else if (ins == Instruction::RETURN || ins == Instruction::REVERT) {
      balanced_stk(&stack, stk_out, stk_in);
      return;
    }

    else if (ins == Instruction::SUICIDE) {
      balanced_stk(&stack, stk_out, stk_in);
      return;  // SELFDESTRUCT opcode (also called SELFDESTRUCT)
    }
  }
}

// build cfg inside each basic block
void WASMCompiler::dry_run(std::vector<BasicBlock> &blocks) {
  std::map<uint64_t, BasicBlock *> idxMap;
  for (auto i = 0; i < blocks.size(); i++) {
    idxMap[blocks[i].firstInstrIdx()] = &blocks[i];
  }

  // assign register pointers for each basic block individually.
  std::vector<std::pair<BasicBlock *, std::vector<uint64_t> *>> dfs;

  std::vector<uint64_t> *root = new std::vector<uint64_t>;
  dfs.push_back(std::make_pair(&blocks[0], root));

  std::set<BasicBlock *> visited;
  while (!dfs.empty()) {
    // std::cerr << "\ndfs size = " << dfs.size() << "\n";
    BasicBlock *bb_ptr = dfs.back().first;
    std::vector<uint64_t> *stk_ptr = dfs.back().second;
    dfs.pop_back();

    // std::cerr << "BB# " << std::hex << " " << bb_ptr->firstInstrIdx() << "-";

    // edges found
    if (visited.find(bb_ptr) != visited.end())
      continue;
    else
      visited.insert(bb_ptr);

    uint64_t pc = (uint64_t)(bb_ptr->terminatorIdx());
    Instruction ternimator = Instruction(*bb_ptr->terminator());
    // std::cerr << " " << *bb_ptr->end() << "@@";
    // std::cerr << "<---->" << pc << ":" << static_cast<uint64_t>(ternimator)
    // << "\n";

    dry_run_bb(*bb_ptr, *stk_ptr);

    if (ternimator == Instruction::JUMP) {
      uint64_t jump_pc = stk_ptr->back();
      stk_ptr->pop_back();
      BasicBlock *next_bb = idxMap[jump_pc];
      if (next_bb && Instruction(*next_bb->begin()) == Instruction::JUMPDEST) {
        bb_ptr->bbs_to.push_back(next_bb);
        next_bb->bbs_from.push_back(bb_ptr);

        std::vector<uint64_t> *new_stk_ptr = new std::vector<uint64_t>;
        new_stk_ptr->assign(stk_ptr->begin(), stk_ptr->end());
        dfs.push_back(std::make_pair(next_bb, new_stk_ptr));
        // std::cerr << "JUMP " << pc << " -> " << jump_pc << "\n";
        // std::cerr << "=>" << std::hex << next_bb->firstInstrIdx() <<
        // "--------" << (uint64_t)next_bb->terminatorIdx()  << " : " <<
        // (uint32_t)std::get<1>(InstrMap[static_cast<uint64_t>(Instruction(*next_bb->terminator()))])
        // <<  "\n";
      } else {
        bb_ptr->bbs_to.push_back(nullptr);  // INVALID
        continue;
      }
    }

    else if (ternimator == Instruction::JUMPI) {
      uint64_t jump_pc = stk_ptr->back();
      stk_ptr->pop_back();
      stk_ptr->pop_back();

      // fallout
      uint64_t fallout_pc = pc + 1;
      BasicBlock *fallout_bb = idxMap[fallout_pc];
      if (static_cast<uint32_t>(*fallout_bb->begin()) != 0xfe) {
        bb_ptr->bbs_to.push_back(fallout_bb);
        fallout_bb->bbs_from.push_back(bb_ptr);

        std::vector<uint64_t> *fall_stk_ptr = new std::vector<uint64_t>;
        fall_stk_ptr->assign(stk_ptr->begin(), stk_ptr->end());

        dfs.push_back(std::make_pair(fallout_bb, fall_stk_ptr));
        // std::cerr << "JUMPI-fall " << pc << " -> " << fallout_pc << "\n";
        // std::cerr << "=====>" << std::hex << fallout_bb->firstInstrIdx() <<
        // "--------" << (uint64_t)fallout_bb->terminatorIdx() << " : " <<
        // (uint32_t)std::get<1>(InstrMap[static_cast<uint64_t>(Instruction(*fallout_bb->terminator()))])
        // << "\n";
      } else {
        // INVALID
        bb_ptr->bbs_to.push_back(nullptr);  // INVALID
      }

      // std::cerr << "JUMPI-next\n";
      // jump
      BasicBlock *jump_bb = idxMap[jump_pc];
      if (jump_bb && Instruction(*jump_bb->begin()) == Instruction::JUMPDEST) {
        bb_ptr->bbs_to.push_back(jump_bb);
        jump_bb->bbs_from.push_back(bb_ptr);

        std::vector<uint64_t> *next_stk_ptr = new std::vector<uint64_t>;
        next_stk_ptr->assign(stk_ptr->begin(), stk_ptr->end());
        dfs.push_back(std::make_pair(jump_bb, next_stk_ptr));
        // std::cerr << "JUMPI-next " << pc << " -> " << jump_pc << "\n";
        // std::cerr << "=>" << std::hex << jump_bb->firstInstrIdx() <<
        // "--------" << (uint64_t)jump_bb->terminatorIdx()  << " : " <<
        // (uint32_t)std::get<1>(InstrMap[static_cast<uint64_t>(Instruction(*jump_bb->terminator()))])
        // << "\n";
      } else {
        // std::cerr << "invalid\n";
        bb_ptr->bbs_to.push_back(nullptr);  // INVALID
      }
      // std::cerr << "JUMPI-next-set\n";
    }

    else if (ternimator == Instruction::STOP ||
             ternimator == Instruction::RETURN ||
             ternimator == Instruction::REVERT ||
             ternimator == Instruction::SUICIDE) {
      // stk_ptr->clear();
      delete stk_ptr;
      continue;
    }
    // std::cerr << "Go to next block\n";
  }
  // std::cerr << "\n[+] CFG Generated!\n";
}

std::vector<llvm::ConstantInt *> WASMCompiler::compileBasicBlock(BasicBlock &_basicBlock) {
  m_builder.SetInsertPoint(_basicBlock.llvm());
  // std::cerr << "--------------- Current Process bb#" << std::hex
  //     << _basicBlock.firstInstrIdx() << "-------------- \n";

  llvm::Argument *const arg_caller_ptr = m_mainFunc->getArg(0);
  llvm::Argument *const arg_callvalue_ptr = m_mainFunc->getArg(1);
  llvm::Argument *const arg_calldata_ptr = m_mainFunc->getArg(2);
  llvm::Argument *const arg_calldata_size = m_mainFunc->getArg(3);
  
  llvm::LLVMContext &Context = m_builder.getContext();

  bool invalid = false;
  for (auto it = _basicBlock.begin(); it != _basicBlock.end(); ++it) {
    auto inst = Instruction(*it);
    uint32_t pc = (uint32_t)(_basicBlock.firstInstrIdx() + it - _basicBlock.begin());
    llvm::MDNode *MetaPC = llvm::MDNode::get(
        Context, llvm::ConstantAsMetadata::get(
        llvm::ConstantInt::get(Context, llvm::APInt(32, pc, false))));
    // std::cerr << "PC: " << std::hex << pc << " :" << (int)*it << "\n";
    if (false) {
      llvm::GlobalVariable* GPCSBuff = m_module.getNamedGlobal(DEBUG_PCS);
      if (!GPCSBuff) {
        GPCSBuff = new llvm::GlobalVariable(
          m_module, llvm::ArrayType::get(Type::Int32Ty, DEBUG_PCS_MAX), false, 
          llvm::GlobalVariable::ExternalLinkage, nullptr, DEBUG_PCS, nullptr,
          llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);
      }
      llvm::GlobalVariable* GPCSPtr = m_module.getNamedGlobal(DEBUG_PCS_PTR);
      if (!GPCSPtr) {
        GPCSPtr = new llvm::GlobalVariable(m_module, Type::Int32Ty, false, 
          llvm::GlobalVariable::ExternalLinkage, llvm::ConstantInt::get(Type::Int32Ty, 0), 
          DEBUG_PCS_PTR, nullptr, llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false); 
      }
      llvm::Value *PCVal = m_builder.CreateLoad(Type::Int32Ty, GPCSPtr);
      llvm::Value *Ptr = m_builder.CreateGEP(GPCSBuff, {m_builder.getInt32(0), PCVal});
      m_builder.CreateStore(m_builder.getInt32(pc), Ptr);
      m_builder.CreateStore(m_builder.CreateAdd(PCVal, m_builder.getInt32(1)), GPCSPtr);
    }

    switch (inst) {
      case Instruction::STOP: {
        m_builder.CreateBr(m_exitBB);
        break;
      }
      case Instruction::ADD: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        auto result = m_builder.CreateAdd(lhs, rhs);
        m_stack->push(result);

        llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        llvm::MDNode *N = llvm::MDNode::get(Context, llvm::MDString::get(Context, "add"));
        Inst->setMetadata(c_intsan, N);
        Inst->setMetadata(c_pc, MetaPC);
        break;
      }
      case Instruction::MUL: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *result = m_builder.CreateMul(lhs, rhs);
        m_stack->push(result);

        llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        llvm::MDNode *N = llvm::MDNode::get(Context, 
                                            llvm::MDString::get(Context, "mul"));
        Inst->setMetadata(c_intsan, N);

        Inst->setMetadata(c_pc, MetaPC);
        break;
      }
      case Instruction::SUB: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        auto result = m_builder.CreateSub(lhs, rhs);
        m_stack->push(result);

        llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        llvm::MDNode *N = llvm::MDNode::get(Context, llvm::MDString::get(Context, "sub"));
        Inst->setMetadata(c_intsan, N);
        Inst->setMetadata(c_pc, MetaPC);
        break;
      }
      case Instruction::DIV: {
        auto d = m_stack->pop();
        auto n = m_stack->pop();

        if (d->getType() != n->getType()) {
          d = m_builder.CreateZExt(d, Type::Int256Ty);
          n = m_builder.CreateZExt(n, Type::Int256Ty);
        }

        llvm::Function *func = arith::getUDiv256Func(m_module);

        llvm::Value *pd = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(d, pd);
        llvm::Value *pn = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(n, pn);
        llvm::Value *presult = m_builder.CreateAlloca(Type::Word);
        llvm::CallInst *Inst = m_builder.CreateCall(func, {pd, pn, presult});
        m_stack->push(m_builder.CreateLoad(Type::Word, presult));
        
        Inst->setMetadata(c_pc, MetaPC);
        llvm::MDNode* N = llvm::MDNode::get(Context, llvm::MDString::get(Context, "udiv")); 
        Inst->setMetadata(c_intsan, N);
        break;
      }
      case Instruction::SDIV: {
        auto d = m_stack->pop();
        auto n = m_stack->pop();
        if (d->getType() != n->getType()) {
          d = m_builder.CreateZExt(d, Type::Int256Ty);
          n = m_builder.CreateZExt(n, Type::Int256Ty);
        }

        llvm::Value *pd = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(d, pd);
        llvm::Value *pn = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(n, pn);
        llvm::Value *presult = m_builder.CreateAlloca(Type::Word);
        llvm::Function *func = arith::getSDiv256Func(m_module);
        llvm::CallInst *Inst = m_builder.CreateCall(func, {pd, pn, presult});
        m_stack->push(m_builder.CreateLoad(Type::Word, presult));

        Inst->setMetadata(c_pc, MetaPC);
        llvm::MDNode* N = llvm::MDNode::get(Context, llvm::MDString::get(Context, "sdiv")); 
        Inst->setMetadata(c_intsan, N);
        break;
      }
      case Instruction::MOD: {
        auto d = m_stack->pop();
        auto n = m_stack->pop();
        if (d->getType() != n->getType()) {
          d = m_builder.CreateZExt(d, Type::Word);
          n = m_builder.CreateZExt(n, Type::Word);
        }
        llvm::Value *result = m_builder.CreateURem(d, n);
        m_stack->push(result);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SMOD: {
        auto d = m_stack->pop();
        auto n = m_stack->pop();
        if (d->getType() != n->getType()) {
          d = m_builder.CreateZExt(d, Type::Word);
          n = m_builder.CreateZExt(n, Type::Word);
        }
        llvm::Value *result = m_builder.CreateSRem(d, n);
        m_stack->push(result);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::ADDMOD: {
        auto i512Ty = m_builder.getIntNTy(512);
        auto a = m_stack->pop();
        auto b = m_stack->pop();
        auto m = m_stack->pop();
        auto divByZero = m_builder.CreateICmpEQ(m, Constant::get(0));
        a = m_builder.CreateZExt(a, i512Ty);
        b = m_builder.CreateZExt(b, i512Ty);
        m = m_builder.CreateZExt(m, i512Ty);
        auto result = m_builder.CreateNUWAdd(a, b);
        result = m_builder.CreateURem(result, m);
        result = m_builder.CreateSelect(divByZero, 
            Constant::get(0), m_builder.CreateTrunc(result, Type::Word));
        m_stack->push(result);
        // auto i256Zero =  m_builder.getIntN(256, 0);
        // auto a = m_stack->pop();
        // auto b = m_stack->pop();
        // auto m = m_stack->pop();
        // a = m_builder.CreateZExt(a, Type::Word);
        // b = m_builder.CreateZExt(b, Type::Word);
        // m = m_builder.CreateZExt(m, Type::Word);
        // auto result = m_builder.CreateAdd(a, b);
        // result = m_builder.CreateURem(result, m);
        // result = m_builder.CreateSelect(
        //     m_builder.CreateICmpEQ(m, i256Zero), i256Zero, result);
        // m_stack->push(result);

        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::MULMOD: {
        auto i512Ty = m_builder.getIntNTy(512);
        auto a = m_stack->pop();
        auto b = m_stack->pop();
        auto m = m_stack->pop();
        auto divByZero = m_builder.CreateICmpEQ(m, Constant::get(0));
        a = m_builder.CreateZExt(a, i512Ty);
        b = m_builder.CreateZExt(b, i512Ty);
        m = m_builder.CreateZExt(m, i512Ty);
        auto p = m_builder.CreateNUWMul(a, b);
        p = m_builder.CreateURem(p, m);
        p = m_builder.CreateTrunc(p, Type::Word);
        p = m_builder.CreateSelect(divByZero, Constant::get(0), p);
        m_stack->push(p);
        // auto i256Zero =  m_builder.getIntN(256, 0);
        // auto a = m_stack->pop();
        // auto b = m_stack->pop();
        // auto m = m_stack->pop();
        // a = m_builder.CreateZExt(a, Type::Word);
        // b = m_builder.CreateZExt(b, Type::Word);
        // m = m_builder.CreateZExt(m, Type::Word);

        // auto p = m_builder.CreateMul(a, b);
        // p = m_builder.CreateURem(p, m);
        // // p = m_builder.CreateTrunc(p, Type::Word);
        // // m = m_builder.CreateTrunc(m, Type::Word);
        // p = m_builder.CreateSelect(m_builder.CreateICmpEQ(m, i256Zero),
        // i256Zero, p); m_stack->push(p);

        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(p);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::EXP: {
        llvm::Value *base = m_stack->pop();
        llvm::Value *exponent = m_stack->pop();
        llvm::Value *ret = nullptr;

        llvm::SmallString<40> TmpB, TmpE, TmpR;
        if (auto c1 = llvm::dyn_cast<llvm::ConstantInt>(base)) {
          c1->getValue().toStringUnsigned(TmpB, 16);
          // std::cerr << TmpB.str().str() << "**";
          if (auto c2 = llvm::dyn_cast<llvm::ConstantInt>(exponent)) {
            auto b = c1->getValue();
            auto e = c2->getValue().getZExtValue();
            auto r = llvm::APInt{256, 1};
            while (e != 0) {
              r *= b;
              e--;
            }
            ret = Constant::get(r); 
            c2->getValue().toStringUnsigned(TmpE, 16);
            r.toStringUnsigned(TmpR, 16);
            // std::cerr << TmpE.str().str() << "=" << TmpR.str().str() << "\n";
          } 
        }
        if (ret == nullptr) {
          llvm::Value *pbase = m_builder.CreateAlloca(Type::Word);
          m_builder.CreateStore(m_builder.CreateZExt(base, Type::Word), pbase);
          llvm::Value *pexponent = m_builder.CreateAlloca(Type::Word);
          m_builder.CreateStore(m_builder.CreateZExt(exponent, Type::Word), pexponent);

          llvm::Value *pret = m_builder.CreateAlloca(Type::Word);
          m_builder.CreateCall(arith::getExpFunc(m_module),
                              {pbase, pexponent, pret});
          ret = m_builder.CreateLoad(pret, Type::Word);
        }
        m_stack->push(ret);

        // // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(ret);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SIGNEXTEND: {
        auto idx = m_stack->pop();
        auto word = m_stack->pop();

        auto k32_ = m_builder.CreateTrunc(idx, m_builder.getIntNTy(5), "k_32");
        auto k32 = m_builder.CreateZExt(k32_, Type::Size);
        auto k32x8 = m_builder.CreateMul(k32, m_builder.getInt64(8), "kx8");

        // test for word >> (k * 8 + 7)
        auto bitpos = m_builder.CreateAdd(k32x8, m_builder.getInt64(7), "bitpos");
        auto bitposEx = m_builder.CreateZExt(bitpos, Type::Word);
        auto bitval = m_builder.CreateLShr(word, bitposEx, "bitval");
        auto bittest = m_builder.CreateTrunc(bitval, Type::Bool, "bittest");

        auto mask_ = m_builder.CreateShl(Constant::get(1), bitposEx);
        auto mask = m_builder.CreateSub(mask_, Constant::get(1), "mask");

        auto negmask = m_builder.CreateXor(
            mask, llvm::ConstantInt::getAllOnesValue(Type::Word), "negmask");
        auto val1 = m_builder.CreateOr(word, negmask);
        auto val0 = m_builder.CreateAnd(word, mask);

        auto kInRange = m_builder.CreateICmpULE(
            idx, llvm::ConstantInt::get(Type::Word, 30));
        auto result = m_builder.CreateSelect(
            kInRange, m_builder.CreateSelect(bittest, val1, val0), word,
            "signed");

        // llvm::MDNode* N = llvm::MDNode::get(m_builder.getContext(),
        // llvm::MDString::get(m_builder.getContext(), "signed"));
        // llvm::SelectInst *Inst = llvm::cast<llvm::SelectInst>(result);
        // Inst->setMetadata("stats.signed",
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(0))));

        m_stack->push(result);
        break;
      }
      case Instruction::LT: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *res = m_builder.CreateICmpULT(lhs, rhs);
        m_stack->push(res);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res256);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::GT: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *res = m_builder.CreateICmpUGT(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res256);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SLT: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *res = m_builder.CreateICmpSLT(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res256);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SGT: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *res = m_builder.CreateICmpSGT(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res256);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::EQ: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *res = m_builder.CreateICmpEQ(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res256);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::ISZERO: {
        auto top = m_stack->pop();
        llvm::Value *iszero = m_builder.CreateICmpEQ(
            top, llvm::ConstantInt::get(top->getType(), 0));
        // llvm::Value *result = m_builder.CreateZExt(iszero, Type::Word);
        m_stack->push(iszero);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::AND: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }

        llvm::Value *res = m_builder.CreateAnd(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::OR: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }

        llvm::Value *res = m_builder.CreateOr(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::XOR: {
        auto lhs = m_stack->pop();
        auto rhs = m_stack->pop();
        if (lhs->getType() != rhs->getType()) {
          lhs = m_builder.CreateZExt(lhs, Type::Word);
          rhs = m_builder.CreateZExt(rhs, Type::Word);
        }
        llvm::Value *res = m_builder.CreateXor(lhs, rhs);
        m_stack->push(res);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::NOT: {
        auto value = m_stack->pop();
        llvm::Value *result = m_builder.CreateNot(value);
        m_stack->push(result);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(result);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::BYTE: {
        const auto idx = m_stack->pop();
        auto value = m_stack->pop();

        auto idxValid = m_builder.CreateICmpULT(idx, Constant::get(32), "idxValid");
        auto _fixed_vec32 = llvm::VectorType::get(
            /*ElementType=*/Type::Byte, /*NumElements=*/32, /*Scalable=*/false);
        auto bytes = m_builder.CreateBitCast(value, _fixed_vec32, "bytes");
        // TODO: Workaround for LLVM bug. Using big value of index causes
        // invalid memory access.
        auto safeIdx = m_builder.CreateTrunc(idx, m_builder.getIntNTy(5));
        // TODO: Workaround for LLVM bug. DAG Builder used sext on index instead
        // of zext
        safeIdx = m_builder.CreateZExt(safeIdx, Type::Size);
        auto byte = m_builder.CreateExtractElement(bytes, safeIdx, "byte");
        value = m_builder.CreateZExt(byte, Type::Word);
        value = m_builder.CreateSelect(idxValid, value, Constant::get(0));
        m_stack->push(value);

        // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(value);
        // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SHL: {
        auto shift = m_stack->pop();
        auto value = m_stack->pop();
        if (shift->getType() != value->getType()) {
          shift = m_builder.CreateZExt(shift, Type::Word);
          value = m_builder.CreateZExt(value, Type::Word);
        }
        llvm::Value *res = m_builder.CreateShl(value, shift, "ShlAssign");
        m_stack->push(res);

        break;
      }
      case Instruction::SHR: {
        auto shift = m_stack->pop();
        auto value = m_stack->pop();
        if (shift->getType() != value->getType()) {
          shift = m_builder.CreateZExt(shift, Type::Word);
          value = m_builder.CreateZExt(value, Type::Word);
        }
        llvm::Value *res = m_builder.CreateLShr(value, shift, "ShrAssign");
        m_stack->push(res);

        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SAR: {
        // signed
        auto shift = m_stack->pop();
        auto value = m_stack->pop();
        if (shift->getType() != value->getType()) {
          shift = m_builder.CreateZExt(shift, Type::Word);
          value = m_builder.CreateZExt(value, Type::Word);
        }
        llvm::Value *res = m_builder.CreateAShr(value, shift, "SarAssign");
        m_stack->push(res);

        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(res);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::SHA3: {
        auto inOff = m_stack->pop();
        auto inSize = m_stack->pop();
        if (inOff->getType() != Type::Int32Ty)
          inOff = m_builder.CreateTrunc(inOff, Type::Int32Ty);
        if (inSize->getType() != Type::Int32Ty)
          inSize = m_builder.CreateTrunc(inSize, Type::Int32Ty);
        
        // inOff = m_builder.CreateURem(inOff, m_builder.getInt32(EVM_MEM_SIZE));
        // inSize = m_builder.CreateURem(inSize, m_builder.CreateSub(m_builder.getInt32(EVM_MEM_SIZE), inOff));

        llvm::Value *memSrc = m_builder.CreateInBoundsGEP(m_mem_ptr, {inOff});

        llvm::Value *hash = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateCall(m_utilsM.getHashFunc(),
            {memSrc, inSize, m_builder.CreateBitCast(hash, Type::BytePtr)});
        m_stack->push(m_builder.CreateLoad(hash));
        break;
      }
      case Instruction::ADDRESS: {
        llvm::Value *SelfAddress = m_builder.CreateLoad(
            m_module.getNamedGlobal(SELFADDR_REF));
        m_stack->push(SelfAddress);

        break;
      }
      case Instruction::BALANCE: {
        llvm::Value *address = m_stack->pop();
        llvm::Value *balance = Constant::get(getIntialBalance());
        balance->setName("?.balance");
        m_stack->push(balance);
        break;
      }
      case Instruction::ORIGIN: {
        llvm::Value *V = m_builder.CreateLoad(m_module.getNamedGlobal(ORIGIN_REF));
        m_stack->push(V);

        auto Inst = llvm::cast<llvm::Instruction>(V);
        Inst->setMetadata(c_orgsan, llvm::MDNode::get(Context, llvm::None));
        Inst->setMetadata(c_pc, MetaPC);
        break;
      }
      case Instruction::CALLER: {       
        llvm::Value *V = m_builder.CreateLoad(arg_caller_ptr);
        m_stack->push(V);
        break;
      }
      case Instruction::CALLVALUE: {
        m_stack->push(m_builder.CreateLoad(arg_callvalue_ptr));
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(value);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::CALLDATALOAD: {
        auto offset = m_stack->pop();
        offset = m_builder.CreateZExtOrTrunc(offset, Type::Int64Ty);
        auto dest256Ptr = m_builder.CreateAlloca(Type::Word, nullptr);
        auto dest8Ptr = m_builder.CreateBitCast(dest256Ptr, Type::BytePtr);
        m_builder.CreateCall(m_utilsM.getCalldataLoad(), 
            {dest8Ptr, arg_calldata_ptr, offset});
        llvm::Value *value = m_builder.CreateLoad(Type::Word, dest256Ptr);
        m_stack->push(value);
        break;
      }
      case Instruction::CALLDATASIZE: {
        if (m_builder.GetInsertBlock()->getParent()->getName() == CUDA_CONTRACT_FUNC) 
          m_stack->push(arg_calldata_size);
        else
          m_stack->push(m_builder.getInt32(0));
        break;
      }
      case Instruction::CALLDATACOPY: {
        llvm::Value *destOffset = m_stack->pop();
        llvm::Value *offset = m_stack->pop();
        llvm::Value *size = m_stack->pop();
        if (destOffset->getType() != Type::Int64Ty)
          destOffset = m_builder.CreateZExtOrTrunc(destOffset, Type::Int64Ty);
        if (offset->getType() != Type::Int64Ty)
          offset = m_builder.CreateZExtOrTrunc(offset, Type::Int64Ty);
        if (size->getType() != Type::Int64Ty)
          size = m_builder.CreateZExtOrTrunc(size, Type::Int64Ty);

        m_builder.CreateCall(m_utilsM.getCalldataCopy(),
            {m_mem_ptr, destOffset, arg_calldata_ptr, offset, size});

        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(callInst);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::CODESIZE: {
        llvm::Value *Codesize;
        if (m_builder.GetInsertBlock()->getParent()->getName() == CUDA_DEPLOY_FUNC) {
          llvm::Value *CodesizePtr = m_module.getNamedGlobal(EVMCODESIZE_REG);
          Codesize = m_builder.CreateLoad(Type::Int32Ty, CodesizePtr, "codesize");
        } else {
          auto InitialSizePtr = m_module.getNamedGlobal(EVMCODESIZE_REG)->getInitializer();
          auto ConstInt = llvm::cast<llvm::ConstantInt>(InitialSizePtr)->getZExtValue();
          uint32_t Offset = this->getBytecodeOffset();
          std::cerr << "CODESIZE: Runtime Code Size = " << ConstInt - Offset << "\n";
          Codesize = m_builder.getInt32(ConstInt - Offset);
        }
        m_stack->push(Codesize);
        break;
      }
      case Instruction::CODECOPY: {
        auto destMemIdx = m_stack->pop();
        auto srcIdx = m_stack->pop();
        auto reqBytes = m_stack->pop();

        if (destMemIdx->getType() != Type::Int32Ty)
          destMemIdx = m_builder.CreateZExtOrTrunc(destMemIdx, Type::Int32Ty);
        if (reqBytes->getType() != Type::Int32Ty)
          reqBytes = m_builder.CreateZExtOrTrunc(reqBytes, Type::Int32Ty);
        if (srcIdx->getType() != Type::Int32Ty)
          srcIdx = m_builder.CreateZExtOrTrunc(srcIdx, Type::Int32Ty);

        destMemIdx = m_builder.CreateURem(destMemIdx, m_builder.getInt32(EVM_MEM_SIZE));
        llvm::Value *destMemAddr = m_builder.CreateGEP(m_mem_ptr, {destMemIdx});

        llvm::GlobalValue *CodeG = m_module.getNamedGlobal(EVMCODE_REG);
        if (m_builder.GetInsertBlock()->getParent()->getName() == CUDA_CONTRACT_FUNC) {
          uint32_t Offset = this->getBytecodeOffset();
          srcIdx = m_builder.CreateAdd(m_builder.getInt32(Offset), srcIdx);
        }

        auto Codesize = m_builder.CreateLoad(m_module.getNamedGlobal(EVMCODESIZE_REG));
        
        srcIdx = m_builder.CreateURem(srcIdx, Codesize);
        reqBytes = m_builder.CreateURem(reqBytes, Codesize);

        reqBytes = m_builder.CreateSelect(
            m_builder.CreateICmpUGT(m_builder.CreateAdd(srcIdx, reqBytes), Codesize),
            m_builder.getInt32(0), reqBytes);

        reqBytes = m_builder.CreateSelect(
            m_builder.CreateICmpUGT(m_builder.CreateAdd(destMemIdx, reqBytes), m_builder.getInt32(EVM_MEM_SIZE)),
            m_builder.getInt32(0), reqBytes);

        llvm::Value *BytecodePtr = m_builder.CreateGEP(CodeG, {m_builder.getInt32(0), srcIdx});

        // m_builder.CreateMemCpy(destMemAddr, llvm::MaybeAlign(1), BytecodePtr, llvm::MaybeAlign(1), reqBytes);
        static const auto funcName = "llvm.memcpy.p0i8.p4i8.i32";
        llvm::Function *memcpy = m_module.getFunction(funcName);
        if (memcpy == nullptr) {
          auto _funcTy = llvm::FunctionType::get(Type::Void, 
              {Type::BytePtr, Type::Byte->getPointerTo(AS::CONST),
              Type::Int32Ty, Type::Bool}, false);
          memcpy = llvm::Function::Create(
              _funcTy, llvm::Function::ExternalLinkage, funcName, m_module);
        }
        m_builder.CreateCall(memcpy, {destMemAddr, BytecodePtr, 
            reqBytes, m_builder.getFalse()});
        break;
      }
      case Instruction::GASPRICE: {
        llvm::Value *gasprice = m_builder.getInt64(100);
        gasprice->setName("gasprice");
        m_stack->push(gasprice);
        break;
      }
      case Instruction::EXTCODESIZE: {
        auto address = m_stack->pop();
        // if (address->getType() != Type::AddressTy)
        // 	address = m_builder.CreateTruncOrBitCast(address, Type::AddressTy);
        // auto AddressPtr = m_builder.CreateAlloca(Type::AddressTy, nullptr);
        // if (address->getType() != Type::AddressTy)
        // 	address = m_builder.CreateTruncOrBitCast(address,
        // Type::AddressTy);
        // m_builder.CreateStore(m_utilsM.emitEndianConvert(address), AddressPtr);
        m_stack->push(Constant::get(1));  // a COA account
        break;
      }
      case Instruction::EXTCODECOPY: {
        auto addr = m_stack->pop();
        auto destMemIdx = m_stack->pop();
        auto srcIdx = m_stack->pop();
        auto reqBytes = m_stack->pop();
        // ignore extcodecopy.
        break;
      }
      case Instruction::RETURNDATASIZE: {
        llvm::Value *returnsize = m_builder.getInt64(0);
        returnsize->setName("returnsize");
        m_stack->push(returnsize);
        break;
      }
      case Instruction::RETURNDATACOPY: {
        auto destOffset = m_stack->pop();
        auto offset = m_stack->pop();
        auto size = m_stack->pop();
        // llvm::Value *dest_mem_addr =
        //     m_builder.CreateGEP(m_mem_ptr, {destMemIdx});
        // if (srcIdx->getType() != Type::Int32Ty)
        //   srcIdx = m_builder.CreateTruncOrBitCast(srcIdx, Type::Int32Ty);
        // if (reqBytes->getType() != Type::Int32Ty)
        //   reqBytes = m_builder.CreateTruncOrBitCast(reqBytes, Type::Int32Ty);

        // m_builder.CreateCall(m_utilsM.getMstore(), 
            // {m_mem_ptr, destOffset, ValPtr8, m_builder.getInt64(32)});

        break;
      }
      case Instruction::EXTCODEHASH: {
        auto address = m_stack->pop();
        m_stack->push(Constant::get(0xabcd1234));
        break;
      }
      case Instruction::BLOCKHASH: {
        llvm::Value *number = m_stack->pop();
        llvm::Value *blockhash = m_builder.getInt64(0x12345abcd);
        blockhash->setName("?.blockhash");
        m_stack->push(blockhash);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(hash);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::COINBASE: {
        llvm::Value *coinbase = m_builder.getInt32(0);
        coinbase->setName("coinbase");
        m_stack->push(coinbase);

        break;
      }
      case Instruction::TIMESTAMP: {
        llvm::Value *Timestamp = m_builder.CreateLoad(
            m_module.getNamedGlobal(TIMESTAMP_REF), "timestamp");
        m_stack->push(Timestamp); 

        auto Inst = llvm::cast<llvm::Instruction>(Timestamp);
        Inst->setMetadata(c_bsdsan, llvm::MDNode::get(Context, llvm::None));
        Inst->setMetadata(c_pc, MetaPC);

        break;
      }
      case Instruction::NUMBER: {
        llvm::Value *BlockNum = m_builder.CreateLoad(
            m_module.getNamedGlobal(BLOCKNUM_REF), "blockheight");
        m_stack->push(BlockNum); 

        auto Inst = llvm::cast<llvm::Instruction>(BlockNum);
        Inst->setMetadata(c_bsdsan, llvm::MDNode::get(Context, llvm::None));
        Inst->setMetadata(c_pc, MetaPC);
        break;
      }
      case Instruction::DIFFICULTY: {
        llvm::Value *difficulty = m_builder.getInt64(10000000);
        difficulty->setName("difficulty");
        m_stack->push(difficulty);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(value);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::GASLIMIT: {
        llvm::Value *gaslimit = m_builder.getInt64(GASLIMIT_MAX);
        gaslimit->setName("gaslimit");
        m_stack->push(gaslimit);  // unlimited

        break;
      }
      case Instruction::CHAINID: {
        llvm::Value *chainid = m_builder.getInt32(1);
        chainid->setName("chainid");
        m_stack->push(chainid);

        break;
      }
      case Instruction::SELFBALANCE: {
        llvm::Value *selfbalance = m_builder.CreateLoad(m_module.getNamedGlobal(BALANCECALLER_REF));
        selfbalance->setName("this.balance");
        m_stack->push(selfbalance); 
        break;
      }
      case Instruction::BASEFEE: {
        m_stack->push(m_builder.getInt32(65535));
        break;
      }
      case Instruction::POP: {
        m_stack->pop();
        break;
      }
      case Instruction::MLOAD: {
        llvm::Value *Offset = m_stack->pop();
        if (Offset->getType() != Type::Int64Ty)
          Offset = m_builder.CreateZExtOrTrunc(Offset, Type::Int64Ty);
        llvm::Value *ValPtr = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateCall(m_utilsM.getMload(), {m_mem_ptr, Offset, ValPtr});
        llvm::Value *Val = m_builder.CreateLoad(ValPtr);
        m_stack->push(Val);


        break;
      }
      case Instruction::MSTORE: {        
        llvm::Value *Offset = m_stack->pop();
        llvm::Value *Value = m_stack->pop();    
        if (Offset->getType() != Type::Int64Ty)
          Offset = m_builder.CreateZExtOrTrunc(Offset, Type::Int64Ty);
        if (Value->getType() != Type::Word)
          Value = m_builder.CreateZExt(Value, Type::Word);
        auto ValPtr = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(Value, ValPtr);
        llvm::Value *ValPtr8 = m_builder.CreateBitCast(ValPtr, Type::BytePtr);

        m_builder.CreateCall(m_utilsM.getMstore(), 
            {m_mem_ptr, Offset, ValPtr8, m_builder.getInt64(32)});

        break;
      }
      case Instruction::MSTORE8: {
        llvm::Value *Offset = m_stack->pop();
        llvm::Value *Value8 = m_stack->pop();    
        if (Offset->getType() != Type::Int64Ty)
          Offset = m_builder.CreateZExtOrTrunc(Offset, Type::Int64Ty);
        if (Value8->getType() != Type::Byte)
          Value8 = m_builder.CreateTrunc(Value8, Type::Byte);
        auto ValPtr8 = m_builder.CreateAlloca(Type::Byte);
        m_builder.CreateStore(Value8, ValPtr8);
        m_builder.CreateCall(m_utilsM.getMstore(), 
            {m_mem_ptr, Offset, ValPtr8, m_builder.getInt64(1)});
        break;
      }
      case Instruction::SLOAD: {
        auto index = m_stack->pop();
        if (index->getType() != Type::Word)
          index = m_builder.CreateZExt(index, Type::Word);
        llvm::Value *pindex = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(index, pindex);
        llvm::Value *pvalue = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateCall(m_utilsM.getStorageLoad(), {pindex, pvalue});
        m_stack->push(m_builder.CreateLoad(Type::Word, pvalue));

        break;
      }
      case Instruction::SSTORE: {
        auto index = m_stack->pop();
        auto value = m_stack->pop();

        if (index->getType() != Type::Word)
          index = m_builder.CreateZExt(index, Type::Word);
        llvm::Value *pindex = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(index, pindex);

        if (value->getType() != Type::Word)
          value = m_builder.CreateZExt(value, Type::Word);
        llvm::Value *pvalue = m_builder.CreateAlloca(Type::Word);
        m_builder.CreateStore(value, pvalue);
        m_builder.CreateCall(m_utilsM.getStorageStore(), {pindex, pvalue});

        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(callInst);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::JUMP:
      case Instruction::JUMPI: {
        auto JumpDest = m_stack->pop();
        auto idx = m_builder.CreateTrunc(JumpDest, Type::Int64Ty);
        llvm::BranchInst *jumpInst = nullptr;
        if (inst == Instruction::JUMP) {
          // JUMP
          m_builder.CreateStore(idx, m_targetPtr);
          jumpInst = m_builder.CreateBr(m_jumpTableBB);
        } else {
          // JUMPI
          llvm::Value *rawCond = m_stack->pop();
          llvm::Value *cond = m_builder.CreateICmpNE(rawCond, 
              llvm::ConstantInt::get(rawCond->getType(), 0), "jump.check");
          m_builder.CreateStore(idx, m_targetPtr);
          jumpInst = m_builder.CreateCondBr(cond, m_jumpTableBB,
              _basicBlock.llvm()->getNextNode());
        }
        /* Attach medatada to branch instruction with 
           information about destination index. */
        auto destIdx = llvm::MDNode::get(Context,
            llvm::ValueAsMetadata::get(JumpDest));
        jumpInst->setMetadata(c_destIdxLabel, destIdx);
        break;
      }
      case Instruction::PC: {
        auto value = Constant::get(it - _basicBlock.begin() +
                                   _basicBlock.firstInstrIdx());
        m_stack->push(value);
        // // llvm::Instruction *Inst = llvm::cast<llvm::Instruction>(value);
        // // Inst->setMetadata(c_pc,
        // llvm::MDNode::get(m_builder.getContext(),
        // llvm::ValueAsMetadata::get(Constant::get(pc))));
        break;
      }
      case Instruction::MSIZE: {
        m_stack->push(m_builder.getInt32(EVM_MEM_SIZE));
        break;
      }
      case Instruction::GAS: {
        llvm::Value *gas = m_builder.getInt64(2300000);
        gas->setName("gas");
        m_stack->push(gas);  // unlimited

        break;
      }
      case Instruction::JUMPDEST: {
        // Add the basic block to the jump table.
        assert(it == _basicBlock.begin() &&
               "JUMPDEST must be the first instruction of a basic block");
        auto jumpTable = llvm::cast<llvm::SwitchInst>(
            m_jumpTableBB->getTerminator());
        jumpTable->addCase(llvm::ConstantInt::get(Type::Int64Ty, 
            _basicBlock.firstInstrIdx()), _basicBlock.llvm());
        break;
      }
      case Instruction::ANY_PUSH: {
        auto value = readPushData(it, _basicBlock.end());
        // std::cerr << "* Push data = " << value.toString(10, false) << "\n";
        m_stack->push(llvm::ConstantInt::get(Type::Int256Ty, value));
        break;
      }
      case Instruction::ANY_DUP: {
        auto index =
            static_cast<size_t>(inst) - static_cast<size_t>(Instruction::DUP1);
        m_stack->dup(index);
        break;
      }
      case Instruction::ANY_SWAP: {
        auto index =
            static_cast<size_t>(inst) - static_cast<size_t>(Instruction::SWAP1);
        m_stack->swap(index);
        break;
      }
      case Instruction::LOG0:
      case Instruction::LOG1:
      case Instruction::LOG2:
      case Instruction::LOG3:
      case Instruction::LOG4: {
        auto mStart = m_stack->pop();
        auto mSize = m_stack->pop();

        uint32_t numTopics = static_cast<uint32_t>(inst) -
                             static_cast<uint32_t>(Instruction::LOG0);
        
        if (numTopics == 1) {
          llvm::Function *SigFn = m_module.getFunction("addBugSet");
          if (!SigFn) SigFn = llvm::Function::Create(
              llvm::FunctionType::get(Type::Void, {Type::Int64Ty}, false),
              llvm::Function::ExternalLinkage, "addBugSet", m_module);
              
#ifdef MAZE_ENABLE
          auto topic0 = m_stack->pop();
          
          if (mStart->getType() != Type::Int64Ty)
            mStart = m_builder.CreateZExtOrTrunc(mStart, Type::Int64Ty);
          auto Offset = m_builder.CreateAdd(mStart, m_builder.getInt64(0x40));

          llvm::Value *ValPtr = m_builder.CreateAlloca(Type::Word);
          m_builder.CreateCall(m_utilsM.getMload(), {m_mem_ptr, Offset, ValPtr});
          llvm::Value *LogString = m_builder.CreateLShr(
            m_builder.CreateLoad(ValPtr), m_builder.getIntN(256, 192)
          );
          LogString = m_builder.CreateTrunc(LogString, Type::Int64Ty);

          m_builder.CreateCall(SigFn, {LogString});
#else
          m_builder.CreateCall(SigFn, {m_builder.CreateZExtOrTrunc(m_stack->pop(), Type::Int64Ty)});
#endif
        } else {
          // auto number = m_builder.getInt32(numTopics);
          for (auto i = 0; i < numTopics; i++) m_stack->pop();
        }
        break;
      }
      case Instruction::CREATE: {
        auto endowment = m_stack->pop();
        auto initOff = m_stack->pop();
        auto initSize = m_stack->pop();
        m_stack->push(Constant::get(0xff));
        break;
      }
      case Instruction::CREATE2: {
        auto endowment = m_stack->pop();
        auto initOff = m_stack->pop();
        auto initSize = m_stack->pop();
        auto salt = m_stack->pop();
        m_stack->push(Constant::get(0xfe));
        break;
      }
      case Instruction::CALL:
      case Instruction::CALLCODE:
      case Instruction::DELEGATECALL:
      case Instruction::STATICCALL: {
        llvm::Value *benefit = NULL;
        auto callGas = m_builder.CreateZExtOrTrunc(m_stack->pop(), Type::Int64Ty);
        auto address = m_builder.CreateZExtOrTrunc(m_stack->pop(), Type::AddressTy);   
        if (inst == Instruction::CALL || inst == Instruction::CALLCODE) {
          benefit = m_builder.CreateZExt(m_stack->pop(), Type::Int256Ty);
        }
    
        auto inOff = m_stack->pop();
        auto inSize = m_stack->pop();
        auto outOff = m_stack->pop();
        auto outSize = m_stack->pop();

        // llvm::Value *Ret = m_builder.getTrue();
        llvm::Value *Ret = m_builder.CreateCall(m_utilsM.fn_transfer(), {});
        // if (benefit) {
        //   llvm::Value *Balance = m_builder.CreateLoad(
        //       m_module.getNamedGlobal(BALANCECALLER_REF));
        //   auto Sufficient = m_builder.CreateICmpUGT(Balance, benefit);

        //   auto NewBalance = m_builder.CreateSelect(Cond,
        //       m_builder.CreateSub(Balance, benefit), Balance);
        //   m_builder.CreateStore(NewBalance, m_module.getNamedGlobal(BALANCECALLER_REF));
        //   Ret = m_builder.CreateSelect(Cond, m_builder.getTrue(), m_builder.getFalse());
        // } else {
        //   Ret = m_builder.getTrue();
        // }

        m_stack->push(Ret);

        llvm::Instruction *Transfer = llvm::cast<llvm::Instruction>(Ret);
        Transfer->setMetadata(c_pc, MetaPC);

        // for mishandle exception oracle
        // Transfer->setMetadata(c_msesan, llvm::MDNode::get(Context, llvm::None));

        // for ether leak oracle
        // if (inst == Instruction::CALL || inst == Instruction::CALLCODE) {
        //   llvm::MDNode *EthLeakN = llvm::MDNode::get(Context, 
        //       {llvm::ValueAsMetadata::get(address), 
        //        llvm::ValueAsMetadata::get(benefit)});
        //   Transfer->setMetadata(c_elksan, EthLeakN);
        // }
  
        // for reentrancy oracle
        // llvm::MDNode *GasN = llvm::MDNode::get(Context, 
            // {llvm::ValueAsMetadata::get(callGas)});
        // Transfer->setMetadata(c_rensan, GasN);
        break;
      }
      case Instruction::RETURN:
      case Instruction::REVERT: {
        llvm::Value *Index = m_stack->pop();
        llvm::Value *Size = m_stack->pop();

        if (std::getenv("EVM_RET")) {
          /* Enable to export EVM returns (single thread) */
          llvm::Value *memSrc = m_builder.CreateGEP(m_mem_ptr, {Index});
          llvm::Value *Length = m_builder.CreateTrunc(Size, Type::Int32Ty);

          llvm::GlobalVariable* GBuff = m_module.getNamedGlobal(DEBUG_BUFFER);
          if (!GBuff) {
            GBuff = new llvm::GlobalVariable(
              m_module, llvm::ArrayType::get(Type::Int8Ty, 4096), false, 
              llvm::GlobalVariable::ExternalLinkage, nullptr, DEBUG_BUFFER, nullptr,
              llvm::GlobalValue::NotThreadLocal, AS::GLOBAL, false);
            GBuff = m_module.getNamedGlobal(DEBUG_BUFFER); 
          }
          llvm::Value *Ptr = m_builder.CreateGEP(GBuff, 
              {m_builder.getInt32(0), m_builder.getInt32(0)});
          m_builder.CreateCall(m_utilsM.getMemcpyGtoH(),
                              {memSrc, Ptr, Length});
          std::cerr << "EVM RET Enabled.\n";
        }
        if (inst == Instruction::REVERT)
          m_builder.CreateBr(m_abortBB);
        else
          m_builder.CreateBr(m_exitBB);
          
        break;
      }
      case Instruction::INVALID: {
        m_builder.CreateBr(m_abortBB);
        break;
      }
      case Instruction::SUICIDE: {
        // exit
        llvm::Value* To = m_stack->pop();
        m_builder.CreateBr(m_exitBB);

        auto Inst = llvm::cast<llvm::Instruction>(To);
        Inst->setMetadata(c_sucsan, llvm::MDNode::get(Context, llvm::None));
        Inst->setMetadata(c_pc, MetaPC);

        break;
      }
      default:  
        // Invalid instruction - abort
        std::cerr << "INVALID Opcode: " << std::hex << static_cast<size_t>(inst) 
                  << "; translate to an exit.\n";
        m_builder.CreateBr(m_abortBB);
        // m_builder.CreateUnreachable();
        it = _basicBlock.end() - 1;  // finish block compilation
        // invalid = true;
        break;
    }
    // if (invalid)  break;
  }
  
  // label the basic blocks built from EVM bytecode
  llvm::MDNode *BBNode = llvm::MDNode::get(Context, llvm::MDString::get(Context, "BB"));
  auto Terminator = _basicBlock.llvm()->getTerminator();
  if (Terminator) Terminator->setMetadata(c_EVMBB, BBNode);
  
  std::vector<llvm::ConstantInt *> nextBlockIdxs;
  return nextBlockIdxs;
  }
}  // namespace trans
}  // namespace eth
}  // namespace dev
