# pipe-crypt

A CLI pipe tool for encrypting and decrypting data

Following the UNIX philosophy "do one thing, do one thing only, but do it well",
this tool only does encryption and decryption (signing and integrity verification maybe in future versions).

See [credits](#credits-and-thanks) if you want to know more.

## Features

- Supports UNIX pipe operation
- High throughput, see [Benchmarks](#benchmarks)
- Encryption/Decryption supports [AES-256-CTR](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#CTR)
- Password hashing supports [SHA3-256](https://en.wikipedia.org/wiki/SHA-3)
- Custom nonce values may be specified
- Byte-wise encrypt/decrypt to support instantaneous output with `-b 1`

## Usage Examples

### Encrypting/Decrypting files

- Encrypting a file

  ```bash
  cat SOME_FILE | pipe-crypt ... - > SOME_FILE.crypt
  ```

- Decrypting a file

  ```bash
  cat SOME_FILE.crypt | pipe-crypt ... > SOME_FILE
  ```

### Combined with `tar`

- Creating an encrypted archive

  ```bash
  tar -c SOME_FOLDER | pipe-crypt ... > archive.tar.crypt
  ```

- Extracting an encrypted archive

  ```bash
  cat archive.tar.crypt | pipe-crypt ... | tar -x
  ```

## Benchmarks

_Reference system is an AMD Ryzen 9 3900X_

## Tips for maximum performance and security

> Once you have encrypted something here is no way of telling what it
> was before, which password was used to encrypt it, what algorithm
> was used for password hashing, with AES mode was used for encryption
> and how many times it has been encrypted.
>
>**If you forget the encryption parameters, you might lose your data!**

### Choosing password and nonce

At the moment, AES is virtually unbreakable, but only if the nonce is unique with regard to the message. This means:

- **If possible, choose a different nonce for every message**

In ideal world, you would use a different password for everything. But since you can't remember everything, you probably won't. But if using the same password for multiple messages, make sure the nonce unique per message!

### Choosing block size (`-b`, `--block-size`) option

In general, the more data per block, the more throughput, but since the program waits until this data is available via stdin, the program will be idle for as long as it has received less than that.

- **The smaller the block size, the more responsive the output. The larger the block size, the more throughput at the cost if increased latency**

- **If your input yields data in small bits and slowly (logging etc.), use smaller sizes like `1-256` for more responsive processing.**

- **If your input yields large amounts of data and quite fast (reading files etc.), use larger sizes like `4096-65536` for more throughput.**

### Choosing chain length (`-c`, `--chain-length`) option

Most probably, you don't need to tune this option, but if your input yields some data chunk-wise you may want to have more buffer to read data as soon as it is available and keep it, so that the encryption process doesn't have to wait for the input.

- **If your input yields chunk-wise, so that the encryption cannot run continuously, increase the chain length**

- **In such cases, you might want to increase the block size as well**

## Testing

- Create a test file with 400 MiB of random binary data:

  ```bash
  dd if=/dev/random bs=$[1024*1024] count=400 of=test.bin
  ```

- Encrypt the data:

  ```bash
  cat test.bin | ./pipe-crypt -b 40000 -c 256 "password" > encrypted.bin
  ```

- Decrypt the data using the same password:

  ```bash
  cat encrypted.bin | ./pipe-crypt -b 40000 -c 256 "password" > decrypted.bin
  ```

- Verify, that `test.bin` and `decrypted.bin` have the exact same content:

  ```bash
  echo "$(cmp --silent test.bin decrypted.bin; echo $?)"
  ```

## Backlog (Planned Features)

- More AES modes like [AES-GCM](https://en.wikipedia.org/wiki/Galois/Counter_Mode) and EBC, CBC, etc.
- More hash functions for password and nonce
- Option to choose hash functions for password/nonce separately
- specify a password file with `-f` rather than exposing the password to CLI

## Credits and thanks

- Argument parsing is done with [argparse](https://github.com/p-ranav/argparse) by [p-ranav](https://github.com/p-ranav), a very great and intuitive library supporting CMake across platforms

- pipe-crypt uses [Crypto++](https://github.com/weidai11/cryptopp) for all cryptographic functionalities, it is by far the most used C++ library for cryptography and can virtually do everything, if you dig deep enough in the documentation.

## Disclaimer

I always try my best to make this tool as fast, secure and convenient as possible.
But since I am not a cybersecurity expert, I can not guarantee for the cryptographic strength
of the tool or any messages encoded/decoded with it.

If you find bugs or security issues, please feel free to fork this repo and create a pull
request, or at least open up a Git Issue and tell me what's wrong, so that I can fix it!

Thanks for choosing open-source software!
