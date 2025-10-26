#include "switch_case_manager.hpp"

namespace obfuscation
{
SwitchCaseManager::SwitchCaseManager ( llvm::SwitchInst *const                        Switch,
                                       const std::function< llvm::ConstantInt *( ) > &GenerateCaseValue )
    : Switch_ ( Switch ), GenerateCaseValue_ ( GenerateCaseValue )
{
}

llvm::ConstantInt *SwitchCaseManager::generateUniqueCaseValue ( )
{

    llvm::ConstantInt *CaseValue;
    do
    {
        CaseValue = GenerateCaseValue_ ( );

    } while ( Switch_->findCaseValue ( CaseValue ) != Switch_->case_default ( ) );

    return CaseValue;
}

void SwitchCaseManager::addCase ( llvm::ConstantInt *const CaseValue, llvm::BasicBlock *const TargetBlock )
{
    Switch_->addCase ( CaseValue, TargetBlock );
}

llvm::ConstantInt *SwitchCaseManager::addCaseIfNotExist ( llvm::BasicBlock *const TargetBlock )
{

    if ( auto *const CaseValue = Switch_->findCaseDest ( TargetBlock ) )
    {
        return CaseValue;
    }

    auto *const NewCaseValue = generateUniqueCaseValue ( );

    Switch_->addCase ( NewCaseValue, TargetBlock );

    return NewCaseValue;
}
} // namespace obfuscation