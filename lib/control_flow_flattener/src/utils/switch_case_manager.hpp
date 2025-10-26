#pragma once

#include <functional>
#include <llvm/IR/Instructions.h>

namespace obfuscation
{

/**
 * @brief Manages the cases of a switch instruction.
 *
 */
class SwitchCaseManager
{

  private:
    llvm::SwitchInst                       *Switch_;
    std::function< llvm::ConstantInt *( ) > GenerateCaseValue_;

  public:
    /**
     * @brief Construct a new Switch Case Manager object
     *
     * @param Switch Switch instruction to manage
     * @param GenerateCaseValue Function to generate unique case values
     */
    explicit SwitchCaseManager ( llvm::SwitchInst *const                        Switch,
                                 const std::function< llvm::ConstantInt *( ) > &GenerateCaseValue );

  private:
    /**
     * @brief Generate a unique case value for the switch instruction.
     *
     * @return llvm::ConstantInt*
     */
    llvm::ConstantInt *generateUniqueCaseValue ( );

  public:
    /**
     * @brief Add a case to the switch instruction.
     *
     * @param CaseValue Unique value for the case
     * @param TargetBlock Block to which control will be transferred
     */
    void addCase ( llvm::ConstantInt *const CaseValue, llvm::BasicBlock *const TargetBlock );

    /**
     * @brief Add a case to the switch instruction if it does not already exist.
     *
     * @param TargetBlock Block to which control will be transferred
     * @return llvm::ConstantInt* Unique value for the case
     */
    llvm::ConstantInt *addCaseIfNotExist ( llvm::BasicBlock *const TargetBlock );
};

} // namespace obfuscation