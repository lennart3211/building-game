#include "Editor.h"

#include <cstdlib>
#include <iostream>

int main() {
    engine::Editor app;
    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}