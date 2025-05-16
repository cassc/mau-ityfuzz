#include "Arith256.h"

namespace sema = dev::eth::trans;

namespace arith {
namespace {

/* example:
    ctlz.Entry:
    %x = load i256, i256* %_X_ptr
    br label %ctlz.LoopHeader

    ctlz.LoopHeader:                                  ; preds = %ctlz.LoopBody,
    %ctlz.Entry
    %tmp = phi i256 [ %x, %ctlz.Entry ], [ %t1, %ctlz.LoopBody ]
    %cnt = phi i256 [ 0, %ctlz.Entry ], [ %c1, %ctlz.LoopBody ]
    %tmp.nonzero = icmp ne i256 %tmp, 0
    br i1 %tmp.nonzero, label %ctlz.LoopBody, label %ctlz.Return

    ctlz.LoopBody:                                    ; preds = %ctlz.LoopHeader
    %t1 = lshr i256 %tmp, 1
    %c1 = add i256 %cnt, 1
    br label %ctlz.LoopHeader

    ctlz.Return:                                      ; preds = %ctlz.LoopHeader
    %0 = sub i256 256, %cnt
    %1 = trunc i256 %0 to i32
    ret i32 %1
*

llvm::Function* createCTLZFunc(llvm::Type* _type, llvm::Module& _module) {
  assert(_type == sema::Type::Word);
  auto func = llvm::Function::Create(
      llvm::FunctionType::get(sema::Type::Int32Ty, {sema::Type::WordPtr},
                              false),
      llvm::Function::InternalLinkage, "ctlz.i256", &_module);
  func->setDoesNotThrow();

  llvm::Argument* _X_ptr = &(*func->arg_begin());
  _X_ptr->setName("_X_ptr");

  llvm::Function* Func_ctlz_i64 = llvm::Intrinsic::getDeclaration(
      &_module, llvm::Intrinsic::ctlz, {sema::Type::Int64Ty});

  auto entryBB = llvm::BasicBlock::Create(_module.getContext(), "Entry", func);

  llvm::IRBuilder<> builder(entryBB);
  builder.SetInsertPoint(entryBB);

  llvm::Value* p64 = builder.CreateBitCast(_X_ptr, sema::Type::Int64PtrTy);
  llvm::Value* v64 =
      builder.CreateLoad(builder.CreateGEP(p64, {builder.getInt64(0)}));
  llvm::Value* cnt =
      builder.CreateCall(Func_ctlz_i64, {v64, builder.getInt1(0)}, "cnt");

  v64 =
      builder.CreateLoad(builder.CreateGEP(p64, {builder.getInt64(1)}));
  cnt = builder.CreateAdd(
      cnt, builder.CreateCall(Func_ctlz_i64, {v64, builder.getInt1(0)}));

  v64 =
      builder.CreateLoad(builder.CreateGEP(p64, {builder.getInt64(2)}));
  cnt = builder.CreateAdd(
      cnt, builder.CreateCall(Func_ctlz_i64, {v64, builder.getInt1(0)}));

  v64 =
      builder.CreateLoad(builder.CreateGEP(p64, {builder.getInt64(3)}));
  cnt = builder.CreateAdd(
      cnt, builder.CreateCall(Func_ctlz_i64, {v64, builder.getInt1(0)}));

  builder.CreateRet(builder.CreateTrunc(cnt, sema::Type::Int32Ty));

  return func;
}
*/

llvm::Function* createUDivRemFunc(llvm::Type* _type, llvm::Module& _module,
                                  char const* _funcName) {
  // Based of "Improved shift divisor algorithm" from "Software Integer
  // Division" by Microsoft Research The following algorithm also handles
  // divisor of value 0 returning 0 for both quotient and remainder

  auto func = llvm::Function::Create(
      llvm::FunctionType::get(sema::Type::Void,
                              {sema::Type::WordPtr, sema::Type::WordPtr,
                               sema::Type::WordPtr, sema::Type::WordPtr},
                              false),
      llvm::Function::InternalLinkage, _funcName, &_module);
  func->setDoesNotThrow();
  func->setDoesNotAccessMemory();

  auto zero = llvm::ConstantInt::get(_type, 0);
  auto one = llvm::ConstantInt::get(_type, 1);

  auto iter = func->arg_begin();
  llvm::Argument* X_ptr = &(*iter++);
  X_ptr->setName("x_ptr");
  llvm::Argument* Y_ptr = &(*iter++);
  Y_ptr->setName("y_ptr");
  llvm::Argument* Q_ptr = &(*iter++);
  Q_ptr->setName("q_Ptr");
  llvm::Argument* R_ptr = &(*iter);
  R_ptr->setName("r_ptr");

  auto entryBB = llvm::BasicBlock::Create(_module.getContext(), "Entry", func);
  auto mainBB = llvm::BasicBlock::Create(_module.getContext(), "Main", func);
  auto beforeloopYBB =
      llvm::BasicBlock::Create(_module.getContext(), "beforeloopY", func);
  auto loopYBB = llvm::BasicBlock::Create(_module.getContext(), "LoopY", func);
  auto loopBB = llvm::BasicBlock::Create(_module.getContext(), "Loop", func);
  auto continueBB =
      llvm::BasicBlock::Create(_module.getContext(), "Continue", func);
  auto returnBB =
      llvm::BasicBlock::Create(_module.getContext(), "Return", func);

  llvm::IRBuilder<> builder(entryBB);
  llvm::Value* x = builder.CreateLoad(X_ptr);
  llvm::Value* y = builder.CreateLoad(Y_ptr);
  auto yLEx = builder.CreateICmpULE(y, x);
  auto r0 = x;
  builder.CreateCondBr(yLEx, mainBB, returnBB);

  builder.SetInsertPoint(mainBB);
  // auto ctlzIntr = llvm::Intrinsic::getDeclaration(&_module,
  // llvm::Intrinsic::ctlz, _type); auto ctlzIntr = createCTLZFunc(_type,
  // _module);

  llvm::Function* ctlzIntr = llvm::Intrinsic::getDeclaration(
      &_module, llvm::Intrinsic::ctlz, {sema::Type::Int256Ty});

  // both y and r are non-zero
  // auto yLz = builder.CreateCall(ctlzIntr, {y, builder.getInt1(true)},
  // "y.lz"); auto rLz = builder.CreateCall(ctlzIntr, {r0,
  // builder.getInt1(true)}, "r.lz");

  // auto _py = builder.CreateAlloca(sema::Type::WordPtr);
  // builder.CreateStore(y, _py);
  auto yLz = builder.CreateCall(ctlzIntr, {y, builder.getInt1(0)}, "y.lz");

  // auto _pr0 = builder.CreateAlloca(sema::Type::WordPtr);
  // builder.CreateStore(r0, _pr0);
  auto rLz = builder.CreateCall(ctlzIntr, {x, builder.getInt1(0)}, "r.lz");

  // auto i0 = builder.CreateZExt(builder.CreateNUWSub(yLz, rLz, "i0"),
  // sema::Type::Word); //i32
  auto i0 = builder.CreateNUWSub(yLz, rLz, "i0");
  // auto y0 = builder.CreateShl(y, i0);
  // builder.CreateBr(loopBB);
  builder.CreateBr(beforeloopYBB);

  builder.SetInsertPoint(beforeloopYBB);
  auto i0Phi = builder.CreatePHI(_type, 2, "i0.phi");
  auto y0 = builder.CreatePHI(_type, 2, "y0");
  auto i0NonZero = builder.CreateICmpNE(i0Phi, zero, "i0.nonzero");
  builder.CreateCondBr(i0NonZero, loopYBB, loopBB);

  builder.SetInsertPoint(loopYBB);
  auto y1 = builder.CreateShl(y0, one);
  auto i1 = builder.CreateSub(i0Phi, one);
  builder.CreateBr(beforeloopYBB);

  y0->addIncoming(y, mainBB);
  y0->addIncoming(y1, loopYBB);
  i0Phi->addIncoming(i0, mainBB);
  i0Phi->addIncoming(i1, loopYBB);

  builder.SetInsertPoint(loopBB);
  auto yPhi = builder.CreatePHI(_type, 2, "y.phi");
  auto rPhi = builder.CreatePHI(_type, 2, "r.phi");
  auto iPhi = builder.CreatePHI(_type, 2, "i.phi");
  auto qPhi = builder.CreatePHI(_type, 2, "q.phi");
  auto rUpdate = builder.CreateNUWSub(rPhi, yPhi);
  auto qUpdate = builder.CreateOr(qPhi, one);  // q += 1, q lowest bit is 0
  auto rGEy = builder.CreateICmpUGE(rPhi, yPhi);
  auto r1 = builder.CreateSelect(rGEy, rUpdate, rPhi, "r1");
  auto q1 = builder.CreateSelect(rGEy, qUpdate, qPhi, "q");
  auto iZero = builder.CreateICmpEQ(iPhi, zero);
  builder.CreateCondBr(iZero, returnBB, continueBB);

  builder.SetInsertPoint(continueBB);
  auto i2 = builder.CreateNUWSub(iPhi, one);
  auto q2 = builder.CreateShl(q1, one);
  auto y2 = builder.CreateLShr(yPhi, one);
  builder.CreateBr(loopBB);

  yPhi->addIncoming(y0, beforeloopYBB);
  yPhi->addIncoming(y2, continueBB);
  rPhi->addIncoming(r0, beforeloopYBB);
  rPhi->addIncoming(r1, continueBB);
  iPhi->addIncoming(i0, beforeloopYBB);
  iPhi->addIncoming(i2, continueBB);
  qPhi->addIncoming(zero, beforeloopYBB);
  qPhi->addIncoming(q2, continueBB);

  builder.SetInsertPoint(returnBB);
  auto qRet = builder.CreatePHI(_type, 2, "q.ret");
  qRet->addIncoming(zero, entryBB);
  qRet->addIncoming(q1, loopBB);
  auto rRet = builder.CreatePHI(_type, 2, "r.ret");
  rRet->addIncoming(r0, entryBB);
  rRet->addIncoming(r1, loopBB);

  builder.CreateStore(qRet, Q_ptr);
  builder.CreateStore(rRet, R_ptr);
  builder.CreateRetVoid();
  // auto ret = builder.CreateInsertElement(llvm::UndefValue::get(retType),
  // qRet, uint64_t(0), "ret0"); ret = builder.CreateInsertElement(ret, rRet, 1,
  // "ret"); builder.CreateRet(ret);
  return func;
}

}  // namespace

llvm::Function* getUDivRem256Func(llvm::Module& _module) {
  static const auto funcName = "evm.udivrem.i256";
  if (auto func = _module.getFunction(funcName)) return func;

  return createUDivRemFunc(sema::Type::Word, _module, funcName);
}

// llvm::Function* getUDivRem512Func(llvm::Module& _module)
// {
// 	static const auto funcName = "evm.udivrem.i512";
// 	if (auto func = _module.getFunction(funcName))
// 		return func;

// 	return createUDivRemFunc(llvm::IntegerType::get(_module.getContext(),
// 512), _module, funcName);
// }

llvm::Function* getUDiv256Func(llvm::Module& _module) {
  static const auto funcName = "evm.udiv.i256";
  if (auto func = _module.getFunction(funcName)) return func;

  auto udivremFunc = getUDivRem256Func(_module);

  auto func = llvm::Function::Create(
      llvm::FunctionType::get(
          sema::Type::Void,
          {sema::Type::WordPtr, sema::Type::WordPtr, sema::Type::WordPtr},
          false),
      llvm::Function::InternalLinkage, funcName, &_module);
  func->setDoesNotThrow();
  // func->setDoesNotAccessMemory();

  // x / y = q ... r
  auto iter = func->arg_begin();
  llvm::Argument* X = &(*iter++);
  X->setName("_x_ptr");
  llvm::Argument* Y = &(*iter++);
  Y->setName("_y_ptr");
  llvm::Argument* Q = &(*iter);
  Q->setName("_q_ptr");
  // llvm::Argument* R = &(*iter);
  // R->setName("_r_ptr");

  auto bb = llvm::BasicBlock::Create(_module.getContext(), {}, func);
  llvm::IRBuilder<> builder(bb);
  // auto udivrem =
  auto _ = builder.CreateAlloca(sema::Type::Word);
  builder.CreateCall(udivremFunc, {X, Y, Q, _});
  // auto udiv = builder.CreateExtractElement(udivrem, uint64_t(0));
  // builder.CreateRet(udiv);
  builder.CreateRetVoid();

  return func;
}

llvm::Function* getMulFunc(llvm::Module& _module, uint64_t BitWidth,
                           const std::string funcName) {
  llvm::Type* IntHiTy =
      BitWidth == 256 ? sema::Type::Word : sema::Type::Int128Ty;
  llvm::Type* IntLoTy =
      BitWidth == 256 ? sema::Type::Int128Ty : sema::Type::Int64Ty;

  auto func = llvm::Function::Create(
      llvm::FunctionType::get(sema::Type::Void, {sema::Type::WordPtr, 
      sema::Type::WordPtr, sema::Type::WordPtr}, false),
      llvm::Function::InternalLinkage, funcName, &_module);
  func->setDoesNotThrow();

  llvm::Argument* RETPtr = func->arg_begin();
  RETPtr->setName("retptr");
  llvm::Argument* LHSPtr = RETPtr + 1;
  LHSPtr->setName("lhsptr");
  llvm::Argument* RHSPtr = LHSPtr + 1;
  RHSPtr->setName("rhsptr");


  auto entryBB =
      llvm::BasicBlock::Create(_module.getContext(), "mul.Entry", func);
  llvm::IRBuilder<> Builder(entryBB);

  llvm::Value *LHS = Builder.CreateLoad(sema::Type::Word, LHSPtr, "lhs");
  llvm::Value *RHS = Builder.CreateLoad(sema::Type::Word, RHSPtr, "rhs");

  const uint64_t Shift = BitWidth / 2;
  const llvm::APInt HalfMask =
      llvm::APInt::getLowBitsSet(BitWidth / 2, BitWidth / 4);
  const uint64_t HalfShift = BitWidth / 4;

  llvm::Value* LHS_L = Builder.CreateTrunc(LHS, IntLoTy, "lhs_l");
  llvm::Value* LHS_H =
      Builder.CreateTrunc(Builder.CreateLShr(LHS, Shift), IntLoTy, "lhs_h");
  llvm::Value* RHS_L = Builder.CreateTrunc(RHS, IntLoTy, "rhs_l");
  llvm::Value* RHS_H =
      Builder.CreateTrunc(Builder.CreateLShr(RHS, Shift), IntLoTy, "rhs_h");

  llvm::Value* LHS_LL = Builder.CreateAnd(LHS_L, HalfMask, "lhs_ll");
  llvm::Value* RHS_LL = Builder.CreateAnd(RHS_L, HalfMask, "rhs_ll");
  llvm::Value* T = Builder.CreateMul(LHS_LL, RHS_LL, "t");

  llvm::Value* T_L = Builder.CreateAnd(T, HalfMask, "t_l");
  llvm::Value* T_H = Builder.CreateLShr(T, HalfShift, "t_h");

  llvm::Value* LHS_LH = Builder.CreateLShr(LHS_L, HalfShift, "lhs_lh");
  llvm::Value* RHS_LH = Builder.CreateLShr(RHS_L, HalfShift, "rhs_lh");

  llvm::Value* U =
      Builder.CreateAdd(Builder.CreateMul(LHS_LH, RHS_LL), T_H, "u");
  llvm::Value* U_L = Builder.CreateAnd(U, HalfMask, "u_l");
  llvm::Value* U_H = Builder.CreateLShr(U, HalfShift, "u_h");

  llvm::Value* V =
      Builder.CreateAdd(Builder.CreateMul(LHS_LL, RHS_LH), U_L, "v");
  llvm::Value* V_H = Builder.CreateLShr(V, HalfShift, "v_h");

  llvm::Value* W = Builder.CreateAdd(Builder.CreateMul(LHS_LH, RHS_LH),
                                     Builder.CreateAdd(U_H, V_H), "w");

  llvm::Value* O_L = Builder.CreateZExt(
      Builder.CreateAdd(T_L, Builder.CreateShl(V, HalfShift)), IntHiTy, "o_l");
  llvm::Value* O_H = Builder.CreateZExt(
      Builder.CreateAdd(W, Builder.CreateAdd(Builder.CreateMul(RHS_H, LHS_L),
                                             Builder.CreateMul(RHS_L, LHS_H))),
      IntHiTy, "o_h");

  llvm::Value* O = Builder.CreateAdd(O_L, Builder.CreateShl(O_H, Shift), "o");
  Builder.CreateStore(O, RETPtr);
//   Builder.CreateRet(O);
    Builder.CreateRetVoid();

  return func;
}

llvm::Function* getMul256Func(llvm::Module& _module) {
  static const auto funcName = "evm.mul.i256";
  if (auto func = _module.getFunction(funcName)) return func;
  return getMulFunc(_module, 256, funcName);
}

llvm::Function* getMul128Func(llvm::Module& _module) {
  static const auto funcName = "evm.mul.i128";
  if (auto func = _module.getFunction(funcName)) return func;
  return getMulFunc(_module, 128, funcName);
}

namespace {
llvm::Function* createURemFunc(llvm::Type* _type, llvm::Module& _module,
                               char const* _funcName) {
  // auto udivremFunc = _type == sema::Type::Word ?
  // getUDivRem256Func(_module) :
  // getUDivRem512Func(_module);
  assert(_type == sema::Type::Word);
  auto udivremFunc = getUDivRem256Func(_module);
  // auto udivremFunc = getUDivRem256Func(_module);
  auto func = llvm::Function::Create(
      llvm::FunctionType::get(_type, {_type, _type}, false),
      llvm::Function::InternalLinkage, _funcName, &_module);
  func->setDoesNotThrow();
  func->setDoesNotAccessMemory();

  auto iter = func->arg_begin();
  llvm::Argument* x = &(*iter++);
  x->setName("x");
  llvm::Argument* y = &(*iter);
  y->setName("y");

  auto bb = llvm::BasicBlock::Create(_module.getContext(), {}, func);
  llvm::IRBuilder<> builder(bb);
  auto udivrem = builder.CreateCall(udivremFunc, {x, y});
  auto r = builder.CreateExtractElement(udivrem, uint64_t(1));
  builder.CreateRet(r);

  return func;
}
}  // namespace

llvm::Function* getURem256Func(llvm::Module& _module) {
  static const auto funcName = "evm.urem.i256";
  if (auto func = _module.getFunction(funcName)) return func;
  return createURemFunc(sema::Type::Word, _module, funcName);
}

// llvm::Function* getURem512Func(llvm::Module& _module)
// {
// 	static const auto funcName = "evm.urem.i512";
// 	if (auto func = _module.getFunction(funcName))
// 		return func;
// 	return createURemFunc(llvm::IntegerType::get(_module.getContext(), 512),
// _module, funcName);
// }

llvm::Function* getSDivRem256Func(llvm::Module& _module) {
  static const auto funcName = "evm.sdivrem.i256";
  if (auto func = _module.getFunction(funcName)) return func;

  auto udivremFunc = getUDivRem256Func(_module);

  auto func = llvm::Function::Create(
      llvm::FunctionType::get(sema::Type::Void,
                              {sema::Type::WordPtr, sema::Type::WordPtr,
                               sema::Type::WordPtr, sema::Type::WordPtr},
                              false),
      llvm::Function::InternalLinkage, funcName, &_module);
  func->setDoesNotThrow();

  auto iter = func->arg_begin();
  llvm::Argument* X_ptr = &(*iter++);
  X_ptr->setName("x_ptr");
  llvm::Argument* Y_ptr = &(*iter++);
  Y_ptr->setName("y_ptr");
  llvm::Argument* Q_ptr = &(*iter++);
  Q_ptr->setName("q_Ptr");
  llvm::Argument* R_ptr = &(*iter);
  R_ptr->setName("r_ptr");

  auto bb = llvm::BasicBlock::Create(_module.getContext(), "", func);
  llvm::IRBuilder<> builder(bb);

  llvm::Value* Zero256 = builder.getIntN(256, 0);
  llvm::Value* x = builder.CreateLoad(X_ptr);
  llvm::Value* y = builder.CreateLoad(Y_ptr);
  auto xIsNeg = builder.CreateICmpSLT(x, Zero256);
  auto xNeg = builder.CreateSub(Zero256, x);
  auto xAbs = builder.CreateSelect(xIsNeg, xNeg, x);

  auto yIsNeg = builder.CreateICmpSLT(y, Zero256);
  auto yNeg = builder.CreateSub(Zero256, y);
  auto yAbs = builder.CreateSelect(yIsNeg, yNeg, y);

  auto pxabs = builder.CreateAlloca(sema::Type::Word);
  builder.CreateStore(xAbs, pxabs);
  auto pyabs = builder.CreateAlloca(sema::Type::Word);
  builder.CreateStore(yAbs, pyabs);
  auto pq = builder.CreateAlloca(sema::Type::Word);
  auto pr = builder.CreateAlloca(sema::Type::Word);
  builder.CreateCall(udivremFunc, {pxabs, pyabs, pq, pr});
  auto qAbs = builder.CreateLoad(pq);
  auto rAbs = builder.CreateLoad(pr);
  // auto res = builder.CreateCall(udivremFunc, {xAbs, yAbs});

  // auto qAbs = builder.CreateExtractElement(res, uint64_t(0));
  // auto rAbs = builder.CreateExtractElement(res, 1);

  // the remainder has the same sign as dividend
  auto rNeg = builder.CreateSub(Zero256, rAbs);
  auto r = builder.CreateSelect(xIsNeg, rNeg, rAbs);

  auto qNeg = builder.CreateSub(Zero256, qAbs);
  auto xyOpposite = builder.CreateXor(xIsNeg, yIsNeg);
  auto q = builder.CreateSelect(xyOpposite, qNeg, qAbs);

  // auto ret = builder.CreateInsertElement(llvm::UndefValue::get(retType), q,
  // uint64_t(0)); ret = builder.CreateInsertElement(ret, r, 1);
  // builder.CreateRet(ret);
  builder.CreateStore(q, Q_ptr);
  builder.CreateStore(r, R_ptr);
  builder.CreateRetVoid();

  return func;
}

llvm::Function* getSDiv256Func(llvm::Module& _module) {
  static const auto funcName = "evm.sdiv.i256";
  if (auto func = _module.getFunction(funcName)) return func;

  auto sdivremFunc = getSDivRem256Func(_module);

  // auto func =
  // llvm::Function::Create(llvm::FunctionType::get(sema::Type::Word,
  // {sema::Type::Word, sema::Type::Word}, false),
  // llvm::Function::InternalLinkage, funcName, &_module);
  auto func = llvm::Function::Create(
      llvm::FunctionType::get(
          sema::Type::Void,
          {sema::Type::WordPtr, sema::Type::WordPtr, sema::Type::WordPtr},
          false),
      llvm::Function::InternalLinkage, funcName, &_module);
  func->setDoesNotThrow();
  // func->setDoesNotAccessMemory();

  auto iter = func->arg_begin();
  llvm::Argument* X = &(*iter++);
  X->setName("_x_ptr");
  llvm::Argument* Y = &(*iter++);
  Y->setName("_y_ptr");
  llvm::Argument* Q = &(*iter);
  Q->setName("_q_ptr");

  auto bb = llvm::BasicBlock::Create(_module.getContext(), {}, func);
  llvm::IRBuilder<> builder(bb);
  // auto sdivrem = builder.CreateCall(sdivremFunc, {x, y});
  // auto q = builder.CreateExtractElement(sdivrem, uint64_t(0));
  // builder.CreateRet(q);
  auto _ = builder.CreateAlloca(sema::Type::Word);
  builder.CreateCall(sdivremFunc, {X, Y, Q, _});
  builder.CreateRetVoid();

  return func;
}

llvm::Function* getSRem256Func(llvm::Module& _module) {
  static const auto funcName = "evm.srem.i256";
  if (auto func = _module.getFunction(funcName)) return func;

  auto sdivremFunc = getSDivRem256Func(_module);

  auto func = llvm::Function::Create(
      llvm::FunctionType::get(sema::Type::Word,
                              {sema::Type::Word, sema::Type::Word}, false),
      llvm::Function::InternalLinkage, funcName, &_module);
  func->setDoesNotThrow();
  func->setDoesNotAccessMemory();

  auto iter = func->arg_begin();
  llvm::Argument* x = &(*iter++);
  x->setName("x");
  llvm::Argument* y = &(*iter);
  y->setName("y");

  auto bb = llvm::BasicBlock::Create(_module.getContext(), {}, func);
  llvm::IRBuilder<> builder(bb);
  auto sdivrem = builder.CreateCall(sdivremFunc, {x, y});
  auto r = builder.CreateExtractElement(sdivrem, uint64_t(1));
  builder.CreateRet(r);

  return func;
}

llvm::Function* getExpFunc(llvm::Module& _module) {
  static const auto funcName =  "__power_word";
  llvm::Function *func = _module.getFunction(funcName);
  if (func != nullptr) return func;
  func = llvm::Function::Create(
      llvm::FunctionType::get(sema::Type::Void, {sema::Type::WordPtr,
        sema::Type::WordPtr, sema::Type::WordPtr}, false),
      llvm::Function::ExternalLinkage, funcName, _module);
  return func;
}

// llvm::Function* getExpFunc(llvm::Module& _module) {
//   static const auto funcName = "evm.exp";
//   if (auto m_exp = _module.getFunction(funcName)) return m_exp;

//   llvm::Type* argTypes[] = {sema::Type::WordPtr, sema::Type::WordPtr,
//                             sema::Type::WordPtr};
//   auto m_exp = llvm::Function::Create(
//       llvm::FunctionType::get(sema::Type::Void, argTypes, false),
//       llvm::Function::InternalLinkage, "evm.exp", _module);
//   m_exp->setDoesNotThrow();

//   auto iter = m_exp->arg_begin();
//   llvm::Argument* pbase = &(*iter);
//   pbase->setName("pbase");
//   llvm::Argument* pexponent = pbase + 1;
//   pexponent->setName("pexponent");
//   llvm::Argument* pret = pexponent + 1;
//   pret->setName("pret");

//   //	while (e != 0) {
//   //		if (e % 2 == 1)
//   //			r *= b;
//   //		b *= b;
//   //		e /= 2;
//   //	}

//   llvm::LLVMContext& context = _module.getContext();
//   auto entryBB = llvm::BasicBlock::Create(context, "exp.Entry", m_exp);
//   auto headerBB = llvm::BasicBlock::Create(context, "exp.LoopHeader", m_exp);
//   auto bodyBB = llvm::BasicBlock::Create(context, "exp.LoopBody", m_exp);
//   auto updateBB = llvm::BasicBlock::Create(context, "exp.ResultUpdate", m_exp);
//   auto continueBB = llvm::BasicBlock::Create(context, "exp.Continue", m_exp);
//   auto returnBB = llvm::BasicBlock::Create(context, "exp.Return", m_exp);

//   llvm::IRBuilder<> m_builder(entryBB);
//   llvm::Value* base = m_builder.CreateLoad(pbase);
//   llvm::Value* exponent = m_builder.CreateLoad(pexponent);
//   m_builder.CreateBr(headerBB);

//   m_builder.SetInsertPoint(headerBB);
//   auto r = m_builder.CreatePHI(sema::Type::Word, 2, "r");
//   auto b = m_builder.CreatePHI(sema::Type::Word, 2, "b");
//   auto e = m_builder.CreatePHI(sema::Type::Word, 2, "e");
//   auto eNonZero =
//       m_builder.CreateICmpNE(e, m_builder.getIntN(256, 0), "e.nonzero");
//   m_builder.CreateCondBr(eNonZero, bodyBB, returnBB);

//   m_builder.SetInsertPoint(bodyBB);
//   auto eOdd =
//       m_builder.CreateICmpNE(m_builder.CreateAnd(e, m_builder.getIntN(256, 1)),
//                              m_builder.getIntN(256, 0), "e.isodd");
//   m_builder.CreateCondBr(eOdd, updateBB, continueBB);

//   m_builder.SetInsertPoint(updateBB);
//   auto r0 = m_builder.CreateMul(r, b);
//   m_builder.CreateBr(continueBB);

//   m_builder.SetInsertPoint(continueBB);
//   auto r1 = m_builder.CreatePHI(sema::Type::Word, 2, "r1");
//   r1->addIncoming(r, bodyBB);
//   r1->addIncoming(r0, updateBB);
//   auto b1 = m_builder.CreateMul(b, b);
//   auto e1 = m_builder.CreateLShr(e, m_builder.getIntN(256, 1), "e1");
//   m_builder.CreateBr(headerBB);

//   r->addIncoming(m_builder.getIntN(256, 0), entryBB);
//   r->addIncoming(r1, continueBB);
//   b->addIncoming(base, entryBB);
//   b->addIncoming(b1, continueBB);
//   e->addIncoming(exponent, entryBB);
//   e->addIncoming(e1, continueBB);

//   m_builder.SetInsertPoint(returnBB);
//   m_builder.CreateStore(r, pret);
//   m_builder.CreateRetVoid();
//   return m_exp;
// }

}  // namespace arith
