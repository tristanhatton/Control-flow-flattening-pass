
#include "core/control_flow_flattening.hpp"

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

/**
 * @brief Get the Control Flow Flattener Plugin Info object
 *
 * @return llvm::PassPluginLibraryInfo
 */
llvm::PassPluginLibraryInfo getControlFlowFlattenerPluginInfo ( )
{
    return { LLVM_PLUGIN_API_VERSION, "ControlFlowFlattener", "v0.1", [] ( llvm::PassBuilder &PassBuilder ) {
                PassBuilder.registerPipelineParsingCallback (
                    [] ( llvm::StringRef            Name,
                         llvm::FunctionPassManager &FunctionPassManager,
                         llvm::ArrayRef< llvm::PassBuilder::PipelineElement > ) {
                        if ( Name == "control-flow-flattener" )
                        {
                            FunctionPassManager.addPass ( obfuscation::ControlFlowFlattenerPass ( ) );
                            return true;
                        }
                        return false;
                    } );
            } };
}

/**
 * @brief Get the Control Flow Flattener Plugin Info object
 *
 * @return llvm::PassPluginLibraryInfo
 */

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo llvmGetPassPluginInfo ( )
{
    return getControlFlowFlattenerPluginInfo ( );
}