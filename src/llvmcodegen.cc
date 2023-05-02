#include "llvmcodegen.hh"
#include "ast.hh"
#include <iostream>
#include <stdio.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <vector>

#define MAIN_FUNC compiler->module.getFunction("main")

/*
The documentation for LLVM codegen, and how exactly this file works can be found
ins `docs/llvm.md`
*/

void LLVMCompiler::compile(Node *root) {
    /* Adding reference to print_i in the runtime library */
    // void printi();
    FunctionType *printi_func_type = FunctionType::get(
        builder.getVoidTy(),
        {builder.getInt64Ty()},
        false
    );
    Function::Create(
        printi_func_type,
        GlobalValue::ExternalLinkage,
        "printi",
        &module
    );
    /* we can get this later 
        module.getFunction("printi");
    */

    /* Main Function */
    // int main();
    // FunctionType *main_func_type = FunctionType::get(
    //     builder.getInt32Ty(), {}, false /* is vararg */
    // );
    // Function *main_func = Function::Create(
    //     main_func_type,
    //     GlobalValue::ExternalLinkage,
    //     "main",
    //     &module
    // );

    // // create main function block
    // BasicBlock *main_func_entry_bb = BasicBlock::Create(
    //     *context,
    //     "entry",
    //     main_func
    // );

    // move the builder to the start of the main function block
    // builder.SetInsertPoint(main_func_entry_bb);

    root->llvm_codegen(this);

    Function *main_func = module.getFunction("main");
    if(!main_func) {
        std::cerr << "Function main not found in module.\n";
        exit(EXIT_FAILURE);
    }

    // Create the start function
    FunctionType *start_func_type = FunctionType::get(
        builder.getInt32Ty(),
        {},
        false /* is vararg */
    );
    Function *start_func = Function::Create(
        start_func_type,
        GlobalValue::ExternalLinkage,
        "start",
        &module
    );

    // Create a basic block for the start function
    BasicBlock *start_func_entry_bb = BasicBlock::Create(
        *context,
        "entry",
        start_func
    );

    // Set the builder to the start of the start function block
    builder.SetInsertPoint(start_func_entry_bb);

    // Call the main function
    builder.CreateCall(main_func, {});

    // return 0;
    builder.CreateRet(builder.getInt32(0));
}

void LLVMCompiler::dump() {
    outs() << module;
}

void LLVMCompiler::write(std::string file_name) {
    std::error_code EC;
    raw_fd_ostream fout(file_name, EC, sys::fs::OF_None);
    WriteBitcodeToFile(module, fout);
    fout.flush();
    fout.close();
}

//  ┌―――――――――――――――――――――┐  //
//  │ AST -> LLVM Codegen │  //
//  └―――――――――――――――――――――┘  //

// codegen for statements
Value *NodeStmts::llvm_codegen(LLVMCompiler *compiler) {
    Value *last = nullptr;
    for(auto node : list) {
        last = node->llvm_codegen(compiler);
    }
    return last;
}

Value *NodeDebug::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);
    Type *expr_type = expr->getType();
    
    Function *printi_func = compiler->module.getFunction("printi");

    if (expr_type->isIntegerTy(16)) {
        expr = compiler->builder.CreateSExt(expr, compiler->builder.getInt64Ty(), "short_to_long");
    } else if (expr_type->isIntegerTy(32)) {
        expr = compiler->builder.CreateSExt(expr, compiler->builder.getInt64Ty(), "int_to_long");
    }

    if (expr_type->isIntegerTy(16) || expr_type->isIntegerTy(32) || expr_type->isIntegerTy(64)) {
        compiler->builder.CreateCall(printi_func, {expr});
    } else {
        std::cerr << "NodeDebug does not support the given data type.\n";
        exit(EXIT_FAILURE);
    }

    return expr;
}


