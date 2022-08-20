#include "main.hpp"
#include "List.h"

#define SLEEP_NS(ns) std::this_thread::sleep_for (std::chrono::nanoseconds(ns))


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

#define print(x)  printMutex.lock(); std::cerr << x; printMutex.unlock(); 

#define BLOCK_SIZE 1024*8       // max length of one block (in bytes)
#define BLOCK_BUFFER 64      // max length of block chain (in blocks)

bool READING_FINISHED = false;
bool PROCESSING_FINISHED = false;
bool WRITING_FINISHED = false;


void writeBlock(Block c){
    std::cout.write(c.data.c_str(), c.data.length());
}

Block readBlock(int n){
    char* buf = new char[n];
    int rc = read(0, buf, n);
    if (rc == 0){
        print("Reading last block\n")
        return Block( std::string(), true);
    }
    return Block( std::string(std::move(buf), rc));
}


void readerThread(){
    // read new Blocks while stdin is full
    bool lastBlock = false;
    while (!lastBlock){
        Block c = readBlock(BLOCK_SIZE);
        lastBlock = c.isLast;
        // wait untils block chain has room for anotehr block
        // while (blockChain.size() >= BLOCK_BUFFER){ SLEEP_NS(2); }

        blockChainLock.lock();
        blockChain.put(std::move(c));
        blockChainLock.unlock();
    }
    READING_FINISHED = true;
    std::cerr << "finish reading\n";
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


// -----------------------------------------------------------------------------
bool decryptMode;

void encoderThread(){
    Node<Block>* bp;
    std::string processedData;
    bool last = false;

    CryptoPP::byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
    memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
    memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );
    
    CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CTR_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv);
    CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CTR_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv);

    CryptoPP::StreamTransformationFilter encryptor(cbcEncryption, new CryptoPP::StringSink( processedData ) );
    CryptoPP::StreamTransformationFilter decryptor(cbcDecryption, new CryptoPP::StringSink( processedData ) );
    CryptoPP::StreamTransformationFilter* filter = decryptMode ? &decryptor : &encryptor;

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
            // print("E4b\n")
            // while (bp != nullptr && bp->data.encoded){ bp = bp->next; }
            blockChainLock.unlock();
            // if we found an un.encoded block
            if (bp != nullptr){
                /// processing takes place here
                filter->Put( reinterpret_cast<const unsigned char*>( bp->data.data.c_str() ), bp->data.data.length() );
       
                if (bp->data.isLast){
         
                    print("Processing Last Block\n")
           
                    filter->MessageEnd();
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
        // if (last) break;
    }
    PROCESSING_FINISHED = true;
    std::cerr << "Finished processing\n";
}

int main() {
    int a = ( BLOCK_SIZE * BLOCK_BUFFER) /1024;
    std::cerr << "PipeCrypt buffer size is " << BLOCK_SIZE/1024 << " KiB x " << BLOCK_BUFFER << " = " << a <<" KiB\n";
    // decryptMode = true;
    decryptMode = false;

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
