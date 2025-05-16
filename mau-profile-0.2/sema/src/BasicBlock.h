#pragma once

#include <vector>

#include "Common.h"
#include "CompilerHelper.h"

namespace dev {
namespace eth {
namespace trans {
using namespace evmtrans;
using instr_idx = uint64_t;

class EEIModule;

class GlobalStack {
 public:
  bool enableRegs = false;

  GlobalStack(IRBuilder *_builder, llvm::Value* _stk, llvm::Value* _spPtr);

  void push(llvm::Value* _value);

  llvm::Value* pop();
  void dup(size_t _index);
  void swap(size_t _index);
  llvm::Value* get_sp() { return m_sp; };
  // llvm::Value* GlobalStack::popLocal();
  // void GlobalStack::pushLocal(llvm::Value* _value);

  std::vector<llvm::Value*>* view() { return &m_local; }

  void clearLocals() { m_local.clear(); }

  ssize_t minSize() const { return m_minSize; }
  ssize_t maxSize() const { return m_maxSize; }


 private:
  // llvm::Value* get(size_t _index, bool getValue = false);
  // void set(size_t _index, llvm::Value* _value);

  std::vector<llvm::Value *> m_local;
  std::vector<llvm::Value *> m_address;

  llvm::Value *stk;

  IRBuilder *IRB;
  llvm::Value *m_sp;  // address to the stack pointer
  llvm::BasicBlock *first_block;

  ssize_t m_minSize = 0;
  ssize_t m_maxSize = 0;
  ssize_t topIdx = 0;
  ssize_t stackSize = 32;

  // helper function
  // llvm::Value* storeItem(llvm::Value* item, llvm::Value* _addr = nullptr);
  // llvm::Value* loadItem(llvm::Value* addr);
  llvm::Value* getPtr(size_t _index);
};



class BasicBlock {
 public:
  explicit BasicBlock(instr_idx _firstInstrIdx, code_iterator _begin,
                      code_iterator _end, llvm::Function* _mainFunc);

  llvm::BasicBlock* llvm() { return m_llvmBB; }

  instr_idx firstInstrIdx() const { return m_firstInstrIdx; }

  code_iterator begin() const { return m_begin; }
  code_iterator end() const { return m_end; }
  code_iterator terminator() const { return m_end - 1; }

  instr_idx terminatorIdx() const {
    return m_firstInstrIdx + m_end - m_begin - 1;
  }

  // for cfg
  std::vector<std::pair<instr_idx, std::vector<instr_idx>>> succs() {
    return m_succs;
  }
  void add_succ(std::pair<instr_idx, std::vector<instr_idx>> _succ) {
    m_succs.push_back(_succ);
  }
  void pop_succ() { return m_succs.pop_back(); }
  // std::vector<instr_idx> dsuccs() {return dm_succs;}
  // void add_dsucc(instr_idx _k){ dm_succs.push_back(_k);}

  std::vector<BasicBlock*> bbs_from;
  std::vector<BasicBlock*> bbs_to;

  uint32_t get_stk_in() const { return m_stk_in; };
  uint32_t get_stk_out() const { return m_stk_out; };

  void add_phi(llvm::PHINode* node) { m_phi_nodes.push_back(node); };
  std::vector<llvm::PHINode*>* get_phi_nodes_ptr() { return &m_phi_nodes; };

 private:
  instr_idx const m_firstInstrIdx =
      0;  ///< Code index of first instruction in the block
  code_iterator const m_begin =
      {};  ///< Iterator pointing code beginning of the block
  code_iterator const m_end = {};  ///< Iterator pointing code end of the block

  llvm::BasicBlock* const m_llvmBB;  ///< Reference to the LLVM BasicBlock

  std::vector<std::pair<instr_idx, std::vector<instr_idx>>>
      m_succs;  ///< next edges

  uint32_t m_stk_in;
  uint32_t m_stk_out;

  void ctxDep();

  std::vector<llvm::PHINode*> m_phi_nodes;

  // std::vector<instr_idx> dm_succs{std::vector<instr_idx>(0,0)};
};

}  // namespace trans
}  // namespace eth
}  // namespace dev
