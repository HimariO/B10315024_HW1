import numpy as np
import math

class CipherBase:
    def __init__(self, key):
        self.key = key

    @staticmethod
    def _pre_process(text):
        return text.strip().replace(" ", "").lower()

    @staticmethod
    def _concat_chars(char_list):
        s = ''
        for w in char_list:
            s += w
        return s

    def encrypt(self, plaintext):
        raise NotImplemented('This is just a base class!')

    def decrypt(self, cyphertext):
        raise NotImplemented('This is just a base class!')

    def __str__(self):
        return self.__class__.__name__ + ' with key: ' + str(self.key)

class CaesarCipher(CipherBase):
    def encrypt(self, plaintext):
        assert type(self.key) is int

        plaintext = self._pre_process(plaintext)
        # table = [chr(i) for i in range(ord('a'), ord('z') + 1)]
        result = [(ord(alph) + self.key) % ord('a') + ord('a') for alph in plaintext]
        result = [chr(n) for n in result]

        return self._concat_chars(result)

    def decrypt(self, cyphertext):
        assert type(self.key) is int

        cyphertext = self._pre_process(cyphertext)
        result = [(ord(alph) - self.key) % ord('a') + ord('a') for alph in cyphertext]
        result = [chr(n) for n in result]

        return self._concat_chars(result)


class MonoalphCipher(CipherBase):
    def __init__(self, key):
        self.key = key
        self.rever = {}
        for k in self.key.keys():
            self.rever[self.key[k].lower()] = k

    def encrypt(self, plaintext):
        assert type(self.key) is dict

        plaintext = self._pre_process(plaintext)
        result = list(map(lambda c: self.key[c], plaintext))
        return self._concat_chars(result)

    def decrypt(self, cyphertext):
        assert type(self.key) is dict

        cyphertext = self._pre_process(cyphertext)
        result = list(map(lambda c: self.rever[c], cyphertext))
        return self._concat_chars(result)


class PlayfairCipher(CipherBase):
    def __init__(self, key):
        assert type(key) is str
        pad = 'abcdefghiklmnopqrstuvwxyz'
        key = self._pre_process(key)
        self.key = []

        for c in key:
            if c not in self.key:
                self.key.append(c)

        while len(self.key) < 25:
            if pad[0] not in self.key:
                self.key.append(pad[0])
            pad = pad[1:]
        self.key = np.array(self.key).reshape([5, 5])

    def sep_dup(self, plaintext):
        last = ''
        out = ''
        for i in plaintext:
            if i == last:
                out += 'x'
            out += i
            last = i
        return out

    def encrypt(self, plaintext):
        plaintext = self._pre_process(plaintext)
        plaintext = self.sep_dup(plaintext)
        result = ''

        for i in range(0, len(plaintext), 2):
            fir = plaintext[i]
            if i + 1 <  len(plaintext):
                sec = plaintext[i +1]
            else:
                sec = 'x'

            fir_pos = np.where(self.key == fir)
            sec_pos = np.where(self.key == sec)

            if all(fir_pos[0] == sec_pos[0]):  # same row
                result += self.key[fir_pos[0][0], (fir_pos[1][0] + 1) % 5]
                result += self.key[sec_pos[0][0], (sec_pos[1][0] + 1) % 5]
            elif all(fir_pos[1] == sec_pos[1]):  # same colume
                result += self.key[(fir_pos[0][0] + 1) % 5, fir_pos[1][0]]
                result += self.key[(sec_pos[0][0] + 1) % 5, sec_pos[1][0]]
            else:
                result += self.key[fir_pos[0][0], sec_pos[1][0]]
                result += self.key[sec_pos[0][0], fir_pos[1][0]]

        return result

    def decrypt(self, cyphertext):
        cyphertext = self._pre_process(cyphertext)
        result = ''
        assert len(cyphertext) % 2 == 0

        for i in range(0, len(cyphertext), 2):
            fir = cyphertext[i]
            sec = cyphertext[i +1]

            fir_pos = np.where(self.key == fir)
            sec_pos = np.where(self.key == sec)

            if all(fir_pos[0] == sec_pos[0]):  # same row
                result += self.key[fir_pos[0][0], (fir_pos[1][0] - 1)]
                result += self.key[sec_pos[0][0], (sec_pos[1][0] - 1)]
            elif all(fir_pos[1] == sec_pos[1]):  # same colume
                result += self.key[(fir_pos[0][0] - 1), fir_pos[1][0]]
                result += self.key[(sec_pos[0][0] - 1), sec_pos[1][0]]
            else:
                result += self.key[fir_pos[0][0], sec_pos[1][0]]
                result += self.key[sec_pos[0][0], fir_pos[1][0]]

        return result

