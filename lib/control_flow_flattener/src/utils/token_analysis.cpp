
#include "token_analysis.hpp"

#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Instructions.h>

namespace obfuscation
{

namespace
{

/**
 * @brief Gathers all produced tokens in the given basic block.
 *
 * @param Block The basic block to analyze.
 * @return llvm::DenseSet< const llvm::Value * > A set of produced token values.
 */
llvm::DenseSet< const llvm::Value * > gatherProducedTokens ( llvm::BasicBlock *const Block )
{
    llvm::SmallVector< const llvm::BasicBlock * > WorkList;
    llvm::DenseSet< const llvm::BasicBlock * >    Visited;
    llvm::DenseSet< const llvm::Value * >         ProducedTokens;

    for ( const auto *Predecessor : llvm::predecessors ( Block ) )
    {
        WorkList.push_back ( Predecessor );
    }

    while ( !WorkList.empty ( ) )
    {
        auto *const Current = WorkList.pop_back_val ( );

        if ( !Visited.insert ( Current ).second )
        {
            continue;
        }

        for ( const auto &Instruction : *Current )
        {
            if ( Instruction.getType ( )->isTokenTy ( ) )
            {
                ProducedTokens.insert ( &Instruction );
            }
        }

        for ( const auto *Predecessor : llvm::predecessors ( Current ) )
        {
            if ( !Visited.contains ( Predecessor ) )
            {
                WorkList.push_back ( Predecessor );
            }
        }
    }

    return ProducedTokens;
}

/**
 * @brief Gathers all consumed tokens in the given basic block.
 *
 * @param Block The basic block to analyze.
 * @return llvm::DenseSet< const llvm::Value * > A set of consumed token values.
 */
llvm::DenseSet< const llvm::Value * > gatherConsumedTokens ( llvm::BasicBlock *const Block )
{

    llvm::SmallVector< const llvm::BasicBlock * > WorkList;
    llvm::DenseSet< const llvm::BasicBlock * >    Visited;
    llvm::DenseSet< const llvm::Value * >         ConsumedTokens;

    WorkList.push_back ( Block );

    while ( !WorkList.empty ( ) )
    {
        auto *const Current = WorkList.pop_back_val ( );

        if ( !Visited.insert ( Current ).second )
        {
            continue;
        }

        for ( const auto &Instruction : *Current )
        {
            for ( const auto &Operand : Instruction.operands ( ) )
            {
                if ( Operand->getType ( )->isTokenTy ( ) )
                {
                    ConsumedTokens.insert ( Operand );
                }
            }
        }

        for ( const auto *Successor : llvm::successors ( Current ) )
        {
            if ( !Visited.contains ( Successor ) )
            {
                WorkList.push_back ( Successor );
            }
        }
    }
    return ConsumedTokens;
}
} // namespace

bool isBlockTokenIntermediate ( llvm::BasicBlock *const Block )
{

    const auto ProducedTokens = gatherProducedTokens ( Block );
    const auto ConsumedTokens = gatherConsumedTokens ( Block );

    for ( const auto *Token : ProducedTokens )
    {
        if ( ConsumedTokens.contains ( Token ) )
        {
            return true;
        }
    }

    return false;
}
} // namespace obfuscation