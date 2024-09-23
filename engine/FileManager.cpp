//
// Created by mclen on 21/08/2024.
//

#include "FileManager.h"
#include <iostream>

namespace engine {
FileManager::FileManager() {
    NFD_Init();
}

FileManager::~FileManager() {
    NFD_Quit();
}
std::string FileManager::open(std::vector<nfdu8filteritem_t> &filters) {
    nfdu8char_t *outPath;
    nfdopendialogu8args_t args = {0};
    args.filterList = filters.data();
    args.filterCount = (nfdfiltersize_t) filters.size();
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
      std::string str{outPath};
//      NFD_FreePathU8(outPath);
      return str;
    }
    else if (result == NFD_CANCEL)
    {
      return {};
    }
    else
    {
      std::cerr << "Error: " << NFD_GetError() << '\n';
      return {};
    }
}

} // namespace engine