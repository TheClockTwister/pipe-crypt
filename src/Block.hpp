#include "main.hpp"


struct Block {
    std::string data;
    bool encoded = false;
    bool isLast = false;

    Block(std::string&& data_) : data(std::move(data_)) {}
    Block(std::string&& data_, bool last) : data(std::move(data_)), isLast(last) {}

    static void writeBlock(Block c){
        std::cout.write(c.data.c_str(), c.data.length());
    }

    static Block readBlock(int n){
        char* buf = new char[n];
        int rc = read(0, buf, n);
        std::string x = std::string(std::move(buf), rc);
        delete[] buf;
        return (rc == 0) ? Block( std::string(), true) : Block( std::move(x));
    }
};
