/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        1/23/2020
 *              This project is released under the 3-Clause BSD License.
 *
 */


#include "LTLSpecsBuilder.hh"

#include "utilities/Factory.hh"

using namespace chase;
using namespace chase;
using namespace antlr4;

LTLSpecsBuilder::LTLSpecsBuilder() :
    _system(nullptr),
    _currContract(nullptr)
{
}

LTLSpecsBuilder::~LTLSpecsBuilder() = default;



System *LTLSpecsBuilder::getSystem() const {
    return _system;
}

System *LTLSpecsBuilder::parseSpecificationFile(std::string infile)
{
    ANTLRFileStream input( infile );
    LTLContractsLexer lexer( &input );
    CommonTokenStream tokens( &lexer );

    LTLContractsParser parser( &tokens );

    parser.setBuildParseTree(true);
    LTLContractsParser::SystemSpecContext * tree = parser.systemSpec();

    visitSystemSpec(tree);

    return _system;
}

antlrcpp::Any
LTLSpecsBuilder::visitSystemSpec(LTLContractsParser::SystemSpecContext *ctx) {
    LTLContractsParser::NameContext * n_ctx = ctx->name();
    if( n_ctx != nullptr )
    {
        std::string name = n_ctx->ID()->getText();
        messageInfo("Parsing the system: " + name);
        _system = new System(name);
    } else{
        messageInfo("Parsing the system: SYSTEM (default)");
        _system = new System();
    }

    return LTLContractsBaseVisitor::visitSystemSpec(ctx);
}

antlrcpp::Any
LTLSpecsBuilder::visitDeclaration(LTLContractsParser::DeclarationContext *ctx) {
    std::string name = ctx->ID()->getText();

    DataDeclaration * dec = nullptr;
    Type * type = nullptr;

    auto tctx = ctx->typeKW();
    if( tctx != nullptr) {
        if (tctx->booleanKW() != nullptr)
            type = new chase::Boolean();
        else if (tctx->integer() != nullptr) {
            LTLContractsParser::IntegerContext *ictx = tctx->integer();
            int lb = 0;
            int ub = 16;
            if (ictx->range() != nullptr) {
                lb = std::stoi(ictx->range()->NUMBER(0)->getText());
                ub = std::stoi(ictx->range()->NUMBER(1)->getText());
            }
            type = new chase::Integer(lb, ub);
        } else
            messageError("Wrong type: " + ctx->typeKW()->getText());
    }

    if (ctx->variableKW() != nullptr) {
        auto var = new Variable(type, new Name(name));
        if(ctx->causality() != nullptr){
            if(ctx->causality()->inputKW() != nullptr)
                var->setCausality(input);
            if(ctx->causality()->outputKW() != nullptr)
                var->setCausality(output);
        }
        dec = var;
    }
    else if (ctx->constantKW() != nullptr)
    {
        int num = std::stoi(ctx->NUMBER()->getText().c_str());
        dec = new Constant(
                new Integer(),
                new Name(name),
                new IntegerValue(num));
    }
    else if (ctx->propositionKw() != nullptr)
    {
        dec = new Variable(
                new Boolean(), new Name(name));
        if( ctx->relation() != nullptr ) {
            Expression *exp = createRelation(ctx->relation());
            _map_props_values.insert(
                    std::pair<Variable *, Expression *>(
                            reinterpret_cast<Variable *>(dec), exp));
        }
    }
    else
        messageError("Unknown entry:\t" + ctx->getText());

    if( _currContract != nullptr )
        _currContract->addDeclaration(dec);
    else
        _system->addDeclaration(dec);

    return antlrcpp::Any();
}

Expression *
LTLSpecsBuilder::createRelation(LTLContractsParser::RelationContext *ctx)
{
    std::string rel_op = ctx->relation_op()->getText();
    Operator op = op_neq;
    if( rel_op == "!=" ) op = op_neq;
    else if( rel_op == "=" ) op = op_eq;
    else if( rel_op == ">" ) op = op_gt;
    else if( rel_op == "<" ) op = op_lt;
    else if( rel_op == ">=" ) op = op_ge;
    else if( rel_op == "<=" ) op = op_le;
    else messageError("Unsupported relation operator: " + rel_op);

    return new Expression(op,
                          createValue(ctx->lvalue()->value()),
                          createValue(ctx->rvalue()->value()));
}

