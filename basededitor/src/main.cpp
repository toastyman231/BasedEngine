#include <iostream>
#include "based/engine.h"

int main()
{
    based::Engine& engine = based::Engine::Instance();
    engine.Run();

    std::cout << "Press ENTER to continue...";
    std::cin.ignore();
    return 0;
}