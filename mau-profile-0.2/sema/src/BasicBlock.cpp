#include "BasicBlock.h"

#include <llvm/IR/CFG.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <iostream>

#include "Instruction.h"
#include "Type.h"
#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"
// #include "Utils.h"

namespace dev {
namespace eth {
namespace trans {

BasicBlock::BasicBlock(instr_idx _firstInstrIdx, code_iterator _begin,
                       code_iterator _end, llvm::Function* _mainFunc)
    : m_firstInstrIdx{_firstInstrIdx},
      m_begin(_begin),
      m_end(_end),
      m_llvmBB(llvm::BasicBlock::Create(_mainFunc->getContext(),
                                        {".", std::to_string(_firstInstrIdx)},
                                        _mainFunc)) {
  BasicBlock::ctxDep();
  // stk_in = res.in;
  // stk_out = res.out;
}
void BasicBlock::ctxDep() {
  uint32_t prev_s = 0;
  uint32_t curr_s = 0;
  for (code_iterator it = m_begin; it != m_end;
       it = skipPushDataAndGetNext(it, m_end)) {
    uint8_t opcode = static_cast<uint8_t>(Instruction(*it));
    uint8_t _out = std::get<1>(InstrMap[opcode]);  // stack out , v
    uint8_t _in = std::get<2>(InstrMap[opcode]);   // stack in , \mu

    if (curr_s >= _out) {
      curr_s -= _out;
    } else {
      prev_s += _out - curr_s;
      curr_s = 0;
    }
    curr_s += _in;
  }
  m_stk_in = prev_s;
  m_stk_out = curr_s;
  // StkExecInfoType res = {prev_s, curr_s};
  // return res;
}

 

GlobalStack::GlobalStack(IRBuilder* _builder, llvm::Value* _stk,
                         llvm::Value* _spPtr) {
  IRB = _builder;
  stk = _stk;
  m_sp = _spPtr;
  first_block = IRB->GetInsertBlock();
  topIdx = 0;
}

llvm::Value* GlobalStack::getPtr(size_t idx) {
  // llvm::errs() << "GETPTR\n";
  llvm::Value* index = IRB->CreateLoad(m_sp);
  if (idx > 0) {
    index = IRB->CreateSub(index, llvm::ConstantInt::get(Type::Int64Ty, idx));
  }
  return IRB->CreateGEP(stk, {index});
}

void GlobalStack::push(llvm::Value* _value) {
  if (enableRegs) {
    // llvm::errs() << "PUSH\n";
    m_local.push_back(_value);
  } else {
    auto tsp = IRB->CreateLoad(m_sp);
    IRB->CreateStore(IRB->CreateAdd(tsp, IRB->getInt64(1)), m_sp);  // sp++

    if (_value->getType() != Type::Word) {
      auto _value256 = IRB->CreateZExt(_value, Type::Word);
      IRB->CreateStore(_value256, getPtr(0));  // get stk[sp-0]
    } else {
      IRB->CreateStore(_value, getPtr(0));  // get stk[sp-0]
    }
  }
}
 

llvm::Value* GlobalStack::pop() {
  llvm::Value* item;
  if (enableRegs) {
    item = *(m_local.rbegin());
    m_local.pop_back();
  } else {
    item = IRB->CreateLoad(getPtr(0));  // get stk[sp-0]
    // stk[0] --
    auto tsp = IRB->CreateLoad(m_sp);
    IRB->CreateStore(
        IRB->CreateSub(tsp, IRB->getInt64(1)),
        m_sp);
  }
  return item;
}


void GlobalStack::dup(size_t _index) {
  if (enableRegs) {
    assert(_index < m_local.size());
    llvm::Value* tmp = *(m_local.rbegin() + _index);  // count from back
    m_local.push_back(tmp);
  } else {
    auto ptr = getPtr(_index);  // src
    push(IRB->CreateLoad(ptr));
  }
}

void GlobalStack::swap(size_t _index) {
  // assert(_index >= 0);  // at least two elements

  if (enableRegs) {
    llvm::Value* srcVal = *(m_local.rbegin() + _index + 1);  // count from back
    // std::cerr << "@srcVal = "
    // << llvm::dyn_cast<llvm::ConstantInt>(srcVal)->getSExtValue() << "\n";
    llvm::Value* dstVal = *m_local.rbegin();
    // std::cerr << "@dstVal = "
    // << llvm::dyn_cast<llvm::ConstantInt>(dstVal)->getSExtValue() << "\n";
    auto lastIdx = m_local.size() - 1;
    m_local[lastIdx - (_index + 1)] = dstVal;
    m_local[lastIdx] = srcVal;
  } else {
    auto srcVal = IRB->CreateLoad(getPtr(0));
    auto dstPtr = getPtr(_index + 1);
    IRB->CreateStore(IRB->CreateLoad(dstPtr), getPtr(0));
    IRB->CreateStore(srcVal, dstPtr);
  }
}


}  // namespace trans
}  // namespace eth
}  // namespace dev
