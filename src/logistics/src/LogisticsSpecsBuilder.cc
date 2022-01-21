/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        1/18/2022
 *              This project is released under the 3-Clause BSD License.
 *
 */

#include <utility>

#include "../include/LogisticsSpecsBuilder.hh"
#include "utilities/Factory.hh"

using namespace chase;
using namespace antlr4;

Position::Position(
        unsigned xpos, unsigned ypos, unsigned quantity) :
        xpos(xpos), ypos(ypos), quantity(quantity) {}
Position::~Position() = default;

Destination::Destination(std::string name, unsigned time) :
        name(std::move(name)), time(time) {}
Destination::~Destination() = default;

LogisticsSpecsBuilder::LogisticsSpecsBuilder() :
    map_lines(0),
    map_columns(0),
    warehouse(nullptr)
{
}

LogisticsSpecsBuilder::~LogisticsSpecsBuilder() = default;

void LogisticsSpecsBuilder::parseSpecificationFile(const std::string& infile) {
    ANTLRFileStream input( infile );
    LogisticsLangLexer lexer( &input );
    CommonTokenStream tokens( &lexer );

    LogisticsLangParser parser( &tokens );

    parser.setBuildParseTree(true);
    auto tree = parser.spec();

    messageInfo("Phase 1: parsing of the specification.");
    visitSpec(tree);

    messageInfo("Phase 2: build facility model.");
    warehouse = buildWarehouseModel();

}

antlrcpp::Any LogisticsSpecsBuilder::visitMap(
        LogisticsLangParser::MapContext *ctx) {

    map_lines = ctx->MAPLINE().size();
    map_columns = 0;
    for(auto i : ctx->MAPLINE()) {
        std::string line = i->toString();
        unsigned linesize = line.size();
        if(map_columns == 0) map_columns = linesize;
        if(linesize != map_columns)
            messageError("Ill-formed map.");
        asciimap.push_back(line);
    }
    return LogisticsLangBaseVisitor::visitMap(ctx);
}

antlrcpp::Any LogisticsSpecsBuilder::visitProduct(
        LogisticsLangParser::ProductContext *ctx) {
    std::string name(ctx->ID()->toString());

    std::pair< std::string, std::vector< Position * > > p;
    p.first = name;

    for( auto it : ctx->triple() )
    {
        char * pEnd;
        unsigned xpos = std::strtoul(
                it->xpos()->NUMBER()->getText().c_str(), &pEnd, 10);
        unsigned ypos = std::strtoul(
                it->ypos()->NUMBER()->getText().c_str(), &pEnd, 10);
        unsigned units = std::strtoul(
                it->units()->NUMBER()->getText().c_str(), &pEnd, 10);
    }

    products.insert(p);
    return LogisticsLangBaseVisitor::visitProduct(ctx);
}

antlrcpp::Any
LogisticsSpecsBuilder::visitDestination(
        LogisticsLangParser::DestinationContext *ctx) {
    std::string name(ctx->location()->ID()->getText());
    char * pEnd;
    unsigned time(
            std::strtoul(
                    ctx->time()->NUMBER()->getText().c_str(),
                    &pEnd, 10));
    auto destination = new Destination(name, time);

    for(auto it : ctx->request())
    {
        unsigned req = std::strtoul(
                it->NUMBER()->toString().c_str(), &pEnd, 10);
        std::pair< std::string, unsigned > p(it->ID()->getText(), req);
        destination->requests.insert(p);
    }

    return LogisticsLangBaseVisitor::visitDestination(ctx);
}