Value *LTLSpecsBuilder::createValue(LTLContractsParser::ValueContext *ctx)
{
    // The value is terminal.
    if(ctx->children.size() == 1)
    {
        if(ctx->ID() != nullptr)
            return createIdentifier(ctx->ID()->getText());
        if(ctx->value(0))
            return createValue(ctx->value(0));
        if(ctx->primed_ID())
            return createIdentifier(ctx->primed_ID()->ID()->getText(), true);
        if(ctx->NUMBER() || ctx->minus_number())
            return createNumericValue(ctx->getText());
    }

    if(ctx->children.size() == 3)
    {
        if(ctx->children[0]->getText() == "(" &&
        ctx->children[2]->getText() == ")" )
        {
            return createValue(ctx->value(0));
        }

        std::string math_op = ctx->bin_math_op()->getText();
        Operator op = op_plus;
        if(math_op == "+") op = op_plus;
        if(math_op == "-") op = op_minus;
        if(math_op == "*") op = op_multiply;
        if(math_op == "/") op = op_divide;

        return new Expression(op,
                              createValue(ctx->value(0)),
                              createValue(ctx->value(1)));

    }
    return nullptr;
}

Identifier *LTLSpecsBuilder::createIdentifier(std::string name, bool primed)
{
    auto dd = findDeclaration(name);
    if( dd != nullptr )
        return new Identifier(dd, primed);
    else
        messageError(name + "is not declared in the current scope.");
    return nullptr;
}

antlrcpp::Any
LTLSpecsBuilder::visitContract(LTLContractsParser::ContractContext *ctx)
{
    _currContract = new Contract(ctx->ID()->getText());
    _system->addContract(_currContract);

    // Visit the contract.
    for(size_t it = 0; it != ctx->declaration().size(); ++it)
    {
        visitDeclaration(ctx->declaration()[it]);
    }

    visitAssumptions(ctx->assumptions());
    visitGuarantees(ctx->guarantees());

    _currContract = nullptr;
    return antlrcpp::Any();
}


antlrcpp::Any
LTLSpecsBuilder::visitAssumptions(LTLContractsParser::AssumptionsContext *ctx)
{
    size_t i = 0;
    auto vec = new std::vector< LogicFormula * >();
    vec->push_back(createFormula(ctx->single_formula(i)->formula()));
    ++i;

    while(i < ctx->single_formula().size())
    {
        vec->push_back(createFormula(ctx->single_formula(i)->formula()));
        ++i;
    }

    _currContract->addAssumptions(logic, LargeAnd(*vec));
    return antlrcpp::Any();
}


antlrcpp::Any
LTLSpecsBuilder::visitGuarantees(LTLContractsParser::GuaranteesContext *ctx) {
    size_t i = 0;
    auto vec = new std::vector< LogicFormula * >();
    vec->push_back(createFormula(ctx->single_formula(i)->formula()));
    ++i;

    while(i < ctx->single_formula().size())
    {
        vec->push_back(createFormula(ctx->single_formula(i)->formula()));
        ++i;
    }

    _currContract->addGuarantees(logic, LargeAnd(*vec));

    return antlrcpp::Any();
}




LogicFormula *
LTLSpecsBuilder::createFormula(LTLContractsParser::FormulaContext *ctx)
{
    if(ctx->unary_logic_op())
    {
        if(ctx->unary_logic_op()->NOT())
            return Not(createFormula(ctx->formula(0)));
    }
    if(ctx->unary_temp_op())
    {
        UnaryTemporalFormula * lf;
        if(ctx->unary_temp_op()->ALWAYS())
            lf = Always(createFormula(ctx->formula(0)));
        if(ctx->unary_temp_op()->EVENTUALLY())
            lf = Eventually(createFormula(ctx->formula(0)));
        if(ctx->unary_temp_op()->NEXT())
            lf = Next(createFormula(ctx->formula(0)));
        if(ctx->interval())
            lf->setInterval(createInterval(ctx->interval()));
        return lf;
    }
    if(ctx->bin_logic_op())
    {
        if(ctx->bin_logic_op()->AND())
            return And(createFormula(ctx->formula(0)),
                       createFormula(ctx->formula(1)));
        if(ctx->bin_logic_op()->OR())
            return Or(createFormula(ctx->formula(0)),
                      createFormula(ctx->formula(1)));
        if(ctx->bin_logic_op()->IMPLIES())
            return Implies(createFormula(ctx->formula(0)),
                           createFormula(ctx->formula(1)));
        if(ctx->bin_logic_op()->IFF())
            return Iff(createFormula(ctx->formula(0)),
                       createFormula(ctx->formula(1)));
    }
    if(ctx->bin_temp_op())
    {
        BinaryTemporalFormula * lf;
        if(ctx->bin_temp_op()->UNTIL())
            lf = Until(
                    createFormula(ctx->formula(0)),
                    createFormula(ctx->formula(1)));
        if(ctx->interval())
            lf->setInterval(createInterval(ctx->interval()));
        return lf;
    }
    if(ctx->formula(0) != nullptr)
    {
        return createFormula(ctx->formula(0));
    }
    if(ctx->atom() != nullptr)
    {
        if(ctx->atom()->ID())
            return createProposition(ctx->atom()->ID()->getText());
        if(ctx->atom()->primed_ID())
            return createProposition(ctx->atom()->primed_ID()->ID()->getText(), true);
        if(ctx->atom()->logic_constant())
            return createLogicConstant(ctx->atom()->logic_constant());
        if(ctx->atom()->relation())
            return createPropositionFromRelation(ctx->atom()->relation());
    }
    return nullptr;
}

