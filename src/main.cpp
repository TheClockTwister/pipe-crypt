#include "main.hpp"
#include "List.hpp"
#include "Block.hpp"

#include <argparse/argparse.hpp>


List<Block> blockChain = List<Block>();
std::mutex blockChainLock;
std::mutex printMutex;

#define SLEEP_NS(ns) std::this_thread::sleep_for (std::chrono::nanoseconds(ns))

int BLOCK_SIZE = 1024*512;    // max length of one block (in bytes)
int CHAIN_LENGTH = 64;      // max length of the block chain (in blocks)

bool READING_FINISHED = false;
bool PROCESSING_FINISHED = false;
bool WRITING_FINISHED = false;

CryptoPP::byte key[32], iv[32]; // 256-bit hash as key and nonce


void readerThread(){
    // read new Blocks while stdin is full
    bool lastBlock = false;
    while (!lastBlock){
        Block c = Block::readBlock(BLOCK_SIZE);
        lastBlock = c.isLast;
        while (blockChain.size() >= CHAIN_LENGTH){ SLEEP_NS(5); } // chain is full
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
            Block::writeBlock(blockChain.pop());
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

    argparse::ArgumentParser program("pipe-crypt");
    program.add_argument("password").help("The key used for en-/decryption").required();

    program.add_argument("-n", "--nonce").help("(optional) defaults to the password").default_value(std::string(""));
    program.add_argument("-b", "--block-size").help("Buffer block size in bytes (default: 1024)").default_value(1024).scan<'i',int>();
    program.add_argument("-c", "--chain-length").help("Buffer chain length in blocks (default: 64)").default_value(64).scan<'i',int>();

    program.add_argument("-q", "--quiet").help("No output at all").default_value(false).implicit_value(true);
    
    try {
    program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
    }

    BLOCK_SIZE = program.get<int>("--block-size");
    CHAIN_LENGTH = program.get<int>("--chain-length");
    std::string password = program.get<std::string>("password");
    std::string nonce = program.get<std::string>("-n");
    nonce = nonce != "" ? nonce : password;

    // calculate hashes for key and nonce
    CryptoPP::byte digest[CryptoPP::SHA3_256::DIGESTSIZE];

    memmove(digest, password.c_str(), password.length());
    CryptoPP::SHA3_256().CalculateDigest(key, digest, password.length());

    memmove(digest, nonce.c_str(), nonce.length());
    CryptoPP::SHA3_256().CalculateDigest(iv, digest, nonce.length());

    if (!program.get<bool>("--quiet")){
        std::cerr << "PipeCrypt buffer size is "
        << BLOCK_SIZE/1024.f << " KiB x "
        << CHAIN_LENGTH << " = "
        << round((BLOCK_SIZE*CHAIN_LENGTH)/float(1024*1024)*100)/100 <<" MiB\n";
    }
    

    freopen(NULL, "rb", stdin);      // re-open stdin in binary mode
    std::setvbuf(stdin, nullptr, _IONBF, 0);
    std::setvbuf(stdout, nullptr, _IONBF, 0);

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
