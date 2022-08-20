#include "main.hpp"
#include "List.h"

#include "argparser.hpp"

struct Block {
    std::string data;
    bool encoded = false;
    bool isLast = false;

    Block(std::string&& data_) : data(std::move(data_)) {}
    Block(std::string&& data_, bool last) : data(std::move(data_)), isLast(last) {}
};


List<Block> blockChain = List<Block>();
std::mutex blockChainLock;
std::mutex printMutex;

// #define print(x)  printMutex.lock(); std::cerr << x; printMutex.unlock();
#define SLEEP_NS(ns) std::this_thread::sleep_for (std::chrono::nanoseconds(ns))

int BLOCK_SIZE = 1024*4;    // max length of one block (in bytes)
int CHAIN_LENGTH = 64;      // max length of the block chain (in blocks)

bool READING_FINISHED = false;
bool PROCESSING_FINISHED = false;
bool WRITING_FINISHED = false;

CryptoPP::byte key[32], iv[32]; // 256-bit hash as key and nonce


void writeBlock(Block c){
    std::cout.write(c.data.c_str(), c.data.length());
}

Block readBlock(int n){
    char* buf = new char[n];
    int rc = read(0, buf, n);
    return (rc == 0) ? Block( std::string(), true) : Block( std::string(std::move(buf), rc));
}


void readerThread(){
    // read new Blocks while stdin is full
    bool lastBlock = false;
    while (!lastBlock){
        Block c = readBlock(BLOCK_SIZE);
        lastBlock = c.isLast;
        blockChainLock.lock();
        blockChain.put(std::move(c));
        blockChainLock.unlock();
    }
    READING_FINISHED = true;
}

void writerThread(){
    Node<Block>* bp;
    
    while (!PROCESSING_FINISHED || blockChain.size()){
        blockChainLock.lock();
        try{
            bp = blockChain.front();
            if (bp == nullptr){ throw std::range_error(""); }
            if (!(bp->data.encoded)){ throw std::range_error(""); }
            writeBlock(blockChain.pop());
            blockChainLock.unlock();
            
        } catch (std::range_error e) {
            blockChainLock.unlock();
            SLEEP_NS(5); // block chain is currently empty
        }
    }
}

void encoderThread(){
    Node<Block>* bp;
    bool last = false;

    std::string processedData;
    CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CTR_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv);
    CryptoPP::StreamTransformationFilter filter(cbcEncryption, new CryptoPP::StringSink( processedData ) );

    while (!READING_FINISHED || blockChain.size()){
        
        if (blockChain.size()) {
            // find next un-encoded block in chain
            blockChainLock.lock();
            bp = blockChain.front();
            if (bp != nullptr){
                if (bp->data.encoded){
                    bp = nullptr;
                }
            }
            blockChainLock.unlock();
            // if we found an un.encoded block
            if (bp != nullptr){
                /// processing takes place here
                filter.Put( reinterpret_cast<const unsigned char*>( bp->data.data.c_str() ), bp->data.data.length() );
                if (bp->data.isLast){
                    filter.MessageEnd();
                    last = true;
                }
                bp->data.data = processedData;
                processedData.clear();
                bp->data.encoded = true;
            } else {
                SLEEP_NS(100); // block chain has no un-encoded blocks
            }
        } else {
            SLEEP_NS(5); // block chain is currently empty
        }
    }
    PROCESSING_FINISHED = true;
}


int main(int argc, char* argv[]) {
    

    ap::parser p(argc, argv);
    p.add("-p", "--password",   "The key for encryption and decryption",            ap::mode::REQUIRED);
    p.add("-n", "--nonce",      "(optional) defaults to the password",              ap::mode::OPTIONAL);

    p.add("-b", "--block-size", "Buffer block size in bytes (default: 1024)",       ap::mode::OPTIONAL);
    p.add("-c", "--chain-length", "Buffer chain length in blocks (default: 64)",    ap::mode::OPTIONAL);

    // p.add("-v", "--verbose",    "More verbose output = more details",               ap::mode::BOOLEAN);
    // p.add("-q", "--quiet",      "No output at all",                                 ap::mode::BOOLEAN);
    
    auto args = p.parse();

    if (!args.parsed_successfully()) {
	    std::cerr << "Invalid arguments!\n";
        return -1;
    }

    BLOCK_SIZE = (args["-b"].empty() ? BLOCK_SIZE : std::stoi(args["-b"]));
    CHAIN_LENGTH = (args["-c"].empty() ? CHAIN_LENGTH : std::stoi(args["-c"]));
    std::string password = args["-p"];
    std::string nonce = (args["-n"].empty() ? args["-p"] : args["-n"]);

    // calculate hashes for key and nonce
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

    memmove(digest, password.c_str(), password.length());
    CryptoPP::SHA256().CalculateDigest(key, digest, password.length());

    memmove(digest, nonce.c_str(), nonce.length());
    CryptoPP::SHA256().CalculateDigest(iv, digest, nonce.length());

    std::cerr << "PipeCrypt buffer size is "
    << BLOCK_SIZE/1024 << " KiB x "
    << CHAIN_LENGTH << " = "
    << ( BLOCK_SIZE * CHAIN_LENGTH) /1024 <<" KiB\n";

    freopen(NULL, "rb", stdin);      // re-open stdin in binary mode

    std::thread tr(readerThread);
    std::thread te(encoderThread);
    std::thread tw(writerThread);
    
    tr.join();
    te.join();
    tw.join();

    return 0;
}

// dd if=/dev/zero of=test.bin bs=1024 count=0 seek=$[500]
// pv test.bin | build/PipeCrypt > test2.bin
// echo "$(cmp --silent test.bin test2.bin; echo $?)"
