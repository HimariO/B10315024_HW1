import numpy as np
import matplotlib.pyplot as plt
import math

def f(x):
    c = [1, 2, -2]  # reverse order
    return sum([c[i] * x ** (i) for i in range(len(c))])

def f2(t):
    return np.exp(-t)

def Bisection(a, b, f):
    if not callable(f):
        raise ValueError('f should be function')

    mid_points = [a, b]
    mid_points_f = [f(a), f(b)]
    while True:
        c = (a + b) / 2
        f_c = f(c)
        mid_points.append(c)
        mid_points_f.append(f_c)

        if math.fabs(f_c) < 1e-3:
            break

        if f_c * f(a) < 0:
            b = c
        else:
            a = c
    # x, y
    return mid_points, mid_points_f

def FalseP(a, b, f):
    if not callable(f):
        raise ValueError('f should be function')

    mid_points = [a, b]
    mid_points_f = [f(a), f(b)]
    count = 0
    while True:
        count += 1
        d = (a - b) / (f(a) - f(b))
        c = b - f(b) * d
        f_c = f(c)
        mid_points.append(c)
        mid_points_f.append(f_c)

        if math.fabs(f_c) < 1e-3 or count > 100:
            break

        if f_c * f(a) < 0:
            a = c
            b = a
        else:
            b = c
    # x, y
    return mid_points, mid_points_f




plt.figure(1)
# plt.plot(t1, f(t1), 'b', t2, f(t2), 'k')
xs, ys = FalseP(-5, 5, f)
print([(x, y) for x, y in zip(xs, ys)])
t1 = np.arange(min(xs), max(xs), 0.1)

plt.plot(xs, ys, 'r')
plt.plot(t1, f(t1), 'b--')
plt.show()
