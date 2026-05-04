

# a = randint(0, 2^4352 - 1)

# x = randint(0, 2^256 - 1)
# y = randint(0, 2^4096 - 1)
# z = x * y + a

# print(z.bit_length())
# # assert z.bit_length() <= 4352

# print("0x%0x" % (x))
# print("0x%0x" % (y))
# print("0x%0x" % (a))
# print()
# print("0x%0x" % (z))
# print()

set_random_seed(2)

MOD_SIZE = 2048
PRIME_SIZE = MOD_SIZE // 2
WORD_SIZE = 256

p = random_prime(2^PRIME_SIZE - 1, lbound=2^(PRIME_SIZE-1))
q = random_prime(2^PRIME_SIZE - 1, lbound=2^(PRIME_SIZE-1))

n = p * q

mu = power_mod(-n, -1, 2^WORD_SIZE)
r2 = power_mod(2^MOD_SIZE, 2, n)

def mont(a, b):
    c = 0
    for i in range(MOD_SIZE // WORD_SIZE):
        ai = (a >> WORD_SIZE*i) % 2^WORD_SIZE
        c = c + ai * b
        d = (mu * c) % 2^WORD_SIZE
        c = (c + d * n) >> WORD_SIZE
    # if c >= 2^MOD_SIZE:
    #     c -= n
    if c >= n:
        print("fsaf")
        c -= n
    return c


# good = False
# i = 0
# while True:

a = randint(0, 2^(MOD_SIZE) - 1)
b = randint(0, 2^(MOD_SIZE) - 1)
c = mont(a, b)

print("0x%0x" % (a))
print("0x%0x" % (b))
print("0x%0x" % (n))
print("0x%0x" % (mu))
print("0x%0x" % (c))
print(n.bit_length())


ci = (a * b) % n
#assert c == ci

w = 10
c0 = 2^(MOD_SIZE-w)

its = MOD_SIZE + w

for i in range(its):
    c0 = (c0 + c0) % n

print()
print("0x%0x" % r2)
print("0x%0x" % c0)
assert r2 == c0
