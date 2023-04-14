#pragma once

#include <memory>
#include <vector>

#include "cli.hpp"
#include "sunxi-part.h"

class sunxi_mbr {
  public:
    sunxi_mbr(const std::string _path);

    void read();
    void write();

    void add_file(
        const std::string &name,
        const std::string &classname,
        uint32_t user_type);

    sunxi_mbr_t header;

  private:
    std::string path;
};

struct mbr_cmd : public subcmd {
    mbr_cmd();
    void run();

    std::vector<std::unique_ptr<subcmd>> cmds;
};