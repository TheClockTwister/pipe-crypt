#include "headers.hpp"

using namespace CryptoPP;


CryptoPP::SHA3_256 keyHash;
CryptoPP::RIPEMD128 ivHash;
byte key [keyHash.DIGESTSIZE];
byte iv  [ivHash.DIGESTSIZE];

CTR_Mode< AES >::Encryption e;
CTR_Mode< AES >::Decryption d;

std::string inputBuffer, outputBuffer;


int encrypt(){
    try {
        StringSource ss1(inputBuffer, true,
            new StreamTransformationFilter(e, new StringSink(outputBuffer))
        );
    }
    
    catch( CryptoPP::Exception& e ) { return 1; }
    return 0;
}

int decrypt(){
    try {
        StringSource ss3(inputBuffer, true,
        // The StreamTransformationFilter removes padding as required.
            new StreamTransformationFilter(d, new StringSink(outputBuffer))
        );
    }
    
    catch( CryptoPP::Exception& e ) { return 2; }
    return 0;
}


int perform(int function() ){
    int rc;
    while (std::cin >> inputBuffer){
        outputBuffer.clear();
        rc = function(); // read inputBuffer and write outputBuffer
        if (rc != 0){ return rc; }
        std::cout << outputBuffer;
        inputBuffer.clear();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2){ return 5; } // not enough arguments
    
    // pre-allocate 4MB for input and output buffer each
    inputBuffer.reserve(1024*1024*4);
    outputBuffer.reserve(1024*1024*4);

    // TODO: Read password from CLI args
    std::string const password = "MySecretPassword!";

    // get encryption key from password hash
    keyHash.CalculateDigest( key, (CryptoPP::byte*) password.c_str(), password.length() );
    // get initialization vector from (password hash) hash
    ivHash.CalculateDigest( iv, key, password.length() );

    d.SetKeyWithIV( key, sizeof(key), iv, sizeof(iv) );
    e.SetKeyWithIV( key, sizeof(key), iv, sizeof(iv) );

    if (strcmp(argv[1], "-e") == 0){ return perform(encrypt); }
    if (strcmp(argv[1], "-d") == 0){ return perform(decrypt); }

    return 6;
}
