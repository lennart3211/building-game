#pragma once

#include <native_file_dialog/include/nfd.h>

#include <string>
#include <vector>

namespace engine {

class FileManager {
private:

public:
    FileManager();
    ~FileManager();

    std::string open(std::vector<nfdu8filteritem_t> &filters);
};

} // namespace engine
