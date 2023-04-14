#include "image.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>

#include "json.hpp"

#include "util.hpp"

sunxi_image::sunxi_image(const std::string _path)
    : path(_path) {

    memset(&header, 0, sizeof(imagewty_header_t));

    memcpy(header.magic, IMAGEWTY_MAGIC, IMAGEWTY_MAGIC_LEN);
    header.header_version = 0x0300;
    header.header_size = 96;
    header.ram_base = 0x4d00000;
    header.version = IMAGEWTY_VERSION;
    header.image_size = 0x0; // overwritten later
    header.image_header_size = 0x0;

    header.v3.unknown = 1024;
    header.v3.pid = 0x1234;
    header.v3.vid = 0x8743;
    header.v3.hardware_id = 0x100;
    header.v3.firmware_id = 0x100;
    header.v3.val1 = 0x1;
    header.v3.val1024 = 1024;
    header.v3.num_files = 0; // overwritten later
    header.v3.val1024_2 = 1024;
    header.v3.val0 = 0;
    header.v3.val0_2 = 0;
    header.v3.val0_3 = 0;
    header.v3.val0_4 = 0;
}

void sunxi_image::read() {
    std::fstream file(path, std::ios_base::binary | std::ios_base::in);

    file.read((char *)&header, sizeof(imagewty_header_t));
    if (memcmp(header.magic, IMAGEWTY_MAGIC, IMAGEWTY_MAGIC_LEN) != 0) {
        throw std::runtime_error("invalid magic");
    }

    const uint32_t file_num = is_v3() ? header.v3.num_files : header.v1.num_files;
    for (size_t i = 0; i < file_num; i++) {
        imagewty_file_header_t file_header;
        file.seekg((i + 1) * IMAGEWTY_FILEHDR_LEN, std::ios_base::beg);
        file.read((char *)&file_header, sizeof(imagewty_file_header_t));
        file_headers.push_back(file_header);
    }
}

