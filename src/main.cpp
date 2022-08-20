#include "main.hpp"
#include "List.h"

#define SLEEP_NS(ns) std::this_thread::sleep_for (std::chrono::nanoseconds(ns))

typedef std::shared_ptr<std::string> str_ptr;

//Key and IV setup
CryptoPP::byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
//
// String and Sink setup
//
str_ptr ciphertext = std::make_shared<std::string>(std::string());
str_ptr decryptedtext = std::make_shared<std::string>(std::string());

//
// Create Cipher Text
//
CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );
CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( *ciphertext ) );

CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );
CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( *decryptedtext ) );


struct Block {
    std::shared_ptr<std::string> data;
    bool encoded = false;

    Block(std::shared_ptr<std::string>&& data_) : data(std::move(data_)) {}
    Block(std::string&& data_) : data(std::make_shared<std::string>(std::move(data_))) {}
};


List<Block> blockChain = List<Block>();
std::mutex blockChainLock;
std::mutex printMutex;

// #define print(x)  printMutex.lock(); std::cerr << x; printMutex.unlock(); 

#define BLOCK_SIZE 1024*8       // max length of one block (in bytes)
#define BLOCK_BUFFER 64      // max length of block chain (in blocks)

bool READING_FINISHED = false;
bool PROCESSING_FINISHED = false;
bool WRITING_FINISHED = false;

void encryptBlock(Block& c){

}

void writeBlock(Block c){
    std::cout.write(c.data->c_str(), c.data->length());
}

Block readBlock(int n){
    char* buf = new char[n];
    int rc = read(0, buf, n);
    if (rc == 0){ throw std::out_of_range("STDIN finished"); }
    return Block( std::string(std::move(buf), rc) );
}


void readerThread(){
    // read new Blocks while stdin is full
    while (true){
        try{
            Block c = readBlock(BLOCK_SIZE);
            // wait untils block chain has room for anotehr block
            while (blockChain.size() >= BLOCK_BUFFER){
                // SLEEP_NS(2);
            }
            blockChainLock.lock();
            blockChain.put(std::move(c));
            blockChainLock.unlock();

        } catch (std::out_of_range e){
            break; // stdin has finished
        }
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


void encoderThread(){
    Node<Block>* bp;

    while (!READING_FINISHED || blockChain.size()){
        
        if (blockChain.size()) {
            
            // find next un-encoded block in chain
            blockChainLock.lock();
            bp = blockChain.front();
            while (bp != nullptr && bp->data.encoded){ bp = bp->next; }
            blockChainLock.unlock();
            
            // if we found an un.encoded block
            if (bp != nullptr){
                bp->data.encoded = true;
            } else {
                SLEEP_NS(100); // block chain has no un-encoded blocks
            }
            
        } else {
            SLEEP_NS(5); // block chain is currently empty
        }
        
    }
    PROCESSING_FINISHED = true;
    std::cerr << "Finished processing\n";
}



int main() {
    int a = ( BLOCK_SIZE * BLOCK_BUFFER) /1024;
    std::cerr << "PipeCrypt buffer size is " << BLOCK_SIZE/1024 << " KiB x " << BLOCK_BUFFER << " = " << a <<" KiB\n";
    
    freopen(NULL, "rb", stdin);      // re-open stdin in binary mode

    std::thread tr(readerThread);
    std::thread te(encoderThread);
    std::thread tw(writerThread);

    // //Key and IV setup
    // memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
    // memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

    
    tr.join();
    te.join();
    tw.join();


    return 0;
}

// dd if=/dev/zero of=test.bin bs=1024 count=0 seek=$[500]
// pv test.bin | build/PipeCrypt > test2.bin
// echo "$(cmp --silent test.bin test2.bin; echo $?)"


int main3() {
    
    
    //
    // Dump Plain Text
    //
    // std::cout << "Plain Text (" << plaintext->size() << " bytes)" << std::endl;
    // std::cout << plaintext;
    std::cout << std::endl << std::endl;

    
    

    //
    // Dump Cipher Text
    //
    std::cout << "Cipher Text (" << ciphertext->size() << " bytes)" << std::endl;

    for( int i = 0; i < ciphertext->size(); i++ ) {

        std::cout << "0x" << std::hex << (0xFF & static_cast<CryptoPP::byte>((*ciphertext)[i])) << " ";
    }

    std::cout << std::endl << std::endl;

    //
    // Decrypt
    //
    
    stfDecryptor.Put( reinterpret_cast<const unsigned char*>( ciphertext->c_str() ), ciphertext->size() );
    stfDecryptor.MessageEnd();

    //
    // Dump Decrypted Text
    //
    std::cout << "Decrypted Text: " << std::endl;
    std::cout << decryptedtext;
    std::cout << std::endl << std::endl;

    return 0;
}