class VernamAutoCipher(CipherBase):
    def extend_key(self, plaintext):
        self.key = self._pre_process(self.key)
        return self.key + plaintext[:-len(self.key)]

    def encrypt(self, plaintext):
        assert type(self.key) is str
        plaintext = self._pre_process(plaintext)
        result = []

        for w, k in zip(plaintext, self.extend_key(plaintext)):
            result.append(((ord(w) - ord('a')) ^ (ord(k) - ord('a'))))

        table = [chr(i) for i in range(ord('a'), ord('z') + 1)] + [str(i) for i in range(6)]
        result = list(map(lambda x: table[x], result))
        return self._concat_chars(result)

    def decrypt(self, cyphertext):
        assert type(self.key) is str

        cyphertext = self._pre_process(cyphertext)
        sec_size = len(self.key)
        lastout = None

        result = ''
        table = [chr(i) for i in range(ord('a'), ord('z') + 1)] + [str(i) for i in range(6)]
        sub = [chr(i) for i in range(123, 129)]

        temp_ = ''
        for i in range(len(cyphertext)):
            if cyphertext[i] in '012345':
                temp_ += sub[int(cyphertext[i])]
            else:
                temp_ += cyphertext[i]
        cyphertext = temp_

        for sec in range(0, len(cyphertext), sec_size):
            sec_text = cyphertext[sec: sec + sec_size]
            sec_key = lastout if sec != 0 else self.key
            # print(sec_text)
            sec_out = [table[(ord(w) - ord('a')) ^ (ord(k) - ord('a'))] for w, k in zip(sec_text, sec_key)]
            lastout = sec_out = self._concat_chars(sec_out)
            result += sec_out

        return result


class RowTransCipher(CipherBase):
    def rever_key(self):
        r_key = ['0'] * len(self.key)
        for i in range(len(self.key)):
            r_key[int(self.key[i]) - 1] = str(i)
        return self._concat_chars(r_key)

    def text_matrix(self, plaintext, reverse=False):
        array = []
        step = math.floor(len(plaintext) / len(self.key)) if reverse else len(self.key)

        for n in range(0, len(plaintext), step):
            chars = [plaintext[m] for m in range(n, n + step) if m < len(plaintext)]
            if len(chars) < step:
                chars += ['#' for i in range(step - len(chars))]
            array.append(chars)
        array = np.array(array)
        return  array

    def encrypt(self, plaintext):
        assert type(self.key) is str or int

        plaintext = self._pre_process(plaintext)
        text_mat = self.text_matrix(plaintext)
        text_mat = text_mat.T
        result_mat = text_mat.copy()

        for row_pln, row_n in zip(text_mat, self.key):
            result_mat[int(row_n) - 1] = row_pln

        result = ''
        # result_mat = result_mat.T
        for row in result_mat:
            result += self._concat_chars(row.tolist())
        return result

    def decrypt(self, cyphertext):
        assert type(self.key) is str or int

        cyphertext = self._pre_process(cyphertext)
        text_mat = self.text_matrix(cyphertext, reverse=True)
        text_mat = text_mat
        result_mat = text_mat.copy()

        for row_cry, row_n in zip(text_mat, self.rever_key()):
            result_mat[int(row_n)] = row_cry

        result = ''
        result_mat = result_mat.T
        for row in result_mat:
            result += self._concat_chars(row.tolist())
        return result

class ProductCipher(CipherBase):
    def __init__(self, key):
        assert type(key) is list
        self.key = key

    def rever_key(self):
        r_key = [0] * len(self.key)
        for i in range(len(self.key)):
            r_key[self.key[i] - 1] = i
        return r_key

    def _encrypt(self, plaintext):
        plaintext += '#' * (len(self.key) - len(plaintext) + 1)
        plaintext = self._pre_process(plaintext)
        result = [''] * len(plaintext)

        for w, k in zip(plaintext, self.key):
            result[k - 1] = w

        return self._concat_chars(result)

    def encrypt(self, plaintext):
        result = ''
        for i in range(0, len(plaintext), len(self.key)):
            end = i + len(self.key) if i + len(self.key) < len(plaintext) else len(plaintext)
            result += self._encrypt(plaintext[i: end])
        return result

    def decrypt(self, cyphertext):
        cyphertext = self._pre_process(cyphertext)
        sec_size = len(self.key)

        result = [''] * len(cyphertext)
        for w, k in zip(cyphertext, self.rever_key()):
            result[k] = w
        return self._concat_chars(result)
