#pragma once

#include <llvm/IR/BasicBlock.h>

namespace obfuscation
{

/**
 * @brief Check if the block is a token intermediate
 *
 * @param Block Block to check
 * @return true
 * @return false
 */

bool isBlockTokenIntermediate ( llvm::BasicBlock *const Block );

} // namespace obfuscation