Value *NodeLIT::llvm_codegen(LLVMCompiler *compiler) {
    if(lit_type == INT){
        return compiler->builder.getInt32(value.value_int);
    }else if(lit_type == SHORT){
        return compiler->builder.getInt16(value.value_short);
    }else{
        return compiler->builder.getInt64(value.value_long);
    }
}

Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler) {
    Value *left_expr = left->llvm_codegen(compiler);
    Value *right_expr = right->llvm_codegen(compiler);

    Type *left_type = left_expr->getType();
    Type *right_type = right_expr->getType();
    Type *result_type = nullptr;

    if (left_type == right_type) {
        result_type = left_type;
    } else{
        // Determine the highest type for integer arithmetic
        if (left_type->getIntegerBitWidth() >= 64 || right_type->getIntegerBitWidth() >= 64) {
            result_type = compiler->builder.getInt64Ty();
        } else if (left_type->getIntegerBitWidth() >= 32 || right_type->getIntegerBitWidth() >= 32) {
            result_type = compiler->builder.getInt32Ty();
        } else {
            result_type = compiler->builder.getInt16Ty();
        }
    }
        // Error: mismatched types
        // std::cerr << "Error: incompatible types in binary operation\n";
        // exit(EXIT_FAILURE);

    switch(op) {
        case PLUS:
        return compiler->builder.CreateAdd(
            compiler->builder.CreateSExt(left_expr, result_type),
            compiler->builder.CreateSExt(right_expr, result_type),
            "addtmp");
        case MINUS:
        return compiler->builder.CreateSub(
            compiler->builder.CreateSExt(left_expr, result_type),
            compiler->builder.CreateSExt(right_expr, result_type),
            "minustmp");
        case MULT:
        return compiler->builder.CreateMul(
            compiler->builder.CreateSExt(left_expr, result_type),
            compiler->builder.CreateSExt(right_expr, result_type),
            "multtmp");
        case DIV:
        return compiler->builder.CreateSDiv(
            compiler->builder.CreateSExt(left_expr, result_type),
            compiler->builder.CreateSExt(right_expr, result_type),
            "divtmp");
    }
}


// Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler) {
//     Value *left_expr = left->llvm_codegen(compiler);
//     Value *right_expr = right->llvm_codegen(compiler);

//     Type *left_type = left_expr->getType();
//     Type *right_type = right_expr->getType();
//     // Type *result_type = nullptr;

    
//     left_type->print(llvm::outs());
//     right_type->print(llvm::outs());
    
//     switch(op) {
//         case PLUS:
//         return compiler->builder.CreateAdd(left_expr, right_expr, "addtmp");
//         case MINUS:
//         return compiler->builder.CreateSub(left_expr, right_expr, "minustmp");
//         case MULT:
//         return compiler->builder.CreateMul(left_expr, right_expr, "multtmp");
//         case DIV:
//         return compiler->builder.CreateSDiv(left_expr, right_expr, "divtmp");
//     }
// }



