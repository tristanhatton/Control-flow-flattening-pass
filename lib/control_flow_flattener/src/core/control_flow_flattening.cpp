#include "control_flow_flattening.hpp"
#include "config.hpp"
#include "utils/pass_helpers.hpp"
#include "utils/switch_case_manager.hpp"
#include "utils/token_analysis.hpp"

#include <algorithm>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/RandomNumberGenerator.h>
#include <llvm/Support/raw_ostream.h>

namespace obfuscation
{

namespace
{

/**
 * @brief Build a trampoline block for a switch case
 *
 * @param Function Function to which the block will be added
 * @param DestinationBlock Block to which control will be transferred
 * @param DispatcherIndex Index of the dispatcher
 * @param ValueToStore Value to store in the dispatcher
 * @return llvm::BasicBlock* Trampoline block
 */
llvm::BasicBlock *buildTrampolineBlock ( llvm::Function          &Function,
                                         llvm::BasicBlock *const  DestinationBlock,
                                         llvm::AllocaInst *const  DispatcherIndex,
                                         llvm::ConstantInt *const ValueToStore )
{
    auto &Context         = Function.getContext ( );
    auto *TrampolineBlock = llvm::BasicBlock::Create ( Context, "trampoline", &Function );

    llvm::IRBuilder<> Builder ( TrampolineBlock );
    Builder.CreateStore ( ValueToStore, DispatcherIndex );
    Builder.CreateBr ( DestinationBlock );

    return TrampolineBlock;
}

/**
 * @brief Run the control flow flattening pass
 *
 * @param Function Function to flatten
 * @return llvm::PreservedAnalyses
 */

llvm::PreservedAnalyses flattenControlFlow ( llvm::Function &Function )
{

    if ( Function.isDeclaration ( ) )
    {
        return llvm::PreservedAnalyses::all ( );
    }

    if ( Function.size ( ) < 2 )
    {
        return llvm::PreservedAnalyses::all ( );
    }

    runDemotePhiNodesPass ( Function );

    auto &Context    = Function.getContext ( );
    auto &entryBlock = Function.getEntryBlock ( );

    const auto allocaPoint =
        std::find_if ( entryBlock.begin ( ), entryBlock.end ( ), [] ( const llvm::Instruction &Instruction ) {
            return !llvm::isa< llvm::AllocaInst > ( Instruction );
        } );

    llvm::IRBuilder<> EntryBuilder ( &entryBlock, allocaPoint );
    auto *const       DispatcherIndex =
        EntryBuilder.CreateAlloca ( EntryBuilder.getInt64Ty ( ), nullptr, "dispatcher.index" );

    llvm::SmallVector< llvm::BasicBlock *, 8 > BlocksToProcess;
    for ( auto &Block : Function )
    {
        BlocksToProcess.push_back ( &Block );
    }

    if constexpr ( config::RANDOMIZE_SWITCH_CASE_ORDER )
    {
        std::shuffle ( BlocksToProcess.begin ( ), BlocksToProcess.end ( ), std::mt19937 { std::random_device { }( ) } );
    }

    auto *const DispatcherDefault = llvm::BasicBlock::Create ( Context, "dispatcher.default", &Function );

    llvm::IRBuilder<> DefaultBuilder ( DispatcherDefault );
    DefaultBuilder.CreateUnreachable ( );

    auto *const Dispatcher = llvm::BasicBlock::Create ( Context, "dispatcher", &Function );

    llvm::IRBuilder<> DispatcherBuilder ( Dispatcher );
    auto *const       DispatcherIndexLoad =
        DispatcherBuilder.CreateLoad ( EntryBuilder.getInt64Ty ( ), DispatcherIndex, "dispatcher.index.load" );
    auto *const Switch =
        DispatcherBuilder.CreateSwitch ( DispatcherIndexLoad, DispatcherDefault, BlocksToProcess.size ( ) );

    const auto RandomNumberGenerator = Function.getParent ( )->createRNG ( "dispatcher.index.random.number.generator" );

    SwitchCaseManager DispatcherSwitch ( Switch, [ &DispatcherBuilder, &RandomNumberGenerator ] ( ) {
        return llvm::ConstantInt::get ( DispatcherBuilder.getInt64Ty ( ), ( *RandomNumberGenerator ) ( ) );
    } );

    for ( auto &Block : BlocksToProcess )
    {

        if ( isBlockTokenIntermediate ( Block ) || Block->isEHPad ( ) )
        {
            continue;
        }

        auto *const Terminator = Block->getTerminator ( );

        if ( auto *const Branch = llvm::dyn_cast< llvm::BranchInst > ( Terminator ) )
        {
            if ( Branch->isUnconditional ( ) )
            {
                auto *const Successor = Branch->getSuccessor ( 0 );
                auto *const CaseValue = DispatcherSwitch.addCaseIfNotExist ( Successor );

                llvm::IRBuilder<> TrampolineBuilder ( Branch );
                TrampolineBuilder.CreateStore ( CaseValue, DispatcherIndex );
                TrampolineBuilder.CreateBr ( Dispatcher );
                Branch->eraseFromParent ( );
            }
            else
            {
                auto *const TrueSuccessor  = Branch->getSuccessor ( 0 );
                auto *const FalseSuccessor = Branch->getSuccessor ( 1 );

                auto *const TrueCaseValue  = DispatcherSwitch.addCaseIfNotExist ( TrueSuccessor );
                auto *const FalseCaseValue = DispatcherSwitch.addCaseIfNotExist ( FalseSuccessor );

                llvm::IRBuilder<> Builder ( Branch );
                auto *const       Condition = Branch->getCondition ( );
                auto *const NextIndex = Builder.CreateSelect ( Condition, TrueCaseValue, FalseCaseValue, "next.index" );

                Builder.CreateStore ( NextIndex, DispatcherIndex );
                Builder.CreateBr ( Dispatcher );
                Branch->eraseFromParent ( );
            }
        }
        else if ( auto *const Switch = llvm::dyn_cast< llvm::SwitchInst > ( Terminator ) )
        {
            {
                auto *const DefaultSuccessor = Switch->getDefaultDest ( );
                auto *const DefaultCaseValue = DispatcherSwitch.addCaseIfNotExist ( DefaultSuccessor );

                auto *const TrampolineBlock =
                    buildTrampolineBlock ( Function, Dispatcher, DispatcherIndex, DefaultCaseValue );

                Switch->setDefaultDest ( TrampolineBlock );
            }

            for ( auto &Case : Switch->cases ( ) )
            {
                auto *const TargetBlock = Case.getCaseSuccessor ( );
                auto *const CaseValue   = DispatcherSwitch.addCaseIfNotExist ( TargetBlock );

                auto *const TrampolineBlock = buildTrampolineBlock ( Function, Dispatcher, DispatcherIndex, CaseValue );

                Case.setSuccessor ( TrampolineBlock );
            }
        }
        else if ( auto *const Invoke = llvm::dyn_cast< llvm::InvokeInst > ( Terminator ) )
        {
            auto *const NormalSuccessor = Invoke->getNormalDest ( );

            auto *const NormalCaseValue = DispatcherSwitch.addCaseIfNotExist ( NormalSuccessor );

            auto *const NormalTrampolineBlock =
                buildTrampolineBlock ( Function, Dispatcher, DispatcherIndex, NormalCaseValue );

            Invoke->setNormalDest ( NormalTrampolineBlock );
        }
        else
        {
            continue;
        }
    }

    runRegToMemPass ( Function );

    return llvm::PreservedAnalyses::none ( );
}

} // namespace

bool ControlFlowFlattenerPass::isRequired ( )
{
    return true;
}

llvm::PreservedAnalyses ControlFlowFlattenerPass::run ( llvm::Function                &Function,
                                                        llvm::FunctionAnalysisManager &functionAnalysisManager )
{

    llvm::PreservedAnalyses PreservedAnalyses ( llvm::PreservedAnalyses::all ( ) );
    for ( auto iteration = 0; iteration < config::FLATTEN_ITERATIONS; ++iteration )
    {
        PreservedAnalyses.intersect ( flattenControlFlow ( Function ) );
    }
    return PreservedAnalyses;
}
} // namespace obfuscation