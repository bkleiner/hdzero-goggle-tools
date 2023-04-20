#include "dlinfo.hpp"

#include <cstring>
#include <fstream>

#include "json.hpp"

#include "config.h"
#include "util.hpp"

sunxi_dlinfo::sunxi_dlinfo(const std::string _path)
    : path(_path) {

    header.crc32 = 0x0; // updated later
    header.version = 0x200;
    memcpy(header.magic, SUNXI_MAGIC, 8);
    header.download_count = 0x0; // updated later
}

void sunxi_dlinfo::read() {
    std::fstream file(path, std::ios_base::binary | std::ios_base::in);

    file.read((char *)&header, sizeof(sunxi_dl_t));
    if (memcmp(header.magic, SUNXI_MAGIC, 8) != 0) {
        throw std::runtime_error("invalid magic");
    }
}

void sunxi_dlinfo::add_file(
    const std::string &name,
    const std::string &dl_name,
    const std::string &vf_name,
    bool encrypt,
    bool verify) {
    const uint64_t part_size = MEM_ALIGN(std::filesystem::file_size(name + ".fex"), PART_ALIGN_SIZE) / SUNXI_PAGE_SIZE;

    auto part = &header.array[header.download_count];
    insert_string(part->name, name, 16);
    part->lenhi = (part_size >> 32) & 0xffffffff;
    part->lenlo = (part_size >> 0) & 0xffffffff;
    insert_string(part->dl_name, dl_name, 16);
    insert_string(part->vf_name, vf_name, 16);
    part->encrypt = encrypt;
    part->verify = verify;

    header.download_count++;
}

void sunxi_dlinfo::write() {
    uint64_t offset = SUNXI_DL_SIZE / SUNXI_PAGE_SIZE;
    for (size_t i = 0; i < header.download_count; i++) {
        auto part = &header.array[i];

        part->addrhi = (offset >> 32) & 0xffffffff;
        part->addrlo = (offset >> 0) & 0xffffffff;

        const uint64_t size = (uint64_t)(part->lenhi) << 32 | part->lenlo;
        offset += size;
    }
    header.crc32 = crc32_calc((uint8_t *)(&header) + 4, sizeof(sunxi_dl_t) - 4);

    std::vector<char> buf(SUNXI_DL_SIZE);
    memcpy(buf.data(), &header, sizeof(sunxi_dl_t));

    std::fstream file(path, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    file.write(buf.data(), buf.size());
}

struct dlinfo_dump_cmd : public subcmd {
    dlinfo_dump_cmd() {
        cmd = new argparse::ArgumentParser("dump");
        cmd->add_description("dump dlinfo");
        cmd->add_argument("file").help("dlinfo file to process.").required();
    }

    void run() {
        auto filename = cmd->get<std::string>("file");

        sunxi_dlinfo dlinfo(filename);
        dlinfo.read();

        nlohmann::json json;
        for (size_t i = 0; i < dlinfo.header.download_count; i++) {
            auto part = &dlinfo.header.array[i];

            nlohmann::json entry;
            entry["name"] = extract_string((const char *)part->name, 16);
            entry["dl_name"] = extract_string((const char *)part->dl_name, 16);
            entry["vf_name"] = extract_string((const char *)part->vf_name, 16);
            entry["encrypt"] = part->encrypt > 0;
            entry["verify"] = part->verify > 0;
            json.push_back(entry);
        }

        std::cout << json.dump() << std::endl;
    }
};

struct dlinfo_gen_cmd : public subcmd {
    dlinfo_gen_cmd() {
        cmd = new argparse::ArgumentParser("gen");
        cmd->add_description("generate dlinfo");
        cmd->add_argument("config").help("dlinfo config file to process.").required();
        cmd->add_argument("file").help("dlinfo file to generate.").required();
    }

    void run() {
        auto image_path = cmd->get<std::string>("file");
        auto config_path = cmd->get<std::string>("config");

        std::ifstream config_file(config_path);
        nlohmann::json json = nlohmann::json::parse(config_file);

        sunxi_dlinfo dlinfo(image_path);
        for (auto &el : json) {
            std::cout << "adding " << el.dump() << std::endl;
            dlinfo.add_file(el["name"],
                            el["dl_name"],
                            el["vf_name"],
                            el["encrypt"],
                            el["verify"]);
        }
        dlinfo.write();
    }
};

dlinfo_cmd::dlinfo_cmd() {
    cmd = new argparse::ArgumentParser("dlinfo");
    cmd->add_description("handle dlinfo");

    cmds.emplace_back(new dlinfo_dump_cmd());
    cmds.emplace_back(new dlinfo_gen_cmd());

    for (auto &sub : cmds) {
        cmd->add_subparser(*sub->cmd);
    }
}

void dlinfo_cmd::run() {
    for (auto &sub : cmds) {
        if (cmd->is_subcommand_used(*sub->cmd)) {
            return sub->run();
        }
    }
    throw std::runtime_error("missing sub command");
}