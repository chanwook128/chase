/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        11/16/2020
 *              This project is released under the 3-Clause BSD License.
 *
 */


#pragma once

#include "representation.hh"
#include "GuideVisitor.hh"
#include "representation/Operators.hh"
#include <fstream>


namespace chase {

    class NuSMVPrinter : public GuideVisitor {
    public:
        /// @brief Constructor.
        NuSMVPrinter();

        /// @brief Destructor.
        ~NuSMVPrinter();

        /// @brief Function printing on the file.
        /// @param contract The contract to be printed.
        /// @param path The path of the output file.
        void print(Contract * contract, std::string path);


        /// @cond
        // Visit functions.
        int visitBooleanValue(BooleanValue &value) override;
        int visitIdentifier(Identifier &identifier) override;
        int visitVariable(Variable &variable) override;
        int visitProposition(Proposition &proposition) override;
        int visitBinaryBooleanOperation(BinaryBooleanFormula &formula) override;
        int visitUnaryBooleanOperation(UnaryBooleanFormula &formula) override;
        int visitLargeBooleanFormula(LargeBooleanFormula &formula) override;
        int visitUnaryTemporalOperation(UnaryTemporalFormula &formula) override;
        int visitBinaryTemporalOperation(BinaryTemporalFormula &formula) override;
        /// @endcond

    protected:

        /// @brief The contract to be printed.
        Contract * _contract;
        /// @brief The file to print.
        std::ofstream _fout;

        /// @brief Procedure printing the variables declarations.
        void _printDeclarations();

        /// @brief Procedure printing the LTL for the compatibility operation.
        void _printCompatibility();

        /// @brief Procedure printing the LTL for the consistency operation.
        void _printConsistency();

        /// @brief Procedure to print boolean operators.
        /// @param op Boolean operator to be printed.
        /// @return The string corresponding to the NuSMV operator.
        std::string _printBooleanOperator(BooleanOperator op);

        /// @brief Procedure to print temporal operators.
        /// @param op Temporal operator to be printed.
        /// @return The string corresponding to the NuSMV operator.
        std::string _printTemporalOperator(TemporalOperator op);
    };

}
