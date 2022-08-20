# PipeCrypt

A CLI tool for encrypting and decrypting pipe data

https://github.com/weidai11/cryptopp
https://raw.githubusercontent.com/noloader/cryptopp-cmake/master/CMakeLists.txt

## General Notice

PipeCrypt uses [AES-256-CTR](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#CTR)
for encryption and [SHA3-256](https://en.wikipedia.org/wiki/SHA-3) for password hash generation.
While CTR offers many benefits, it does not protect against data manipulation like
[AES-GCM](https://en.wikipedia.org/wiki/Galois/Counter_Mode) does.

## Usage

### Options

PipeCrypt needs either `-e` or `-d` to be present, in order to perform either encryption or decryption respectively.

You can either provide an encryption/decryption phrase directly in the CLI, or provide a password file using the `-f` parameter. By doing so, your password will not show up in the CLI log, but make sure you have read permission on the password file.

| Option    | Description
|-----------|-------------
| `-e`      | Encrypt
| `-d`      | Decrypt
| `-f`      | Password file

## Usage Examples

### Encrypting/Decrypting files

- Encrypting a file

  ```bash
  cat SOME_FILE | pipecrypt -e PASSWORD > SOME_FILE.crypt
  ```

- Decrypting a file

  ```bash
  cat SOME_FILE.crypt | pipecrypt -d PASSWORD > SOME_FILE
  ```

### Combined with `tar`

- Creating an encrypted archive

  ```bash
  tar -c SOME_FOLDER | pipecrypt -e PASSWORD > archive.tar.crypt
  ```

- Extracting an encrypted archive

  ```bash
  cat archive.tar.crypt | pipecrypt -d PASSWORD | tar -x
  ```