void sunxi_image::dump_files() {
    std::fstream src_file(path, std::ios_base::binary | std::ios_base::in);

    for (auto &file_header : file_headers) {
        auto offset = is_v3() ? file_header.v3.offset : file_header.v3.offset;
        auto size = is_v3() ? file_header.v3.original_length : file_header.v3.original_length;

        auto filename_ptr = is_v3() ? file_header.v3.filename : file_header.v3.filename;
        auto filename = std::string(filename_ptr, filename_ptr + file_header.filename_len);

        std::vector<char> buf(size);
        src_file.seekg(offset, std::ios_base::beg);
        src_file.read(buf.data(), size);

        std::fstream fs(filename, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
        fs.write(buf.data(), size);
    }
}

void sunxi_image::add_file(std::string name, std::string maintype, std::string subtype) {
    imagewty_file_header_t file_header;
    memset(&file_header, 0, sizeof(imagewty_file_header_t));
    file_header.filename_len = name.size();
    file_header.total_header_size = 1024;

    memset(file_header.maintype, 0x20, IMAGEWTY_FHDR_MAINTYPE_LEN);
    memcpy(file_header.maintype, maintype.c_str(), maintype.size());

    memset(file_header.subtype, 0x20, IMAGEWTY_FHDR_SUBTYPE_LEN);
    memcpy(file_header.subtype, subtype.c_str(), subtype.size());

    file_header.v3.unknown_0 = 0;
    file_header.v3.pad1 = 0;
    file_header.v3.pad2 = 0;

    memcpy(file_header.v3.filename, name.c_str(), name.size());

    auto file_size = std::filesystem::file_size(name);
    file_header.v3.original_length = file_size;
    file_header.v3.stored_length = MEM_ALIGN(file_size, 512);
    file_header.v3.offset = 0; // overwritten later

    file_headers.push_back(file_header);
}

void sunxi_image::write() {
    finish_header();

    std::fstream file(path, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    file.write((char *)&header, sizeof(imagewty_header_t));

    for (size_t i = 0; i < file_headers.size(); i++) {
        imagewty_file_header_t *file_header = &file_headers[i];

        file.seekp((i + 1) * IMAGEWTY_FILEHDR_LEN, std::ios_base::beg);
        file.write((char *)file_header, sizeof(imagewty_file_header_t));
    }

    for (size_t i = 0; i < file_headers.size(); i++) {
        imagewty_file_header_t *file_header = &file_headers[i];

        const auto file_offset = is_v3() ? file_header->v3.offset : file_header->v1.offset;

        const auto filename_ptr = is_v3() ? file_header->v3.filename : file_header->v1.filename;
        const auto filename = std::string(filename_ptr, filename_ptr + file_header->filename_len);

        const size_t file_end = i < file_headers.size() - 1 ? (is_v3() ? file_headers[i + 1].v3.offset : file_headers[i + 1].v1.offset) : header.image_size;

        const size_t full_length = file_end - file_offset;
        std::vector<char> buf(full_length);

        const auto stored_length = is_v3() ? file_header->v3.stored_length : file_header->v1.stored_length;
        memset(buf.data() + stored_length, 0xCD, full_length - stored_length);

        const auto original_length = is_v3() ? file_header->v3.original_length : file_header->v1.original_length;
        std::fstream src_file(filename, std::ios_base::binary | std::ios_base::in);
        src_file.read(buf.data(), original_length);

        file.seekp(file_offset, std::ios_base::beg);
        file.write(buf.data(), full_length);
    }
}

bool sunxi_image::is_v3() {
    return header.header_version == 0x0300;
}

void sunxi_image::finish_header() {
    if (is_v3()) {
        header.v3.num_files = file_headers.size();
    } else {
        header.v1.num_files = file_headers.size();
    }

    size_t offset = (file_headers.size() + 1) * IMAGEWTY_FILEHDR_LEN;
    for (auto &file_header : file_headers) {
        if (is_v3()) {
            file_header.v3.offset = offset;
        } else {
            file_header.v1.offset = offset;
        }
        const uint32_t size = is_v3() ? file_header.v3.stored_length : file_header.v1.stored_length;
        offset = offset + MEM_ALIGN(size, 1024);
    }
    header.image_size = offset;
}

struct image_dump_cmd : public subcmd {
    image_dump_cmd() {
        cmd = new argparse::ArgumentParser("dump");
        cmd->add_description("dump image");
        cmd->add_argument("file").help("image file to process.").required();
    }

    void run() {
        auto filename = cmd->get<std::string>("file");

        sunxi_image image(filename);
        image.read();
        image.dump_files();

        nlohmann::json json;
        for (auto &f : image.file_headers) {

            nlohmann::json entry;
            const auto filename_ptr = image.is_v3() ? f.v3.filename : f.v1.filename;
            entry["filename"] = extract_string((const char *)filename_ptr, f.filename_len);
            entry["maintype"] = extract_string((const char *)f.maintype, IMAGEWTY_FHDR_MAINTYPE_LEN);
            entry["subtype"] = extract_string((const char *)f.subtype, IMAGEWTY_FHDR_SUBTYPE_LEN);
            json.push_back(entry);
        }

        std::cout << json.dump() << std::endl;
    }
};

struct image_gen_cmd : public subcmd {
    image_gen_cmd() {
        cmd = new argparse::ArgumentParser("gen");
        cmd->add_description("generate image");
        cmd->add_argument("config").help("image config file to process.").required();
        cmd->add_argument("file").help("image file to generate.").required();
    }

    void run() {
        auto image_path = cmd->get<std::string>("file");
        auto config_path = cmd->get<std::string>("config");

        std::ifstream config_file(config_path);
        nlohmann::json json = nlohmann::json::parse(config_file);

        sunxi_image image(image_path);
        for (auto &el : json) {
            std::cout << "adding " << el.dump() << std::endl;
            image.add_file(el["filename"],
                           el["maintype"],
                           el["subtype"]);
        }
        image.write();
    }
};

image_cmd::image_cmd() {
    cmd = new argparse::ArgumentParser("image");
    cmd->add_description("handle image");

    cmds.emplace_back(new image_dump_cmd());
    cmds.emplace_back(new image_gen_cmd());

    for (auto &sub : cmds) {
        cmd->add_subparser(*sub->cmd);
    }
}

void image_cmd::run() {
    for (auto &sub : cmds) {
        if (cmd->is_subcommand_used(*sub->cmd)) {
            return sub->run();
        }
    }
    throw std::runtime_error("missing sub command");
}