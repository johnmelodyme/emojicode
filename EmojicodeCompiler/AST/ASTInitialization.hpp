//
//  ASTInitialization.hpp
//  Emojicode
//
//  Created by Theo Weidmann on 07/08/2017.
//  Copyright © 2017 Theo Weidmann. All rights reserved.
//

#ifndef ASTInitialization_hpp
#define ASTInitialization_hpp

#include "ASTExpr.hpp"

namespace EmojicodeCompiler {

class ASTVTInitDest final : public ASTExpr {
public:
    ASTVTInitDest(VariableID varId, bool inInstanceScope, bool declare, const Type &exprType, const SourcePosition &p)
    : ASTExpr(p), inInstanceScope_(inInstanceScope), varId_(varId), declare_(declare) {
        setExpressionType(exprType);
    }

    void generateExpr(FnCodeGenerator *fncg) const override;
    void initialize(FnCodeGenerator *fncg);

    Type analyse(SemanticAnalyser *, const TypeExpectation &) override { return expressionType(); }
private:
    CGScoper& scoper(FnCodeGenerator *fncg) const;

    bool inInstanceScope_;
    VariableID varId_;
    bool declare_;
};

class ASTInitialization final : public ASTExpr {
public:
    enum class InitType {
        Enum, ValueType, Class
    };

    ASTInitialization(const EmojicodeString &name, const std::shared_ptr<ASTTypeExpr> &type,
                      const ASTArguments &args, const SourcePosition &p)
    : ASTExpr(p), name_(name), typeExpr_(type), args_(args) {}
    Type analyse(SemanticAnalyser *analyser, const TypeExpectation &expectation) override;
    void generateExpr(FnCodeGenerator *fncg) const override;

    void setDestination(const std::shared_ptr<ASTVTInitDest> &vtInitable) { vtDestination_ = vtInitable; }
    InitType initType() { return initType_; }
private:
    InitType initType_ = InitType::Class;
    EmojicodeString name_;
    std::shared_ptr<ASTTypeExpr> typeExpr_;
    std::shared_ptr<ASTVTInitDest> vtDestination_;
    ASTArguments args_;
};

}

#endif /* ASTInitialization_hpp */
