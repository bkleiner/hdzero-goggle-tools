#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cli.hpp"
#include "sunxi-image.h"

class sunxi_image {
  public:
    sunxi_image(const std::string _path);

    void read();
    void write();

    void dump_files();
    void add_file(std::string name, std::string maintype, std::string subtype);

    bool is_v3();

    imagewty_header_t header;
    std::vector<imagewty_file_header_t> file_headers;

  private:
    void finish_header();

    std::string path;
};

struct image_cmd : public subcmd {
    image_cmd();
    void run();

    std::vector<std::unique_ptr<subcmd>> cmds;
};