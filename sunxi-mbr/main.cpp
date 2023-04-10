#include "argparse.hpp"
#include "iniparser.h"

#include <cstring>
#include <memory>
#include <vector>

#include "sunxi-image.h"

struct subcmd {
    virtual argparse::ArgumentParser parser() = 0;
    virtual void run(const argparse::ArgumentParser &cmd) = 0;

    argparse::ArgumentParser _p;
};

struct dump_cmd : public subcmd {
    argparse::ArgumentParser parser() {
        argparse::ArgumentParser dump_cmd("dump");
        dump_cmd.add_description("display sunxi-mbr of <image>");
        dump_cmd.add_argument("image").help("image to process.").required();
        return dump_cmd;
    }

    void run(const argparse::ArgumentParser &cmd) {
        auto filename = cmd.get<std::string>("image");

        sunxi_image_t img;
        if (sunxi_image_open(&img, filename.c_str()) != 0) {
            throw std::runtime_error("sunxi_image_open failed");
        }

        std::cout
            << std::setw(16) << "name"
            << std::setw(20) << "start"
            << std::setw(20) << "size"
            << std::setw(16) << "class"
            << std::endl;
        for (size_t i = 0; i < img.mbr.part_count; i++) {
            auto part = &img.mbr.array[i];
            std::cout
                << std::setw(16) << part->name
                << std::setw(20) << part->addrlo * SUNXI_MBR_PAGE_SIZE
                << std::setw(20) << part->lenlo * SUNXI_MBR_PAGE_SIZE
                << std::setw(16) << part->classname
                << std::endl;
        }

        sunxi_image_close(&img);
    }
};

struct patch_cmd : public subcmd {
    argparse::ArgumentParser parser() {
        argparse::ArgumentParser patch_cmd("patch");
        patch_cmd.add_description("override <part> with <file> of <image>");
        patch_cmd.add_argument("image").help("image to process.").required();
        patch_cmd.add_argument("part").help("part to patch.").required();
        patch_cmd.add_argument("file").help("file to write.").required();
        return patch_cmd;
    }

    void run(const argparse::ArgumentParser &cmd) {
        auto filename = cmd.get<std::string>("image");
        auto part = cmd.get<std::string>("part");
        auto file = cmd.get<std::string>("file");

        sunxi_image_t img;
        if (sunxi_image_open(&img, filename.c_str()) != 0) {
            throw std::runtime_error("sunxi_image_open failed");
        }

        if (sunxi_image_patch_part(&img, part.c_str(), file.c_str()) != 0) {
            sunxi_image_close(&img);
            throw std::runtime_error("sunxi_image_patch_part failed");
        }

        sunxi_image_close(&img);
    }
};

struct gen_cmd : public subcmd {
    argparse::ArgumentParser parser() {
        argparse::ArgumentParser gen_cmd("gen");
        gen_cmd.add_description("generate mbr from ini");
        gen_cmd.add_argument("mbr").help("mbr file to generate.").required();
        gen_cmd.add_argument("ini").help("ini file to process.").required();
        return gen_cmd;
    }

    uint32_t calc_crc32(uint8_t *buf, uint32_t size) {
        uint32_t table[256];
        for (size_t i = 0; i < 256; i++) {
            uint32_t val = i;
            for (size_t j = 0; j < 8; j++) {
                if (val & 1) {
                    val = (val >> 1) ^ 0xEDB88320;
                } else {
                    val >>= 1;
                }
            }
            table[i] = val;
        }

        uint32_t crc = 0xffffffff;
        for (size_t i = 0; i < size; i++) {
            crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
        }
        return crc ^ 0xffffffff;
    }

    void run(const argparse::ArgumentParser &cmd) {

        auto ini_filename = cmd.get<std::string>("ini");
        iniparser ini(ini_filename);

        sunxi_mbr_t mbr;
        memset(&mbr, 0, sizeof(sunxi_mbr_t));

        mbr.version = 0x00000200;
        memcpy(mbr.magic, SUNXI_MBR_MAGIC, 8);
        mbr.copy = 1;

        size_t index = 0;
        size_t offset = SUNXI_MBR_SIZE / 512;
        for (auto &part : ini.get("partition")) {
            auto name = part.at("name");

            std::cout << "adding " << name << " @ " << offset << std::endl;

            strcpy((char *)mbr.array[index].name, name.c_str());
            strcpy((char *)mbr.array[index].classname, "DISK");

            auto user_type = std::stoi(part.at("user_type"));
            mbr.array[index].user_type = user_type;

            auto size = std::stoi(part.at("size"));
            mbr.array[index].lenlo = size;
            mbr.array[index].addrlo = offset;

            offset += size;
            index++;
        }
        mbr.part_count = index;
        mbr.crc32 = calc_crc32((uint8_t *)(&mbr) + 4, (sizeof(sunxi_mbr_t) - 4));

        auto mbr_filename = cmd.get<std::string>("mbr");
        std::fstream file(mbr_filename, std::ios::binary | std::ios::out | std::ios::trunc);
        file.write((char *)&mbr, SUNXI_MBR_SIZE);
    }
};

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sunxi-mbr");

    std::vector<std::unique_ptr<subcmd>> cmds;
    cmds.emplace_back(new dump_cmd());
    cmds.emplace_back(new patch_cmd());
    cmds.emplace_back(new gen_cmd());

    for (auto &cmd : cmds) {
        cmd->_p = cmd->parser();
        program.add_subparser(cmd->_p);
    }

    try {
        program.parse_args(argc, argv);

        for (auto &cmd : cmds) {
            if (program.is_subcommand_used(cmd->_p)) {
                cmd->run(cmd->_p);
                return 0;
            }
        }
        throw new std::runtime_error("missing sub command");

    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }
}
