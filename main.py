from Ciphers import *

table = {
    'a': 'Q',
    'b': 'W',
    'c': 'E',
    'D': 'R',
    'e': 'T',
    'f': 'Y',
    'g': 'U',
    'h': 'I',
    'i': 'O',
    'j': 'P',
    'k': 'A',
    'l': 'S',
    'm': 'D',
    'n': 'F',
    'o': 'G',
    'p': 'H',
    'q': 'J',
    'r': 'K',
    's': 'L',
    't': 'Z',
    'u': 'X',
    'v': 'C',
    'w': 'V',
    'x': 'B',
    'y': 'N',
    'z': 'M',
}

transport = [15, 11, 2, 10, 16, 3, 7, 14, 4, 12, 9, 6, 1, 5, 8, 13]

inp = 'cipher testing'
ciphers = [CaesarCipher, MonoalphCipher, PlayfairCipher, VernamAutoCipher, RowTransCipher, ProductCipher]
keys = [3, table, 'DPP', 'KMT', '52834671', transport]

for C, K in zip(ciphers, keys):
    cipher = C(K)
    print(cipher)

    out = cipher.encrypt(inp)
    print(out)
    res = cipher.decrypt(out)
    print(res)

    print('------------------------------------------')