BooleanConstant *LTLSpecsBuilder::createLogicConstant(
        LTLContractsParser::Logic_constantContext *ctx)
{
    return new BooleanConstant(ctx->trueKW() != nullptr);
}

Proposition *LTLSpecsBuilder::createProposition(std::string name, bool primed)
{
    auto v = dynamic_cast<Variable *>(findDeclaration(name));
    if( v == nullptr) {
        messageError(name + " is not declared in the current scope.");
        return nullptr;
    }

    auto prop = new Proposition();
    prop->setName(v->getName()->clone());

    auto p = _map_props_values.find(v);
    if(p != _map_props_values.end())
        prop->setValue(p->second);
    else
        prop->setValue(new Identifier(v, primed));
    return prop;
}

DataDeclaration *LTLSpecsBuilder::findDeclaration(std::string name) {
    if( _currContract != nullptr )
    {
        // search declaration inside contract.
        for(auto & declaration : _currContract->declarations)
        {
            auto currDec = dynamic_cast<DataDeclaration *>(declaration);
            if( currDec == nullptr ) continue;
            if(currDec->getName()->getString() == name )
                return currDec;
        }
    }

    // if it is not in the current contract, search in the global declarations.
    for(auto i : _system->getDeclarationsSet())
    {
        auto currDec = dynamic_cast<DataDeclaration *>(i);
        if( currDec == nullptr ) continue;
        if(currDec->getName()->getString() == name )
            return currDec;
    }

    return nullptr;
}

Proposition *LTLSpecsBuilder::createPropositionFromRelation(
        LTLContractsParser::RelationContext * ctx )
{
    Expression * exp = createRelation(ctx);
    auto proposition = new Proposition(exp);
    return proposition;
}

Interval *LTLSpecsBuilder::createInterval(LTLContractsParser::IntervalContext *ctx) {
    LTLContractsParser::PairContext * pairContext;
    bool leftOpen, rightOpen = false;

    if( ctx->interval_closed() != nullptr )
    {
        leftOpen = false;
        rightOpen = false;
        pairContext = ctx->interval_closed()->pair();
    } else if( ctx->interval_fullopen() != nullptr )
    {
        leftOpen = true;
        rightOpen = true;
        pairContext = ctx->interval_fullopen()->pair();
    } else if( ctx->interval_leftopen() != nullptr )
    {
        leftOpen = true;
        rightOpen = false;
        pairContext = ctx->interval_leftopen()->pair();
    } else if( ctx->interval_rightopen() != nullptr )
    {
        leftOpen = false;
        rightOpen = true;
        pairContext = ctx->interval_rightopen()->pair();
    }
    else
    {
        messageError("Invalid interval.");
    }

    Value * v1 = createValue(pairContext->value(0));
    Value * v2 = createValue(pairContext->value(1));

    return new Interval(v1, v2, leftOpen, rightOpen);
}

NumericValue *LTLSpecsBuilder::createNumericValue(std::string s) {
    bool real = false;
    bool error = false;
    for(size_t i = 0; i < s.length(); ++i)
    {
        if(isdigit(s[i])) continue;
        if(s[i] == '.')
        {
            if(real) error = true;
            else real = true;
        } else error = true;
    }

    if(error == true)
    {
        messageError("Invalid number format: "+ s);
        return nullptr;
    }
    if(real == true) return new RealValue(std::stod(s));
    else return new IntegerValue(std::stoi(s));
}















