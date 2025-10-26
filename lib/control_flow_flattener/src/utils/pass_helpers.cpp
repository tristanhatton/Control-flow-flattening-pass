
#include "pass_helpers.hpp"

#include <list>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Transforms/Scalar/Reg2Mem.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Local.h>

namespace obfuscation
{

namespace
{

bool valueEscapes ( const llvm::Instruction &Instruction )
{
    if ( !Instruction.getType ( )->isSized ( ) )
    {
        return false;
    }

    const llvm::BasicBlock *Block = Instruction.getParent ( );

    for ( const llvm::User *User : Instruction.users ( ) )
    {
        const llvm::Instruction *UserInstruction = llvm::cast< llvm::Instruction > ( User );
        if ( UserInstruction->getParent ( ) != Block || llvm::isa< llvm::PHINode > ( UserInstruction ) )
        {
            return true;
        }
    }
    return false;
}
} // namespace

/**
 * @brief Run the reg2mem pass on the given function. Modified version from LLVM
 *
 * @param Function The function to run the pass on
 * @return llvm::PreservedAnalyses
 */
llvm::PreservedAnalyses runRegToMemPass ( llvm::Function &Function )
{

    llvm::SplitAllCriticalEdges ( Function, llvm::CriticalEdgeSplittingOptions ( ) );

    auto *const BBEntry = &Function.getEntryBlock ( );

    auto Instruction = BBEntry->begin ( );

    while ( llvm::isa< llvm::AllocaInst > ( Instruction ) )
    {
        ++Instruction;
    }

    llvm::CastInst *const AllocaInsertionPoint =
        new llvm::BitCastInst ( llvm::Constant::getNullValue ( llvm::Type::getInt32Ty ( Function.getContext ( ) ) ),
                                llvm::Type::getInt32Ty ( Function.getContext ( ) ),
                                "reg2mem alloca point",
                                Instruction );

    std::list< llvm::Instruction * > WorkList;
    for ( llvm::Instruction &Instruction : llvm::instructions ( Function ) )
    {

        if ( !( llvm::isa< llvm::AllocaInst > ( Instruction ) && Instruction.getParent ( ) == BBEntry ) &&
             valueEscapes ( Instruction ) )
        {
            WorkList.push_front ( &Instruction );
        }
    }

    for ( llvm::Instruction *Instruction : WorkList )
    {

        llvm::DemoteRegToStack ( *Instruction, false, AllocaInsertionPoint->getIterator ( ) );
    }

    runDemotePhiNodesPass ( Function );

    return llvm::PreservedAnalyses::none ( );
}

llvm::PreservedAnalyses runDemotePhiNodesPass ( llvm::Function &Function )
{

    llvm::SmallVector< llvm::PHINode * > PhiNodes;

    for ( auto &Block : Function )
    {
        for ( auto &Phi : Block.phis ( ) )
        {
            PhiNodes.push_back ( &Phi );
        }
    }

    if ( PhiNodes.empty ( ) )
    {
        return llvm::PreservedAnalyses::all ( );
    }

    for ( auto *PhiNode : PhiNodes )
    {
        llvm::DemotePHIToStack ( PhiNode );
    }

    return llvm::PreservedAnalyses::none ( );
}

} // namespace obfuscation