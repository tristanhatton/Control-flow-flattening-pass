#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>

namespace obfuscation
{

/**
 * @brief Control Flow Flattening Pass
 *
 */

struct ControlFlowFlattenerPass : public llvm::PassInfoMixin< ControlFlowFlattenerPass >
{

    /**
     * @brief Run the control flow flattening pass
     *
     * @param Function Function to flatten
     * @param FunctionAnalysisManager Function Analysis Manager
     * @return llvm::PreservedAnalyses
     */
    llvm::PreservedAnalyses run ( llvm::Function &Function, llvm::FunctionAnalysisManager &FunctionAnalysisManager );

    /**
     * @brief Check if the pass is required
     *
     * @return true
     * @return false
     */
    static bool isRequired ( );
};

} // namespace obfuscation