Value *NodeDecl::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);
    Type *expr_type = expr->getType();
    
    IRBuilder<> temp_builder(
        &MAIN_FUNC->getEntryBlock(),
        MAIN_FUNC->getEntryBlock().begin()
    );

    Type *target_type;
    if (datatype == "short") {
        target_type = compiler->builder.getInt16Ty();
    } else if (datatype == "int") {
        target_type = compiler->builder.getInt32Ty();
    } else {
        target_type = compiler->builder.getInt64Ty();
    }

    // Type checking and coercion
    if (expr_type != target_type) {
        if (expr_type->isIntegerTy(16) && target_type->isIntegerTy(32)) {
            expr = compiler->builder.CreateSExt(expr, compiler->builder.getInt32Ty(), "short_to_int");
        } else if (expr_type->isIntegerTy(16) && target_type->isIntegerTy(64)) {
            expr = compiler->builder.CreateSExt(expr, compiler->builder.getInt64Ty(), "short_to_long");
        } else if (expr_type->isIntegerTy(32) && target_type->isIntegerTy(64)) {
            expr = compiler->builder.CreateSExt(expr, compiler->builder.getInt64Ty(), "int_to_long");
        } else if (expr_type->isIntegerTy(32) && target_type->isIntegerTy(16)) {
            if (ConstantInt *CI = dyn_cast<ConstantInt>(expr)) {
                int32_t val = CI->getSExtValue();
                if (val >= INT16_MIN && val <= INT16_MAX) {
                    expr = compiler->builder.CreateTrunc(expr, compiler->builder.getInt16Ty(), "int_to_short");
                } else {
                    std::cerr << "Type mismatch: the int value " << val << " is out of range for a short variable" << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "Type mismatch: cannot assign a value of type int to a variable of type short without explicit cast" << std::endl;
                exit(1);
            }
        } else if (expr_type->isIntegerTy(64) && target_type->isIntegerTy(32)) {
            if (ConstantInt *CI = dyn_cast<ConstantInt>(expr)) {
                int64_t val = CI->getSExtValue();
                if (val >= INT32_MIN && val <= INT32_MAX) {
                    expr = compiler->builder.CreateTrunc(expr, compiler->builder.getInt32Ty(), "LONG_to_INT");
                } else {
                    std::cerr << "Type mismatch: the long value " << val << " is out of range for a int variable" << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "Type mismatch: cannot assign a value of type long to a variable of type int without explicit cast" << std::endl;
                exit(1);
            }
        } else if (expr_type->isIntegerTy(64) && target_type->isIntegerTy(16)) {
            if (ConstantInt *CI = dyn_cast<ConstantInt>(expr)) {
                int64_t val = CI->getSExtValue();
                if (val >= INT16_MIN && val <= INT16_MAX) {
                    expr = compiler->builder.CreateTrunc(expr, compiler->builder.getInt64Ty(), "long_to_short");
                } else {
                    std::cerr << "Type mismatch: the long value " << val << " is out of range for a short variable" << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "Type mismatch: cannot assign a value of type long to a variable of type short without explicit cast" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "Type mismatch: cannot assign a value of type " << datatype << " to a variable of type " << expr_type->getTypeID() << std::endl;
            exit(1);
        }
    }

    AllocaInst *alloc = temp_builder.CreateAlloca(target_type, 0, identifier);
    compiler->locals[identifier] = alloc;
    return compiler->builder.CreateStore(expr, alloc);
}


Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler) {
    AllocaInst *alloc = compiler->locals[identifier];
    if(datatype == "short") {
        return compiler->builder.CreateLoad(compiler->builder.getInt16Ty(), alloc, identifier);
    } else if (datatype == "int") {
        return compiler->builder.CreateLoad(compiler->builder.getInt32Ty(), alloc, identifier);
    } else {
        return compiler->builder.CreateLoad(compiler->builder.getInt64Ty(), alloc, identifier);
    }
}

Value *NodeAssign::llvm_codegen(LLVMCompiler *compiler) {
    AllocaInst *alloc = compiler->locals[identifier];
    Value *expr = expression->llvm_codegen(compiler);
    compiler->builder.CreateStore(expr, alloc);
    return expr;
}

Value *NodeTernary::llvm_codegen(LLVMCompiler *compiler) {
    Value *cond = condition->llvm_codegen(compiler);
    cond = compiler->builder.CreateICmpNE(
        cond,
        compiler->builder.getInt32(0),
        "ifcond"
    );

    Function *func = compiler->builder.GetInsertBlock()->getParent();

    BasicBlock *then_bb = BasicBlock::Create(*compiler->context, "then", func);
    BasicBlock *else_bb = BasicBlock::Create(*compiler->context, "else");
    BasicBlock *merge_bb = BasicBlock::Create(*compiler->context, "ifcont");

    compiler->builder.CreateCondBr(cond, then_bb, else_bb);

    compiler->builder.SetInsertPoint(then_bb);

    Value *then_expr = then_expression->llvm_codegen(compiler);
    compiler->builder.CreateBr(merge_bb);
    then_bb = compiler->builder.GetInsertBlock();

    func->getBasicBlockList().push_back(else_bb);
    compiler->builder.SetInsertPoint(else_bb);

    Value *else_expr = else_expression->llvm_codegen(compiler);
    compiler->builder.CreateBr(merge_bb);
    else_bb = compiler->builder.GetInsertBlock();

    func->getBasicBlockList().push_back(merge_bb);
    compiler->builder.SetInsertPoint(merge_bb);

    PHINode *phi = compiler->builder.CreatePHI(
        compiler->builder.getInt32Ty(),
        2,
        "iftmp"
    );

    phi->addIncoming(then_expr, then_bb);
    phi->addIncoming(else_expr, else_bb);

    return phi;
}

Value *NodeIfElse::llvm_codegen(LLVMCompiler *compiler) {
    Value *cond = expression->llvm_codegen(compiler);
    cond = compiler->builder.CreateICmpNE(
        cond,
        compiler->builder.getInt32(0),
        "ifcond"
    );

    Function *func = compiler->builder.GetInsertBlock()->getParent();

    BasicBlock *then_bb = BasicBlock::Create(*compiler->context, "then", func);
    BasicBlock *else_bb = BasicBlock::Create(*compiler->context, "else");
    BasicBlock *merge_bb = BasicBlock::Create(*compiler->context, "ifcont");

    //parent block

    compiler->builder.CreateCondBr(cond, then_bb, else_bb);

    compiler->builder.SetInsertPoint(then_bb);
    if_block->llvm_codegen(compiler);

    compiler->builder.CreateBr(merge_bb);

    then_bb = compiler->builder.GetInsertBlock();

    func->getBasicBlockList().push_back(else_bb);
    compiler->builder.SetInsertPoint(else_bb);

    else_block->llvm_codegen(compiler);

    compiler->builder.CreateBr(merge_bb);
    else_bb = compiler->builder.GetInsertBlock();

    func->getBasicBlockList().push_back(merge_bb);
    compiler->builder.SetInsertPoint(merge_bb);

    return nullptr;
}

Value *NodeArgList::llvm_codegen(LLVMCompiler *compiler){
    return nullptr;
}

Value *NodeParameter::llvm_codegen(LLVMCompiler *compiler){
    return nullptr;
}

Value *NodeParameterList::llvm_codegen(LLVMCompiler *compiler){
    return nullptr;
}

Value *NodeFunctionDecl::llvm_codegen(LLVMCompiler *compiler) {
    std::vector<Type *> param_types;

    std::vector<Node *> param_list = ((NodeParameterList *)param_node)->param_list;

    for (auto &par : param_list) {
        NodeParameter *params = (NodeParameter *) par;
        if (params->datatype == "short") {
            param_types.push_back(compiler->builder.getInt16Ty());
        } else if (params->datatype == "int") {
            param_types.push_back(compiler->builder.getInt32Ty());
        } else {
            param_types.push_back(compiler->builder.getInt64Ty());
        }
    }

    Type *return_type;
    if (datatype == "short") {
        return_type = compiler->builder.getInt16Ty();
    } else if (datatype == "int") {
        return_type = compiler->builder.getInt32Ty();
    } else {
        return_type = compiler->builder.getInt64Ty();
    }

    FunctionType *func_type = FunctionType::get(
        return_type,
        param_types,
        false
    );
    Function *func = Function::Create(
        func_type,
        Function::ExternalLinkage,
        identifier,
        compiler->module
    );

    BasicBlock *entry = BasicBlock::Create(*compiler->context, "entry", func);
    compiler->builder.SetInsertPoint(entry);

    // Set up function arguments in the symbol table
    unsigned idx = 0;
    for (auto &arg : func->args()) {
        arg.setName(((NodeParameter*)param_list[idx])->identifier);
        AllocaInst *alloca = compiler->builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
        compiler->builder.CreateStore(&arg, alloca);
        compiler->locals[arg.getName().str()] = alloca;
        idx++;
    }

    // if (Value *retval = block->llvm_codegen(compiler)) {
    //     // Instead of directly creating the return instruction, branch to the return block
    // compiler->builder.CreateBr(returnBlock);

    //     // Set the insert point to the return block and create the return instruction with the return value
    //     compiler->builder.SetInsertPoint(returnBlock);
    //     compiler->builder.CreateRet(retval);
    //     verifyFunction(*func);
    //     return func;
    // }

    block->llvm_codegen(compiler);

    if(!compiler->builder.GetInsertBlock()->getTerminator()){
        // Create a default return value of 0 based on the return type
        if (return_type->isIntegerTy(16)) {
            compiler->builder.CreateRet(compiler->builder.getInt16(0));
        } else if (return_type->isIntegerTy(32)) {
            compiler->builder.CreateRet(compiler->builder.getInt32(0));
        } else if (return_type->isIntegerTy(64)) {
            compiler->builder.CreateRet(compiler->builder.getInt64(0));
        } else {
            // Assuming a default return value for unsupported types
            compiler->builder.CreateRet(compiler->builder.getInt32(0));
        }
    }

    verifyFunction(*func);
    return func;

    // // Error reading body, remove function.
    // func->eraseFromParent();

    // std::cerr<<"Function couldn't be made, function body has errors"<<std::endl;
    // exit(1);
    // return nullptr;
}


Value *NodeReturn::llvm_codegen(LLVMCompiler *compiler) {
    Function *func = compiler->builder.GetInsertBlock()->getParent();

    Type *return_type = func->getReturnType();

    BasicBlock *return_block = BasicBlock::Create(*compiler->context, "return", func);

    compiler->builder.CreateBr(return_block);
    compiler->builder.SetInsertPoint(return_block);


    Value *retval = expression->llvm_codegen(compiler);
    if (!retval) {
        std::cerr << "Error: return expression has no value" << std::endl;
        return nullptr;
    }
    // Instead of directly creating the return instruction, branch to the return block

    compiler->builder.CreateRet(retval);

    
    return retval;
}


Value *NodeFunctionCall::llvm_codegen(LLVMCompiler *compiler) {
    // Look up the function in the module
    Function *func = compiler->module.getFunction(identifier);
    if (!func) {
        std::cerr << "Error: function not found: " << identifier << std::endl;
        return nullptr;
    }

    // Check that the number of arguments matches the function signature
    if (func->arg_size() != ((NodeArgList*)arg_node)->arg_list.size()) {
        std::cerr << "Error: incorrect number of arguments to function call: " << identifier << std::endl;
        return nullptr;
    }

    // Generate code for each argument
    std::vector<Value *> args;
    unsigned idx = 0;
    for (auto &arg : ((NodeArgList*)arg_node)->arg_list) {
        Value *arg_val = arg->llvm_codegen(compiler);
        args.push_back(arg_val);
        if (!args.back()) {
            std::cerr << "Error: failed to generate code for argument to function call: " << identifier << std::endl;
            return nullptr;
        }

        // Perform type checking
        Type *formal_param_type = func->getFunctionType()->getParamType(idx);
        Type *actual_arg_type = arg_val->getType();

        if (actual_arg_type != formal_param_type) {
            if (actual_arg_type->isIntegerTy() && formal_param_type->isIntegerTy()) {
                unsigned actual_arg_bitwidth = actual_arg_type->getIntegerBitWidth();
                unsigned formal_param_bitwidth = formal_param_type->getIntegerBitWidth();

                if (actual_arg_bitwidth > formal_param_bitwidth) {
                    std::cerr << "Error: argument type mismatch in function call: " << identifier
                              << ". The actual argument type is larger than the formal parameter type."
                              << std::endl;
                    return nullptr;
                } else {
                    // If actual argument type is smaller, perform an integer type promotion
                    Value *promoted_arg = compiler->builder.CreateSExt(arg_val, formal_param_type);
                    args.back() = promoted_arg;
                }
            } else {
                std::string formal;
                std::string actual;
                llvm::raw_string_ostream rso1(formal);
                llvm::raw_string_ostream rso2(actual);
                formal_param_type->print(rso1);
                actual_arg_type->print(rso2);
                std::cerr << "Error: argument type mismatch in function call: " << identifier
                          << ". Expected " << rso1.str() << " but got " << rso2.str() << std::endl;
                return nullptr;
            }
        }

        idx++;
    }

    // Call the function
    return compiler->builder.CreateCall(func, args);
}
