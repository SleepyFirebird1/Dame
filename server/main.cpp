#include "Server.hpp"

int main() {
    // Tworzymy serwer nasłuchujący na porcie 8080
    Server gameServer(8080);
    
    // Uruchamiamy główną pętlę serwera (ta metoda zablokuje się na accept())
    gameServer.start();
    
    return 0;
}