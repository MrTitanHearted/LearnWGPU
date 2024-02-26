#include <iostream>

#include <WEngine.hpp>

int main(int argc, char** argv) {
    WEngine::Init();

    WEngine& engine = WEngine::GetInstance();

    try {
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    WEngine::Shutdown();
    return 0;
}
