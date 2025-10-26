#pragma once

#include <llvm/IR/Analysis.h>
#include <llvm/IR/Function.h>

namespace obfuscation
{

/**
 * @brief Run the register to memory pass
 *
 * @param Function Function to transform
 * @return llvm::PreservedAnalyses
 */
llvm::PreservedAnalyses runRegToMemPass ( llvm::Function &Function );
/**
 * @brief Run the demote phi nodes pass
 *
 * @param Function Function to analyze
 * @return llvm::PreservedAnalyses
 */

llvm::PreservedAnalyses runDemotePhiNodesPass ( llvm::Function &Function );

} // namespace obfuscation