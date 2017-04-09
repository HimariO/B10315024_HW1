import numpy as np
import matplotlib.pyplot as plt
import math
from termcolor import colored


def Bisection(a, b, f):
    if not callable(f):
        raise ValueError('f should be function')

    mid_points = [a, b]
    mid_points_f = [f(a), f(b)]
    count = 0
    print(colored('[Bisection]', color='green'))

    while True:
        count += 1
        print('[%f, %f]' % (a, b))

        c = (a + b) / 2
        f_c = f(c)
        mid_points.append(c)
        mid_points_f.append(f_c)


        if f_c * f(a) < 0:
            b = c
        else:
            a = c

        if math.fabs(f_c) < 1e-3 or count > 100 or math.fabs(a / b) < 1e-3:
            break
    # x, y
    return mid_points, mid_points_f


def FalseP(a, b, f):
    if not callable(f):
        raise ValueError('f should be function')

    mid_points = [a, b]
    mid_points_f = [f(a), f(b)]
    count = 0

    f0 = f(a)
    f1 = f(b)

    print(colored('[FalseP]', color='green'))
    while True:
        count += 1
        print('[%f, %f]' % (a, b))

        if f1 - f0 == 0:
            print(colored('[FalseP]Delta y is too small.', color='red'))
            break

        d = (b - a) / (f1 - f0)
        c = b - f1 * d

        f0 = f1
        f1 = f(c)
        mid_points.append(c)
        mid_points_f.append(f1)


        if f1 * f(b) < 0:
            a = c
            b = b
        else:
            b = c

        if math.fabs(f1) < 1e-3 or count > 100 or math.fabs(a / b) < 1e-4:
            break
    # x, y
    return mid_points, mid_points_f


def Secant(a, b, f):
    if not callable(f):
        raise ValueError('f should be function')

    mid_points = [a, b]
    mid_points_f = [f(a), f(b)]
    count = 0

    f0 = f(a)
    f1 = f(b)

    print(colored('[Secant]', color='green'))
    while True:
        count += 1
        print('[%f, %f]' % (a, b))

        if f1 - f0 == 0:
            print(colored('[Secant]Delta y is too small.', color='red'))
            break

        d = (b - a) / (f1 - f0)
        c = b - f1 * d

        f0 = f1
        f1 = f(c)
        mid_points.append(c)
        mid_points_f.append(f1)


        a = b
        b = c
        if math.fabs(f1) < 1e-3 or count > 100 or math.fabs(a / b) < 1e-3:
            break
    print('Finish')
    return mid_points, mid_points_f


def Newton(p0, f, df):
    if not callable(f):
        raise ValueError('f should be function')

    mid_points = [p0]
    mid_points_f = [f(p0)]
    count = 0

    print(colored('[Newton]', color='green'))
    while True:
        count += 1
        print(p0)

        c = p0 - f(p0) / df(p0)
        f_c = f(c)
        mid_points.append(c)
        mid_points_f.append(f_c)

        if math.fabs(f_c) < 1e-3 or count > 100:
            break

        p0 = c
    # x, y
    return mid_points, mid_points_f
