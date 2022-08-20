from Crypto.Cipher import AES

aes = AES.new(b'\x00'*16, AES.MODE_CTR)

f2 = open("test2.bin", "rb")
f3 = open("test3.bin", "wb")

f3.write(aes.decrypt(f2.read()))
