//
//  CallCodeGenerator.cpp
//  Emojicode
//
//  Created by Theo Weidmann on 05/08/2017.
//  Copyright © 2017 Theo Weidmann. All rights reserved.
//

#include "CallCodeGenerator.hpp"
#include "AST/ASTExpr.hpp"
#include "FunctionCodeGenerator.hpp"
#include "Functions/Initializer.hpp"
#include "Types/Protocol.hpp"
#include "Types/TypeDefinition.hpp"
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>

namespace EmojicodeCompiler {

llvm::Value * CallCodeGenerator::generate(llvm::Value *callee, const Type &calleeType, const ASTArguments &args,
                                          const std::u32string &name) {
    std::vector<Value *> argsVector;
    if (callType_ != CallType::StaticContextfreeDispatch) {
        argsVector.emplace_back(callee);
    }
    for (auto &arg : args.parameters()) {
        argsVector.emplace_back(arg->generate(fg_));
    }
    return createCall(argsVector, calleeType, name, args.isImperative(), args.genericArguments());
}

Function* CallCodeGenerator::lookupFunction(const Type &type, const std::u32string &name, bool imperative) {
    return type.typeDefinition()->lookupMethod(name, imperative);
}

llvm::Value * CallCodeGenerator::createCall(const std::vector<Value *> &args, const Type &type,
                                            const std::u32string &name, bool imperative,
                                            const std::vector<Type> &genericArguments) {
    auto function = lookupFunction(type, name, imperative);
    switch (callType_) {
        case CallType::StaticContextfreeDispatch:
        case CallType::StaticDispatch:
            return fg_->builder().CreateCall(function->reificationFor(genericArguments).function, args);
        case CallType::DynamicDispatch:
        case CallType::DynamicDispatchMeta:
            assert(type.type() == TypeType::Class);
            return createDynamicDispatch(function, args, genericArguments);
        case CallType::DynamicProtocolDispatch:
            assert(type.type() == TypeType::Protocol);
            return createDynamicProtocolDispatch(function, args, type, genericArguments);
        case CallType::None:
            throw std::domain_error("CallType::None is not a valid call type");
    }
}

llvm::Value * CallCodeGenerator::dispatchFromVirtualTable(Function *function, llvm::Value *virtualTable,
                                                          const std::vector<llvm::Value *> &args,
                                                          const std::vector<Type> &genericArguments) {
    auto reification = function->reificationFor(genericArguments);
    auto id = fg()->int32(reification.vti());
    auto dispatchedFunc = fg()->builder().CreateLoad(fg()->builder().CreateGEP(virtualTable, id));

    std::vector<llvm::Type *> argTypes = reification.functionType()->params();
    if (callType_ == CallType::DynamicProtocolDispatch) {
        argTypes.front() = llvm::Type::getInt8PtrTy(fg()->generator()->context());
    }

    auto funcType = llvm::FunctionType::get(reification.functionType()->getReturnType(), argTypes, false);
    auto func = fg()->builder().CreateBitCast(dispatchedFunc, funcType->getPointerTo(), "dispatchFunc");
    return fg_->builder().CreateCall(funcType, func, args);
}

llvm::Value * CallCodeGenerator::createDynamicDispatch(Function *function, const std::vector<llvm::Value *> &args,
                                                       const std::vector<Type> &genericArgs) {
    auto meta = callType_ == CallType::DynamicDispatchMeta ? args.front() : fg()->getMetaFromObject(args.front());

    std::vector<Value *> idx2{ fg()->int32(0), fg()->int32(1) };  // classMeta.table
    auto table = fg()->builder().CreateLoad(fg()->builder().CreateGEP(meta, idx2), "table");
    return dispatchFromVirtualTable(function, table, args, genericArgs);
}

llvm::Value * CallCodeGenerator::createDynamicProtocolDispatch(Function *function, std::vector<llvm::Value *> args,
                                                               const Type &calleeType,
                                                               const std::vector<Type> &genericArgs) {
    auto valueTypeMeta = fg()->builder().CreateLoad(fg()->getMetaTypePtr(args.front()));
    auto i8PtrType = llvm::Type::getInt8Ty(fg()->generator()->context())->getPointerTo()->getPointerTo();
    auto protocolTable = fg()->builder().CreateBitCast(valueTypeMeta, i8PtrType, "protocolTable");
    args.front() = fg()->builder().CreateLoad(fg()->getValuePtr(args.front(), llvm::Type::getInt8PtrTy(fg()->generator()->context())->getPointerTo()));
    return dispatchFromVirtualTable(function, protocolTable, args, genericArgs);
}

Function* TypeMethodCallCodeGenerator::lookupFunction(const Type &type, const std::u32string &name, bool imperative) {
    return type.typeDefinition()->lookupTypeMethod(name, imperative);
}

Function* InitializationCallCodeGenerator::lookupFunction(const Type &type, const std::u32string &name, bool imperative) {
    return type.typeDefinition()->lookupInitializer(name);
}

}  // namespace EmojicodeCompiler
