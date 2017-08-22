//
//  ASTUnary.hpp
//  Emojicode
//
//  Created by Theo Weidmann on 04/08/2017.
//  Copyright © 2017 Theo Weidmann. All rights reserved.
//

#ifndef ASTUnary_hpp
#define ASTUnary_hpp

#include "ASTExpr.hpp"
#include "../Generation/FnCodeGenerator.hpp"

namespace EmojicodeCompiler {

class ASTUnary : public ASTExpr {
public:
    ASTUnary(const std::shared_ptr<ASTExpr> value, const SourcePosition &p) : ASTExpr(p), value_(value) {}
protected:
    void generateHelper(FnCodeGenerator *fncg, EmojicodeInstruction instruction) const {
        value_->generate(fncg);
        fncg->wr().writeInstruction(instruction);
    }
    std::shared_ptr<ASTExpr> value_;
};

class ASTIsNothigness final : public ASTUnary {
    using ASTUnary::ASTUnary;
public:
    Type analyse(SemanticAnalyser *analyser, const TypeExpectation &expectation) override;
    void generateExpr(FnCodeGenerator *fncg) const override;
};

class ASTIsError final : public ASTUnary {
    using ASTUnary::ASTUnary;
public:
    Type analyse(SemanticAnalyser *analyser, const TypeExpectation &expectation) override;
    void generateExpr(FnCodeGenerator *fncg) const override;
};

class ASTUnwrap final : public ASTUnary {
    using ASTUnary::ASTUnary;
public:
    Type analyse(SemanticAnalyser *analyser, const TypeExpectation &expectation) override;
    void generateExpr(FnCodeGenerator *fncg) const override;
private:
    bool error_ = false;
};

class ASTMetaTypeFromInstance final : public ASTUnary {
    using ASTUnary::ASTUnary;
public:
    Type analyse(SemanticAnalyser *analyser, const TypeExpectation &expectation) override;
    void generateExpr(FnCodeGenerator *fncg) const override;
};

}

#endif /* ASTUnary_hpp */
