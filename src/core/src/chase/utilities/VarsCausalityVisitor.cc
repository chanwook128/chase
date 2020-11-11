/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        1/23/2020
 *              This project is released under the 3-Clause BSD License.
 *
 */

#include "utilities/VarsCausalityVisitor.hh"
#include "representation.hh"

chase::VarsCausalityVisitor::VarsCausalityVisitor( Contract * contract = nullptr ) :
    _contract(contract),
    _inAssumptions(false),
    _inGuarantees(false)
{
}

chase::VarsCausalityVisitor::~VarsCausalityVisitor() = default;

int chase::VarsCausalityVisitor::visitVariable(chase::Variable &variable) {
    std::pair< Variable*, bool > p( &variable, false);
    used_in_assumptions.insert(p);
    return GuideVisitor::visitVariable(variable);
}

int chase::VarsCausalityVisitor::visitContract(chase::Contract & contract) {
    int rv = 0;
    for(auto & declaration : contract.declarations)
        rv |= declaration->accept_visitor(*this);

    _inAssumptions = true;
    for(auto & assumption : contract.assumptions)
        rv |= assumption.second->accept_visitor(*this);
    _inAssumptions = false;

    _inGuarantees = true;
    for(auto & guarantee : contract.guarantees)
        rv |= guarantee.second->accept_visitor(*this);
    _inGuarantees = false;

    _fixVarsCausality();

    return rv;
}

chase::Contract *chase::VarsCausalityVisitor::getContract() const {
    return _contract;
}

void chase::VarsCausalityVisitor::setContract(chase::Contract *contract) {
    _contract = contract;
}

int chase::VarsCausalityVisitor::visitIdentifier(chase::Identifier &identifier) {
    auto d = identifier.getDeclaration();
    if(d->IsA() == variable_node)
    {
        auto v = reinterpret_cast< Variable * >(d);
        if(v->getCausality() == output && _inAssumptions)
            messageWarning("System variable in assumptions: " + v->getName()->getString());
        if(_inAssumptions)
        {
            auto p = used_in_assumptions.find(v);
            if(p != used_in_assumptions.end()) p->second = true;
        }
        if(_inGuarantees)
        {
            auto p = used_in_guarantees.find(v);
            if(p != used_in_guarantees.end()) p->second = true;
        }
    }
    return GuideVisitor::visitIdentifier(identifier);
}

void chase::VarsCausalityVisitor::_fixVarsCausality() {
    for( auto & dec : _contract->declarations )
    {
        if(dec->IsA() == variable_node)
        {
            auto v = reinterpret_cast< Variable * >(dec);
            if(v->getCausality() == input || v->getCausality() == output)
                continue;

            bool used_in_A = used_in_assumptions.find(v)->second;

            /// \todo Integrate the check to identify unused variables.
            if (used_in_A) v->setCausality(input);
            else v->setCausality(output);
        }
    }
}

