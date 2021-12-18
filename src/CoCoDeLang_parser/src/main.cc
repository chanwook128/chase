/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        12/10/2021
 *              This project is released under the 3-Clause BSD License.
 *
 */

#include "main.hh"

using namespace chase;

int main(int argc, char *argv[])
{

    std::vector< Value* > v = {new IntegerValue(4),
                               new IntegerValue(4),
                               new IntegerValue(4),
                               new IntegerValue(4)};

    auto * matrix = new Matrix(2, 2, v);

    std::cout << matrix->getString() << std::endl;
    std::cout << matrix->getType()->getString() << std::endl;

    auto params = parseCmdLine(argc, argv);

    CoCoDeSpecsBuilder builder;
    builder.parseSpecificationFile(params->fileIn);
    // System * system = builder.getSystem();
    //delete system;
    return 1;
}

Params * chase::parseCmdLine(int argc, char **argv) {
    auto *parameters = new Params();

    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "i:c:b:o:V")) != -1) {
        switch (c) {
            case 'i':
                parameters->fileIn = std::string(optarg);
                break;
            case 'c':
                parameters->cmdFile = std::string(optarg);
                break;
            case '?':
                printHelp();
                exit(-1);
            case 'o':
                parameters->outDir = std::string(optarg);
                if(parameters->outDir.back() != '/')
                    parameters->outDir += "/";
                break;
            case 'V':
                parameters->verbose = true;
                break;
            default:
                printHelp();
                exit(-1);
        }
    }

    std::ifstream f(parameters->fileIn.c_str());
    if( ! f.good() )
    {
        printHelp();
        f.close();
        std::cout << "fileIn: " << parameters->fileIn.c_str() << std::endl;
        exit(-1);
    }
    f.close();

    struct stat info;
    if(stat (parameters->outDir.c_str(), &info) != 0 && !parameters->outDir
            .empty()) {
        if (mkdir(parameters->outDir.c_str(), 0755) == -1)
        {
            std::cout << strerror(errno) << std::endl;
            exit(-1);

        } else messageInfo(parameters->outDir + " created.");
    }
    return parameters;
}

void chase::printHelp()
{
    std::cerr << "[USAGE]\n" <<
              "chase -i input_file -c commands_file [-V]"
              << std::endl <<
              std::endl <<
              "\t-i : specifies the txt input file containing the specifications."
              << std::endl <<
              "\t-c : command file to be executed."
              << std::endl <<
              "\t-o : output directory path."
              << std::endl <<
              "\t-V : activate the verbose mode." << std::endl;
}