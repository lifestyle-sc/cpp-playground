#include <iostream>
#include <vector>

#include <unistd.h>

int main(int argc, char *argv[]) {
    std::cout << "Welcome to my setup" << std::endl;

    int bytes = 0;
    constexpr int BUFFER_SIZE = 4096;
    std::vector<char> buffer(BUFFER_SIZE);
    while ((bytes = read(STDIN_FILENO, buffer.data(), BUFFER_SIZE)) > 0) {
        if (write(STDOUT_FILENO, buffer.data(), bytes) != bytes) {
            std::cerr << "Write error!" << std::endl;
        }
    }

    if (bytes < 0) {
        std::cerr << "Read error!" << std::endl;
    }
}
