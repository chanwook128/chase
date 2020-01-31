/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        10/31/2019
 *              This project is released under the 3-Clause BSD License.
 *
 */
#include <utility>

#include "chase/representation/Contract.hh"
#include "chase/utilities/ClonedDeclarationVisitor.hh"

using namespace chase;

Contract::Contract( std::string name ) :
    _name(new Name(name))
{
    _node_type = contract_node;
}

Contract::~Contract() = default;

int Contract::accept_visitor(chase::BaseVisitor &v) {
    return v.visitContract(*this);
}

std::string Contract::getString() {
    std::string ret("Contract:\n");
    ret += _name->getString();

    ret += "\nDeclarations:\n";
    std::list< Declaration * >::iterator dit;
    for(dit = declarations.begin(); dit != declarations.end(); ++dit )
    {
        Declaration * d = *dit;
        ret += d->getString();
        ret += "\n";
    }

    ret += "Assumptions:\n";
    std::map< semantic_domain, Specification * >::iterator sit;
    for(sit = assumptions.begin(); sit != assumptions.end(); ++sit)
    {

        Specification * s = (*sit).second;
        ret += s->getString();
        ret += "\n";
    }

    ret += "Guarantees:\n";
    for(sit = guarantees.begin(); sit != guarantees.end(); ++sit)
    {
        Specification * s = (*sit).second;
        ret += s->getString();
        ret += "\n";
    }

    ret += "=======================";

    return ret;
}

Name * Contract::getName() const {
    return _name;
}

void Contract::setName(Name * name) {
    _name = name;
}

void Contract::addDeclaration(Declaration *declaration) {
    declarations.push_back(declaration);
    declaration->setParent(this);
}

void Contract::addAssumptions(semantic_domain domain, Specification *spec) {
    std::pair< semantic_domain, Specification * > a(domain, spec);
    assumptions.insert(a);
    spec->setParent(this);
}

void Contract::addGuarantees(semantic_domain domain, Specification *spec) {
    std::pair< semantic_domain, Specification * > g(domain, spec);
    guarantees.insert(g);
    spec->setParent(this);
}



/// \todo Implement the clone method.
Contract *Contract::clone() {
    auto ret = new Contract(_name->getString());

    // Corresponences maps.
    std::map< Declaration *, Declaration * > declaration_map;

    // Declarations.
    for(auto it = declarations.begin(); it != declarations.end(); ++it)
    {
        Declaration * current = *it;
        auto dec = current->clone();
        std::pair< Declaration *, Declaration * > p(current, dec);
        ret->addDeclaration(dec);
        declaration_map.insert(p);
    }

    // Assumptions.
    for(auto it = assumptions.begin(); it != assumptions.end(); ++it)
    {
        auto spec = (*it).second->clone();
        ret->addAssumptions((*it).first, spec);
    }

    // Guarantees.
    for(auto it = guarantees.begin(); it != guarantees.end(); ++it)
    {
        auto spec = (*it).second->clone();
        ret->addGuarantees((*it).first, spec);
    }

    ClonedDeclarationVisitor c(declaration_map);
    ret->accept_visitor(c);

    return ret;
}
