#include <iostream>
#include <memory>
#include <vector>

#include "dlinfo.hpp"
#include "image.hpp"
#include "mbr.hpp"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sunxi-image");

    try {
        std::vector<std::unique_ptr<subcmd>> cmds;
        cmds.emplace_back(new dlinfo_cmd());
        cmds.emplace_back(new mbr_cmd());
        cmds.emplace_back(new image_cmd());

        for (auto &sub : cmds) {
            program.add_subparser(*sub->cmd);
        }

        program.parse_args(argc, argv);

        for (auto &sub : cmds) {
            if (program.is_subcommand_used(*sub->cmd)) {
                sub->run();
                return 0;
            }
        }
        throw std::runtime_error("missing sub command");

    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }
}