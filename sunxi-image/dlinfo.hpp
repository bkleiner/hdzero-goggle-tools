#pragma once

#include <memory>
#include <vector>

#include "cli.hpp"
#include "sunxi-part.h"

class sunxi_dlinfo {
  public:
    sunxi_dlinfo(const std::string _path);

    void read();
    void write();

    void add_file(
        const std::string &name,
        const std::string &dl_name,
        const std::string &vf_name,
        bool encrypt,
        bool verify);

    sunxi_dl_t header;

  private:
    std::string path;
};

struct dlinfo_cmd : public subcmd {
    dlinfo_cmd();
    void run();

    std::vector<std::unique_ptr<subcmd>> cmds;
};