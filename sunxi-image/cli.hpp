#pragma once

#include "argparse.hpp"

struct subcmd {
    virtual void run() = 0;

    argparse::ArgumentParser *cmd;
};