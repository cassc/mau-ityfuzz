#include "CodeGen.h"

#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/CodeGen/LinkAllAsmWriterComponents.h>
#include <llvm/CodeGen/LinkAllCodegenComponents.h>
#include <llvm/CodeGen/MIRParser/MIRParser.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/AutoUpgrade.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/Pass.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "preprocessor/llvm_includes_start.h"
// #include <binaryen-c.h>
#include "preprocessor/llvm_includes_start.h"
// #include <lld/Common/Driver.h>

namespace dev {
namespace eth {
namespace trans {
llvm::TargetOptions InitTargetOptionsFromCodeGenFlags() {
  llvm::TargetOptions Options;
  Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
  Options.UnsafeFPMath = false;
  Options.NoInfsFPMath = false;
  Options.NoNaNsFPMath = false;
  Options.NoSignedZerosFPMath = false;
  Options.NoTrappingFPMath = false;
  // Options.FPDenormalMode = llvm::FPDenormal::IEEE;
  // Options.setFPDenormalMode(llvm::DenormalMode::IEEE);
  Options.HonorSignDependentRoundingFPMathOption = false;
  Options.NoZerosInBSS = false;
  Options.GuaranteedTailCallOpt = false;
  // Options.StackAlignmentOverride = 0;
  Options.StackSymbolOrdering = true;
  Options.UseInitArray = true;
  Options.RelaxELFRelocations = false;
  Options.DataSections = false;
  Options.FunctionSections = false;
  Options.UniqueSectionNames = true;
  Options.EmulatedTLS = false;
  Options.ExplicitEmulatedTLS = false;
  Options.ExceptionModel = llvm::ExceptionHandling::None;
  Options.EmitStackSizeSection = false;
  Options.EmitAddrsig = false;

  // Options.MCOptions = InitMCTargetOptionsFromFlags();

  Options.ThreadModel = llvm::ThreadModel::POSIX;
  Options.EABIVersion = llvm::EABI::Default;
  Options.DebuggerTuning = llvm::DebuggerKind::Default;

  return Options;
}


/// Set linkage.
///
/// If there are no errors, the function returns false. If an error is
/// found, then true is returned.

bool codeGenModule(llvm::TargetMachine& _target, llvm::Module& _module) {
  // std::cerr << "setTargetTriple\n";
  auto TheTriple = llvm::Triple(llvm::Triple::normalize("nvptx64-nvidia-cuda"));
  _module.setTargetTriple(TheTriple.str());
  // _module.setTargetTriple("wasm32-unknown-unknown-wasm");
  // std::cerr << "setTargetTriple done\n";
  auto PM = llvm::legacy::PassManager{};
  llvm::TargetLibraryInfoImpl TLII(llvm::Triple(_module.getTargetTriple()));
  // auto TLII = std::make_unique<llvm::TargetLibraryInfoImpl>(
  // llvm::Triple(_module.getTargetTriple()));
  // TLII->disableAllFunctions();
  PM.add(new llvm::TargetLibraryInfoWrapperPass(TLII));

  // std::cerr << "setDataLayout\n";
  // _module.setDataLayout(_target.createDataLayout());
  // _module.setDataLayout(llvm::DataLayout("e-m:e-p:32:32-i64:64-n32:64-S128"));
  _module.setDataLayout(llvm::DataLayout(
      "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:"
      "64:64-v16:16:16-v32:32:32-v64:64:64-v128:128:128-n16:32:64"));

  llvm::UpgradeDebugInfo(_module);
  // std::cerr << "setDataLayout done\n";

  llvm::raw_os_ostream cerr{std::cerr};
  if (llvm::verifyModule(_module, &cerr)) {
    // failed
    return true;
  } else {
    return false;
  }
}
// ------------------ End Codegen ------------------

}  // namespace trans
}  // namespace eth
}  // namespace dev