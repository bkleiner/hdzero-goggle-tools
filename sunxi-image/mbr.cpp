#include "mbr.hpp"

#include <cstring>
#include <fstream>

#include "json.hpp"

#include "util.hpp"

sunxi_mbr::sunxi_mbr(const std::string _path)
    : path(_path) {

    header.crc32 = 0x0; // updated later
    header.version = 0x200;
    memcpy(header.magic, SUNXI_MAGIC, 8);
    header.part_count = 0x0; // updated later
    header.copy = 1;
}

void sunxi_mbr::read() {
    std::fstream file(path, std::ios_base::binary | std::ios_base::in);

    file.read((char *)&header, sizeof(sunxi_mbr_t));
    if (memcmp(header.magic, SUNXI_MAGIC, 8) != 0) {
        throw std::runtime_error("invalid magic");
    }
}

void sunxi_mbr::add_file(
    const std::string &name,
    const std::string &classname,
    uint32_t user_type) {

    auto part = &header.array[header.part_count];
    insert_string(part->name, name, 16);
    insert_string(part->classname, classname, 16);
    if (user_type != 0x8100) {
        const uint64_t part_size = MEM_ALIGN(std::filesystem::file_size(name + ".fex"), SUNXI_PAGE_SIZE) / SUNXI_PAGE_SIZE;
        part->lenhi = (part_size >> 32) & 0xffffffff;
        part->lenlo = (part_size >> 0) & 0xffffffff;
    }
    part->user_type = user_type;

    header.part_count++;
}

void sunxi_mbr::write() {
    uint64_t offset = SUNXI_MBR_SIZE / SUNXI_PAGE_SIZE;
    for (size_t i = 0; i < header.part_count; i++) {
        auto part = &header.array[i];

        part->addrhi = (offset >> 32) & 0xffffffff;
        part->addrlo = (offset >> 0) & 0xffffffff;

        const uint64_t size = (uint64_t)(part->lenhi) << 32 | part->lenlo;
        offset += size;
    }
    header.crc32 = crc32_calc((uint8_t *)(&header) + 4, sizeof(sunxi_mbr_t) - 4);

    std::vector<char> buf(SUNXI_MBR_SIZE);
    memcpy(buf.data(), &header, sizeof(sunxi_mbr_t));

    std::fstream file(path, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    file.write(buf.data(), buf.size());
}

struct mbr_dump_cmd : public subcmd {
    mbr_dump_cmd() {
        cmd = new argparse::ArgumentParser("dump");
        cmd->add_description("dump mbr");
        cmd->add_argument("file").help("mbr file to process.").required();
    }

    void run() {
        auto filename = cmd->get<std::string>("file");

        sunxi_mbr mbr(filename);
        mbr.read();

        nlohmann::json json;
        for (size_t i = 0; i < mbr.header.part_count; i++) {
            auto part = &mbr.header.array[i];
            auto user_type = part->user_type;

            nlohmann::json entry;
            entry["name"] = extract_string((const char *)part->name, 16);
            entry["classname"] = extract_string((const char *)part->classname, 16);
            entry["user_type"] = user_type;
            json.push_back(entry);
        }

        std::cout << json.dump() << std::endl;
    }
};

struct mbr_gen_cmd : public subcmd {
    mbr_gen_cmd() {
        cmd = new argparse::ArgumentParser("gen");
        cmd->add_description("generate mbr");
        cmd->add_argument("config").help("mbr config file to process.").required();
        cmd->add_argument("file").help("mbr file to generate.").required();
    }

    void run() {
        auto image_path = cmd->get<std::string>("file");
        auto config_path = cmd->get<std::string>("config");

        std::ifstream config_file(config_path);
        nlohmann::json json = nlohmann::json::parse(config_file);

        sunxi_mbr mbr(image_path);
        for (auto &el : json) {
            std::cout << "adding " << el.dump() << std::endl;
            mbr.add_file(el["name"],
                         el["classname"],
                         el["user_type"]);
        }
        mbr.write();
    }
};

mbr_cmd::mbr_cmd() {
    cmd = new argparse::ArgumentParser("mbr");
    cmd->add_description("handle mbr");

    cmds.emplace_back(new mbr_dump_cmd());
    cmds.emplace_back(new mbr_gen_cmd());

    for (auto &sub : cmds) {
        cmd->add_subparser(*sub->cmd);
    }
}

void mbr_cmd::run() {
    for (auto &sub : cmds) {
        if (cmd->is_subcommand_used(*sub->cmd)) {
            return sub->run();
        }
    }
    throw std::runtime_error("missing sub command");
}