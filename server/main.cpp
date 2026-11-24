#include "Server.hpp"

int main()
{
    // Tworzenie serwera nasłuchującego na porcie 8080
    Server gameServer(8080);
    gameServer.start();

    return 0;
}