#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Type.h>

#include "config.h"
#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"

// #include "BinaryTrans.h"

namespace dev {
namespace eth {
namespace trans {

// struct StkExecInfoType
// {
// 	uint32_t in;
// 	uint32_t out;
// };


/******************************
Address Space	Memory Space
0	Generic
1	Global
2	Internal Use
3	Shared
4	Constant
5	Local
*******************************/

enum AS {
  /* 00 */ GENERIC,
  /* 01 */ GLOBAL,
  /* 02 */ INTERNAL,
  /* 03 */ SHARED,
  /* 04 */ CONST,
  /* 05 */ LOCAL
};

struct Type {

  static llvm::IntegerType* Word;
  static llvm::PointerType* WordPtr;

  static llvm::IntegerType* Bool;
  static llvm::IntegerType* Size;
  static llvm::IntegerType* Gas;
  static llvm::PointerType* GasPtr;

  static llvm::IntegerType* Byte;
  static llvm::PointerType* BytePtr;

  static llvm::Type* Void;

  /// Main function return type
  static llvm::IntegerType* MainReturn;

  static llvm::PointerType* EnvPtr;
  static llvm::PointerType* RuntimeDataPtr;
  static llvm::PointerType* RuntimePtr;

  // TODO: Redesign static LLVM objects
  static llvm::MDNode* expectTrue;

  // WASM Types
  /// i8, i16, i32, i64, i128, i256
  static llvm::IntegerType *Int8Ty, *Int32Ty, *Int64Ty, *Int128Ty, *Int256Ty;
  /// i160
  static llvm::IntegerType *AddressTy, *Int160Ty;

  /// i8*, i32*, i64*, i128*, i256*
  static llvm::PointerType *Int8PtrTy, *Int32PtrTy, *Int64PtrTy, *Int128PtrTy,
      *Int256PtrTy;
  /// i160*
  static llvm::PointerType *AddressPtrTy, *Int160PtrTy;

  /// args types
  static llvm::IntegerType* ArgsElemTy;
  static llvm::PointerType* ArgsElemPtrTy;
  /// return types
  static llvm::IntegerType* ReturnElemTy;
  static llvm::PointerType* ReturnElemPtrTy;
  /// bytes
  static llvm::StructType* BytesTy;
  static llvm::IntegerType* BytesElemTy;
  static llvm::PointerType* BytesElemPtrTy;
  /// String
  static llvm::StructType* StringTy;
  /// Storage
  static llvm::StructType* SlotTy;
  static llvm::PointerType* SlotPtrTy;
  static llvm::ArrayType* StorageTy;
  static llvm::PointerType* StoragePtrTy;

  // cuda types
  static llvm::PointerType *Int8Ptr1Ty, *Int32Ptr1Ty;

  static void init(llvm::LLVMContext& _context);
};

struct Constant {
  static llvm::ConstantInt* gasMax;

  /// Returns word-size constant
  static llvm::ConstantInt* get(int64_t _n);
  static llvm::ConstantInt* get(llvm::APInt const& _n);
};

}  // namespace trans
}  // namespace eth
}  // namespace dev
