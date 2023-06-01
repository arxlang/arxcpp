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

#include <llvm/ADT/StringRef.h>                                 // for Strin...
#include <llvm/ADT/Triple.h>                                    // for Triple
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>              // for Concu...
#include <llvm/ExecutionEngine/Orc/Core.h>                      // for Execu...
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>            // for Dynam...
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>    // for Execu...
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>            // for IRCom...
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>   // for JITTa...
#include <llvm/ExecutionEngine/Orc/Mangling.h>                  // for Mangl...
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>  // for RTDyl...
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>          // for Threa...
#include <llvm/ExecutionEngine/SectionMemoryManager.h>          // for Secti...
#include <llvm/IR/DataLayout.h>                                 // for DataL...
#include <llvm/Support/Error.h>                                 // for Expected
#include <memory>                                               // for __base
#include <new>                                                  // for opera...

namespace llvm {
  class JITEvaluatedSymbol;
}

namespace llvm {
  namespace orc {

    /**
     * @brief Tokenize the known variables by the lexer
     *
     *
     */
    class ArxJIT {
     private:
      std::unique_ptr<ExecutionSession> execution_session;

      DataLayout data_layout;
      MangleAndInterner mangle;

      RTDyldObjectLinkingLayer object_layer;
      IRCompileLayer CompileLayer;

      JITDylib& main_jit_dylib;

     public:
      /**
       * @param execution_session ExecutionSession
       * @param jit_target_machine_builder JITTargetMachineBuilder
       * @param data_layout DataLayout
       */
      ArxJIT(
        std::unique_ptr<ExecutionSession> _execution_session,
        JITTargetMachineBuilder jit_target_machine_builder,
        DataLayout _data_layout)
          : execution_session(std::move(_execution_session)),
            data_layout(_data_layout),
            mangle(*this->execution_session, this->data_layout),
            object_layer(
              *this->execution_session,
              []() { return std::make_unique<SectionMemoryManager>(); }),
            CompileLayer(
              *this->execution_session,
              this->object_layer,
              std::make_unique<ConcurrentIRCompiler>(
                std::move(jit_target_machine_builder))),
            main_jit_dylib(
              this->execution_session->createBareJITDylib("<main>")) {
        this->main_jit_dylib.addGenerator(
          cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
            this->data_layout.getGlobalPrefix())));

        if (jit_target_machine_builder.getTargetTriple().isOSBinFormatCOFF()) {
          this->object_layer.setOverrideObjectFlagsWithResponsibilityFlags(
            true);
          this->object_layer.setAutoClaimResponsibilityForObjectSymbols(true);
        }
      }

      ~ArxJIT() {
        if (auto err = this->execution_session->endSession()) {
          this->execution_session->reportError(std::move(err));
        }
      }

      static Expected<std::unique_ptr<ArxJIT>> Create() {
        auto executor_process_control = SelfExecutorProcessControl::Create();
        if (!executor_process_control) {
          return executor_process_control.takeError();
        }

        auto _execution_session = std::make_unique<ExecutionSession>(
          std::move(*executor_process_control));

        JITTargetMachineBuilder jit_target_machine_builder(
          _execution_session->getExecutorProcessControl().getTargetTriple());

        auto _data_layout =
          jit_target_machine_builder.getDefaultDataLayoutForTarget();
        if (!_data_layout) {
          return _data_layout.takeError();
        }

        return std::make_unique<ArxJIT>(
          std::move(_execution_session),
          std::move(jit_target_machine_builder),
          std::move(*_data_layout));
      }

      const DataLayout& get_data_layout() const {
        return this->data_layout;
      }

      JITDylib& get_main_jit_dylib() {
        return this->main_jit_dylib;
      }

      Error addModule(
        ThreadSafeModule thread_safe_module,
        ResourceTrackerSP resource_tracker_sp = nullptr) {
        if (!resource_tracker_sp) {
          resource_tracker_sp = main_jit_dylib.getDefaultResourceTracker();
        }
        return CompileLayer.add(
          resource_tracker_sp, std::move(thread_safe_module));
      }

      Expected<JITEvaluatedSymbol> lookup(StringRef name) {
        return this->execution_session->lookup(
          {&main_jit_dylib}, mangle(name.str()));
      }
    };

  }  // end namespace orc
}  // end namespace llvm

#endif  // LLVM_EXECUTIONENGINE_ORC_ArxJIT_H
