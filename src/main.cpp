#include <iostream>

#include <WEngine.hpp>

int main(int argc, char **arv) {
    WEngine::Initialize();

    WEngine &engine = WEngine::GetInstance();

    try {
        engine.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    WEngine::Shutdown();
    return 0;
}
