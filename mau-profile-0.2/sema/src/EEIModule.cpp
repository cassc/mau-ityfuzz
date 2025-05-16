// following
// https://github.com/ewasm/design/commit/8946232935822723a4c80bec03eb1b8ecd237d5f
#pragma once

#include "EEIModule.h"

#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>

#include "Type.h"
#include "config.h"
// #include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/raw_ostream.h>

#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"

namespace dev {
namespace eth {
namespace trans {

EEIModule::EEIModule(llvm::Module& _module)
    : VMContext(_module.getContext()) {
      TheModule = &_module;
      Type::init(VMContext);
      
      fn_thread_id();
      fn_update_bits();
      getMutateCalldata();

  // llvm::FunctionType *VprintfFuncType = llvm::FunctionType::get(
  //   Type::Int32Ty, {Type::Int8PtrTy, Type::Int8PtrTy}, false);
  // Func_vprintf = llvm::Function::Create(VprintfFuncType, 
  //   llvm::GlobalVariable::ExternalLinkage, "vprintf", TheModule);
}


llvm::Function *EEIModule::fn_thread_id() {
  static const auto funcName = CUTHREAD_FUNC;
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::Int32Ty, {}, false),
      llvm::Function::InternalLinkage, funcName, TheModule);

  llvm::BasicBlock *BB = llvm::BasicBlock::Create(VMContext, "", func);
  llvm::IRBuilder<> IRB(BB);

  auto sregFuncTy = llvm::FunctionType::get(Type::Int32Ty, {}, false);
  llvm::Function *tidFunc =
      llvm::Function::Create(sregFuncTy, llvm::Function::ExternalLinkage,
                             "llvm.nvvm.read.ptx.sreg.tid.x", TheModule);
  llvm::Function *bidFunc =
      llvm::Function::Create(sregFuncTy, llvm::Function::ExternalLinkage,
                             "llvm.nvvm.read.ptx.sreg.ctaid.x", TheModule);
  llvm::Function *bdimFunc =
      llvm::Function::Create(sregFuncTy, llvm::Function::ExternalLinkage,
                             "llvm.nvvm.read.ptx.sreg.ntid.x", TheModule);

  llvm::Value *_tidx  = IRB.CreateCall(tidFunc, {});
  llvm::Value *_bidx  = IRB.CreateCall(bidFunc, {});
  llvm::Value *_bdimx = IRB.CreateCall(bdimFunc, {});

  auto ret = IRB.CreateAdd(_tidx, IRB.CreateMul(_bidx, _bdimx));
  IRB.CreateRet(ret);

  return func;
}

llvm::Function *EEIModule::fn_sync_warp() {
  static const auto funcName = "llvm.nvvm.bar.warp.sync";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::Void, {Type::Int32Ty}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

llvm::Function *EEIModule::fn_malloc() {
  
  static const auto funcName = "malloc";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::BytePtr, {Type::Int64Ty}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  func->setDoesNotThrow();
  func->setReturnDoesNotAlias();
  return func;
}

llvm::Function *EEIModule::fn_free() {
  
  static const auto funcName = "free";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::Void, {Type::Int8PtrTy}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  func->setDoesNotThrow();
  func->getArg(0)->addAttr(llvm::Attribute::NoCapture);
  return func;
}

llvm::Function *EEIModule::fn_transfer() {
  static const auto funcName = "solidity_call";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::Bool, {}, false),
      llvm::Function::InternalLinkage, funcName, TheModule);
  func->setDoesNotThrow();
  llvm::BasicBlock *Entry = llvm::BasicBlock::Create(VMContext, "entry", func);
  llvm::IRBuilder<> IRB(Entry);
  IRB.CreateRet(IRB.getTrue());
  return func;
}

