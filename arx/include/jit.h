/**
 *
 * @brief Include ArxJIT.h - A simple JIT for Arx in C++
 *
 *
 *
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM
 * Exceptions. See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * ===----------------------------------------------------------------------===
 */

#ifndef LLVM_EXECUTIONENGINE_ORC_ArxJIT_H
#define LLVM_EXECUTIONENGINE_ORC_ArxJIT_H

#include <memory>
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

namespace llvm {
namespace orc {

/**
 * @brief Tokenize the known variables by the lexer
 *
 *
 */
class ArxJIT {
 private:
  std::unique_ptr<ExecutionSession> ES;

  DataLayout DL;
  MangleAndInterner Mangle;

  RTDyldObjectLinkingLayer ObjectLayer;
  IRCompileLayer CompileLayer;

  JITDylib& MainJD;

 public:
  /**
   * @param ES
   * @param JTMB
   * @param DL
   */
  ArxJIT(
      std::unique_ptr<ExecutionSession> ES,
      JITTargetMachineBuilder JTMB,
      DataLayout DL)
      : ES(std::move(ES)),
        DL(DL),
        Mangle(*this->ES, this->DL),
        ObjectLayer(
            *this->ES,
            []() { return std::make_unique<SectionMemoryManager>(); }),
        CompileLayer(
            *this->ES,
            ObjectLayer,
            std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
        MainJD(this->ES->createBareJITDylib("<main>")) {
    MainJD.addGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
  }

  ~ArxJIT() {
    if (auto Err = ES->endSession())
      ES->reportError(std::move(Err));
  }

  static Expected<std::unique_ptr<ArxJIT>> Create() {
    auto EPC = SelfExecutorProcessControl::Create();
    if (!EPC)
      return EPC.takeError();

    auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

    JITTargetMachineBuilder JTMB(
        ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL)
      return DL.takeError();

    return std::make_unique<ArxJIT>(
        std::move(ES), std::move(JTMB), std::move(*DL));
  }

  const DataLayout& getDataLayout() const {
    return DL;
  }

  JITDylib& getMainJITDylib() {
    return MainJD;
  }

  Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr) {
    if (!RT)
      RT = MainJD.getDefaultResourceTracker();
    return CompileLayer.add(RT, std::move(TSM));
  }

  Expected<JITEvaluatedSymbol> lookup(StringRef Name) {
    return ES->lookup({&MainJD}, Mangle(Name.str()));
  }
};

}  // end namespace orc
}  // end namespace llvm

#endif  // LLVM_EXECUTIONENGINE_ORC_ArxJIT_H
