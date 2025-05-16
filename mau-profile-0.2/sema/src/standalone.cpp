#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <stdio.h>

#include <chrono>
#include <fstream>
#include <iostream>

#include "Utils.h"
#include "AFL.h"
#include "CodeGen.h"
#include "Optimizer.h"
#include "WASMCompiler.h"
#include "preprocessor/llvm_includes_end.h"
#include "preprocessor/llvm_includes_start.h"

namespace cl = llvm::cl;
cl::opt<std::string> InputFn(cl::Positional,
                                   cl::desc("<input filename to the EVM bytecode>"),
                                   cl::init("-"), cl::value_desc("filename"));
cl::opt<std::string> OutputFilename("o", cl::desc("Output filename"),
                                    cl::value_desc("filename"), cl::init("-"));

// cl::opt<bool> g_debug{"debug", cl::desc{"Output Debug Info"}};
cl::opt<int64_t> g_txs_len("txs-len", cl::desc{"The length of the transactions sequence"}, cl::init(1));

cl::opt<bool> g_dump{"dump", cl::desc{"Dump LLVM IR module"}};
cl::opt<bool> g_hex{"hex", cl::desc{"Read bytecode in a hexadecimal string"}};
cl::opt<bool> g_hex_runtime{
    "hex-runtime", cl::desc{"Read the runtime code in a hexadecimal string"}};
cl::opt<bool> g_disable_instrument{"disable-instrument",
                                   cl::desc{"Disable the instrumentation"}};

cl::opt<std::string> FSanitize(
    "fsanitize",
    cl::desc("Turn on runtime checks for various forms of undefined or "
             "suspicious behavior. See user manual for available checks"),
    cl::value_desc("check"), cl::init("-"));


std::string HexToStr(const std::string& str) {
  std::string result;
  size_t i = 0;
  if (str.compare(0, 2, "0x") == 0) i = 2; 
  for (; i < str.length(); i += 2) {
      std::string byte = str.substr(i, 2);
      char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
      result.push_back(chr);
  }
  return result;
}

std::unique_ptr<llvm::TargetMachine> createTarget() {
  llvm::CodeGenOpt::Level OLvl = llvm::CodeGenOpt::Default;
  auto TheTriple = llvm::Triple(llvm::Triple::normalize("nvptx64-nvidia-cuda"));
  std::string Error;
  const llvm::Target *TheTarget =
      llvm::TargetRegistry::lookupTarget(TheTriple.getTriple(), Error);
  if (!TheTarget) {
    std::cerr << Error << "\n";
    return nullptr;
  }
  llvm::TargetOptions Options =
      dev::eth::trans::InitTargetOptionsFromCodeGenFlags();
  std::string CPUStr = "";
  llvm::SubtargetFeatures Features;
  std::string FeaturesStr = Features.getString();
  std::unique_ptr<llvm::TargetMachine> Target(TheTarget->createTargetMachine(
      TheTriple.getTriple(), CPUStr, FeaturesStr, Options, llvm::Reloc::Static,
      llvm::CodeModel::Small, OLvl));
  assert(Target && "Could not allocate target machine!");

  return Target;
}

void exportIR(std::string outfile, llvm::Module &module) {
  std::cerr << "exported to " << outfile << "\n";
  std::error_code ec;
  llvm::raw_fd_ostream fout(llvm::StringRef(outfile), 
                            ec);
  if (ec) {
    llvm::raw_os_ostream cerr{std::cerr};
    module.print(cerr, nullptr);
  } else {
    module.print(fout, nullptr);
  }
}

int main(int argc, char **argv) {
  // using clock = std::chrono::high_resolution_clock;
  // auto startTime = clock::now();

  cl::ParseCommandLineOptions(argc, argv, "EVM Bytecode to PTXIR Lifter\n");
  if (g_txs_len < 1 || g_txs_len > TRANSACTIONS_MAX_LENGTH)
    exit(-1);


  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();

  //  Read EVM bytecode from input file
  std::ifstream fp;
  fp.open(InputFn);
  if (fp.fail()) {
    std::cerr << argv[0] << ": " << InputFn << "no such file\n";
    return -1;
  }
  std::string Buffer;
  std::getline(fp, Buffer);
  fp.close();
  std::string Bytes = g_hex ? HexToStr(Buffer) : Buffer;

  uint32_t RuntimeCodeOffset = dev::eth::trans::splitCode(Bytes);
  if (!RuntimeCodeOffset) {
    std::cerr << "Fail to extract the runtime code.\n";
    exit(-1);
  }
  std::string DeployerBytes = Bytes.substr(0, RuntimeCodeOffset);
  std::string RuntimeBytes = Bytes.substr(RuntimeCodeOffset, Bytes.size());

  // Generate LLVM Assembly
  llvm::LLVMContext LLVMCtx;
  auto ModulePtr = std::make_unique<llvm::Module>(InputFn, LLVMCtx);
  llvm::Module &Module = *ModulePtr.get();
  
  dev::eth::trans::EEIModule UtilsM(Module); // EEI emulator
  dev::eth::trans::makeGlobalVar(Module);
  dev::eth::trans::makeGlobalBytecode(Module, Bytes);
  
  dev::eth::trans::WASMCompiler RuntimeCC({}, Module, UtilsM);
  RuntimeCC.compileMain(RuntimeCodeOffset, RuntimeBytes, CUDA_CONTRACT_FUNC);

  // sanitizers
  for (auto& _bb : *Module.getFunction(CUDA_CONTRACT_FUNC)) 
    for (auto it = _bb.begin(); it != _bb.end(); ++it) {
      llvm::Instruction *Inst = &(*it);
      if (Inst->hasMetadata(c_rensan)) Inst->setMetadata(c_rensan, NULL);
      if (Inst->hasMetadata(c_msesan)) Inst->setMetadata(c_msesan, NULL);
      if (Inst->hasMetadata(c_elksan)) Inst->setMetadata(c_elksan, NULL);
    }
    
  auto KFnRuntimer = dev::eth::trans::wrapRuntimer(Module, g_txs_len);

  // the AFL-favour instrumentation
  if (!g_disable_instrument) afl::instrument(Module);

  // dev::eth::trans::WASMCompiler DeployCC({}, Module, UtilsM);
  // DeployCC.compileMain(0, DeployerBytes, CUDA_DEPLOY_FUNC);
  // auto KFnDeployer = dev::eth::trans::wrapDeployer(Module);
  
  // if (!g_disable_instrument) afl::instrumentDeployer(Module);

  // Optimization
  // dev::eth::trans::prepare(Module);

  // TODO, PTX64 Code Geneeration
  std::unique_ptr<llvm::TargetMachine> t_machine = std::move(createTarget());
  if (dev::eth::trans::codeGenModule(*t_machine, Module)) {
    std::cerr << "The Bitcode is broken \n";
    exportIR(OutputFilename, Module);
    exit(-1);
  }
  
  if (g_dump) exportIR(OutputFilename, Module);

  
  // Convert duration to string with microsecond units.
  // constexpr auto to_us_str = [](clock::duration d) {
  //   return std::to_string(
  //       std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  // };
  // std::cout << "Translation Complete. Time [us]: "
  //           << to_us_str(clock::now() - startTime) << "\n";

  return 0;
}