llvm::Function *EEIModule::getStorageStore() {
  static const auto funcName = "__device_sstore";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::Void, {Type::WordPtr, Type::WordPtr}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

llvm::Function *EEIModule::getStorageLoad() {
  static const auto funcName = "__device_sload";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(Type::Void, {Type::WordPtr, Type::WordPtr}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

 
//  llvm::Function *EEIModule::fn_memset() {
//   static const auto funcName = "__device_memset";
//   llvm::Function *func = TheModule->getFunction(funcName);
//   if (func != nullptr) return func;
//   func = llvm::Function::Create(llvm::FunctionType::get(Type::Void,
//       {Type::Int8PtrTy, Type::Int64Ty, Type::BytePtr, Type::Int64Ty}, false),
//       llvm::Function::ExternalLinkage, funcName, TheModule);
//   return func;
// }


llvm::Function *EEIModule::getMstore() {
  // static const auto funcName = "device.mstore";
  // llvm::Function *func = TheModule->getFunction(funcName);
  // if (func != nullptr) return func;
  // func = llvm::Function::Create(
  //     llvm::FunctionType::get(Type::Void, {Type::Int8PtrTy, Type::Int8PtrTy},
  //                             false),
  //     llvm::Function::InternalLinkage, funcName, TheModule);
  // func->addFnAttr(llvm::Attribute::NoUnwind);
  // llvm::Argument *const Dst = func->arg_begin();
  // Dst->setName("memory_dst");
  // llvm::Argument *const Src = Dst + 1;
  // Src->setName("value_src");

  // llvm::BasicBlock *Entry = llvm::BasicBlock::Create(VMContext, "entry", func);
  // llvm::IRBuilder<> IRB(Entry);

  // for (size_t offset = 0; offset < 32; ++offset) {
  //   llvm::Value *v8 = IRB.CreateLoad(Type::Byte, 
  //       IRB.CreateGEP(Src, {IRB.getInt32(offset)}));
  //   IRB.CreateStore(v8, IRB.CreateGEP(Dst, {IRB.getInt32(31 - offset)}));
  // }
  // IRB.CreateRetVoid();
  // return func;
  static const auto funcName = "__device_mstore";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;
  func = llvm::Function::Create(llvm::FunctionType::get(Type::Void,
      {Type::Int8PtrTy, Type::Int64Ty, Type::BytePtr, Type::Int64Ty}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

llvm::Function *EEIModule::getMload() {
  // static const auto funcName = "device.mload";
  // llvm::Function *func = TheModule->getFunction(funcName);
  // if (func != nullptr) return func;

  // func = llvm::Function::Create(
  //     llvm::FunctionType::get(Type::Void, {Type::Int8PtrTy, Type::Int256PtrTy},
  //                             false),
  //     llvm::Function::InternalLinkage, funcName, TheModule);
  // func->addFnAttr(llvm::Attribute::NoUnwind);
  // llvm::Argument *const Src = func->arg_begin();
  // Src->setName("memory_src");
  // llvm::Argument *const Dst = Src + 1;
  // Dst->setName("value.dst");

  // llvm::BasicBlock *Entry = llvm::BasicBlock::Create(VMContext, "entry", func);
  // llvm::IRBuilder<> IRB(Entry);

  // auto Dst8 = IRB.CreateBitCast(Dst, Type::BytePtr);
  // for (size_t offset = 0; offset < 32; ++offset) {
  //   llvm::Value *v8 =
  //       IRB.CreateLoad(Type::Byte, IRB.CreateGEP(Src, {IRB.getInt32(offset)}));
  //   IRB.CreateStore(v8, IRB.CreateGEP(Dst8, {IRB.getInt32(31 - offset)}));
  // }
  // IRB.CreateRetVoid();
  // return func;
  static const auto funcName = "__device_mload";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;
  func = llvm::Function::Create(llvm::FunctionType::get(Type::Void,
      {Type::Int8PtrTy, Type::Int64Ty, Type::WordPtr}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

llvm::Function *EEIModule::getMemcpy() {
  static const auto funcName = "memcpy";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
          Type::Void, {Type::Int8PtrTy, Type::Int8PtrTy, Type::Int32Ty}, false),
      llvm::Function::InternalLinkage, funcName, TheModule);
  func->addFnAttr(llvm::Attribute::NoUnwind);

  llvm::Argument *const Dst = func->arg_begin();
  llvm::Argument *const Src = Dst + 1;
  llvm::Argument *const Length = Src + 1;

  Dst->setName("dst");
  Src->setName("src");
  Length->setName("length");

  llvm::BasicBlock *Entry = llvm::BasicBlock::Create(VMContext, "entry", func);
  llvm::BasicBlock *Loop = llvm::BasicBlock::Create(VMContext, "loop", func);
  llvm::BasicBlock *Return =
      llvm::BasicBlock::Create(VMContext, "return", func);
  ;
  llvm::IRBuilder<> IRB(Entry);
  llvm::Value *Cmp = IRB.CreateICmpNE(Length, IRB.getInt32(0));
  IRB.CreateCondBr(Cmp, Loop, Return);

  IRB.SetInsertPoint(Loop);
  llvm::PHINode *SrcPHI = IRB.CreatePHI(Type::Int8PtrTy, 2);
  llvm::PHINode *DstPHI = IRB.CreatePHI(Type::Int8PtrTy, 2);
  llvm::PHINode *LengthPHI = IRB.CreatePHI(Type::Int32Ty, 2);

  llvm::ConstantInt *const One = IRB.getInt32(1);
  llvm::Value *Value = IRB.CreateLoad(SrcPHI);
  IRB.CreateStore(Value, DstPHI);
  llvm::Value *Src2 = IRB.CreateGEP(SrcPHI, {One});
  llvm::Value *Dst2 = IRB.CreateGEP(DstPHI, {One});
  llvm::Value *Length2 = IRB.CreateSub(LengthPHI, One);
  llvm::Value *Cmp2 = IRB.CreateICmpNE(Length2, IRB.getInt32(0));
  IRB.CreateCondBr(Cmp2, Loop, Return);

  IRB.SetInsertPoint(Return);
  IRB.CreateRetVoid();

  SrcPHI->addIncoming(Src, Entry);
  SrcPHI->addIncoming(Src2, Loop);
  DstPHI->addIncoming(Dst, Entry);
  DstPHI->addIncoming(Dst2, Loop);
  LengthPHI->addIncoming(Length, Entry);
  LengthPHI->addIncoming(Length2, Loop);
  return func;
}

llvm::Function *EEIModule::getMutateCalldata() {
  static const auto funcName = "parallel_mutate";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func) return func;

  func = llvm::Function::Create(llvm::FunctionType::get(Type::Void, 
      {Type::Int8Ty->getPointerTo(AS::GLOBAL), Type::Int32Ty}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  llvm::NamedMDNode *annotation = 
      TheModule->getOrInsertNamedMetadata("nvvm.annotations");
  auto _content = llvm::MDTuple::get(VMContext, {
      llvm::ValueAsMetadata::get(func), llvm::MDString::get(VMContext, "kernel"),
      llvm::ValueAsMetadata::getConstant(llvm::ConstantInt::get(Type::Int32Ty, 1))});
  annotation->addOperand(_content);
  return func;
}

llvm::Function *EEIModule::getCalldataCopy() {
  static const auto funcName = "__device_calldatacpy";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
          Type::Void,
          {Type::BytePtr, Type::Int64Ty, Type::Int8PtrTy, Type::Int64Ty, Type::Int64Ty},
          false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

llvm::Function *EEIModule::getCalldataLoad() {
  static const auto funcName = "__device_calldataload";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
          Type::Void,
          {Type::Int8PtrTy, Type::Int8PtrTy, Type::Int64Ty},
          false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}



llvm::Function *EEIModule::getMemcpyGtoH() {
  // data in address(1) comes from Host, packed by Web3py in big-endianness
  // , but the data in address(0) is stored in little-endianness.
  // Thus, we should reverse the source data and then store to the destination
  // in big-endianness.

  /*
      for (u32 index = length; index !=0; -- index)
          p_src = length - index
          v_src = _src[p_src]

          p_dst = index - 1
          _dst[p_dst] = v_src
  */
  static const auto funcName = "device.memcpyGtoH";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
          Type::Void, {Type::Int8PtrTy, Type::Int8Ptr1Ty, Type::Int32Ty},
          false),
      llvm::Function::InternalLinkage, funcName, TheModule);
  func->addFnAttr(llvm::Attribute::NoUnwind);

  llvm::Argument *const GSrc = func->arg_begin();
  llvm::Argument *const HDst = GSrc + 1;
  llvm::Argument *const Length = GSrc + 2;
  GSrc->setName("_src");
  HDst->setName("_dst");
  Length->setName("_length");

  llvm::BasicBlock *Entry = llvm::BasicBlock::Create(VMContext, "entry", func);
  llvm::BasicBlock *Loop = llvm::BasicBlock::Create(VMContext, "loop", func);
  llvm::BasicBlock *Return =
      llvm::BasicBlock::Create(VMContext, "return", func);

  // entry.
  llvm::IRBuilder<> IRB(Entry);

  llvm::Value *Cmp = IRB.CreateICmpNE(Length, IRB.getInt32(0));
  IRB.CreateCondBr(Cmp, Loop, Return);

  // loop body.
  IRB.SetInsertPoint(Loop);
  llvm::PHINode *Index_phi = IRB.CreatePHI(Type::Int32Ty, 2);

  llvm::Value *src = IRB.CreateGEP(GSrc, {Index_phi});
  llvm::Value *Value = IRB.CreateLoad(src);
  llvm::Value *dst = IRB.CreateGEP(HDst, {Index_phi});
  IRB.CreateStore(Value, dst);

  llvm::Value *next_idx = IRB.CreateAdd(Index_phi, IRB.getInt32(1));
  llvm::Value *Cmp2 = IRB.CreateICmpULT(next_idx, Length);
  IRB.CreateCondBr(Cmp2, Loop, Return);

  // return.
  IRB.SetInsertPoint(Return);
  IRB.CreateRetVoid();

  Index_phi->addIncoming(IRB.getInt32(0), Entry);
  Index_phi->addIncoming(next_idx, Loop);
  return func;
}

llvm::Function *EEIModule::getHashFunc() {
  static const auto funcName = "__device_sha3";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
          Type::Void, {Type::BytePtr, Type::Int32Ty, Type::BytePtr}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  return func;
}

llvm::Function *EEIModule::fn_clear_bitmap() {
  static const auto funcName = "__clear_bitmap";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
        Type::Void, {Type::Byte->getPointerTo(AS::GLOBAL)},
        false
      ), llvm::Function::ExternalLinkage, funcName, TheModule
  );

  return func;
}

llvm::Function *EEIModule::fn_mem_p1_p0() {
  static const auto funcName = "llvm.nvvm.ptr.global.to.gen.p0i8.p1i8";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
        Type::BytePtr, {Type::Byte->getPointerTo(AS::GLOBAL)},
        false
      ),
      llvm::Function::ExternalLinkage, 
      funcName, 
      TheModule
  );

  return func;
}

llvm::Function *EEIModule::fn_mem_p4_p0() {
  static const auto funcName = "llvm.nvvm.ptr.constant.to.gen.p0i8.p4i8";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
        Type::BytePtr, {Type::Byte->getPointerTo(AS::CONST)},
        false
      ),
      llvm::Function::ExternalLinkage, 
      funcName, 
      TheModule
  );

  return func;
}


llvm::Function *EEIModule::fn_update_bits() {
  static const auto funcName = "updateBits";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func) return func;

  func = llvm::Function::Create(llvm::FunctionType::get(Type::Void, 
      {Type::Int64Ty->getPointerTo(AS::GLOBAL),}, false),
      llvm::Function::ExternalLinkage, funcName, TheModule);
  llvm::NamedMDNode *annotation = 
      TheModule->getOrInsertNamedMetadata("nvvm.annotations");
  auto _content = llvm::MDTuple::get(VMContext, {
      llvm::ValueAsMetadata::get(func), llvm::MDString::get(VMContext, "kernel"),
      llvm::ValueAsMetadata::getConstant(llvm::ConstantInt::get(Type::Int32Ty, 1))});
  annotation->addOperand(_content);

  // llvm::Argument *const CovPtr = func->getArg(0);
  // llvm::Argument *const HnbPtr = func->getArg(1);

  // interal function
  // llvm::Function *inlineFunc = llvm::Function::Create(
  //     llvm::FunctionType::get(Type::Void, {Type::Int64Ty->getPointerTo(AS::GLOBAL), 
  //         Type::Int8Ty->getPointerTo(AS::GLOBAL)}, false),
  //     llvm::Function::ExternalLinkage, 
  //     "__update_bits", 
  //     TheModule
  // );

  // llvm::Function *inlineFunc = llvm::Function::Create(
  //     llvm::FunctionType::get(Type::Void, {
  //       Type::Int32Ty, 
  //       Type::Int8Ty->getPointerTo(AS::GLOBAL),
  //       Type::Int64Ty->getPointerTo(AS::GLOBAL)
  //     }, false),
  //     llvm::Function::ExternalLinkage, 
  //     "__mv_new_bits", 
  //     TheModule
  // );

  // llvm::BasicBlock *BB =
  //     llvm::BasicBlock::Create(VMContext, "", func);
  // llvm::IRBuilder<> IRB(BB);

  // // IRB.CreateCall(inlineFunc, {CovPtr, HnbPtr});
  // llvm::Value *tid = IRB.CreateCall(this->fn_thread_id(), {});
  // IRB.CreateCall(inlineFunc, {tid, HnbPtr, CovPtr});
  // IRB.CreateRetVoid();

  return func;
}

llvm::Function *EEIModule::fn_classify_counts() {
  // the function name should be consistent with the rt.o.c
  static const auto funcName = "__classify_counts";
  llvm::Function *func = TheModule->getFunction(funcName);
  if (func != nullptr) return func;

  func = llvm::Function::Create(
      llvm::FunctionType::get(
        Type::Void, {Type::Int64Ty->getPointerTo(AS::GLOBAL)}, 
        false
      ),
      llvm::Function::ExternalLinkage, 
      funcName, 
      TheModule
  );

  return func;
}

}  // namespace trans
}  // namespace eth
}  // namespace dev