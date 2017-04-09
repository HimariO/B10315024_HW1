import numpy as np
import matplotlib.pyplot as plt
import math
from findingM import *
from termcolor import colored


def f(x):
    c = [-11, -3, 2]  # reverse order
    return sum([c[i] * x ** (i) for i in range(len(c))])


def df(x):
    c = [-3, 2]  # reverse order
    return sum([(i + 1) * c[i] * (x ** i) for i in range(len(c))])


def f2(x):
    return 2 ** x - 0.5


def df2(x):
    return np.log(2) * 2 ** x


def f3(x):
    return np.cos(x ** 2)


def df3(x):
    return -np.sin(x ** 2)


def ploting(xs, ys, grid, f, title=None):
    plt.subplot(*grid)

    t1 = np.arange(min(xs), max(xs), 0.1)
    plt.plot(
        xs, ys, 'r',
        xs[1:-1], ys[1:-1], 'bo',
        xs[:1], ys[:1], 'go',
        xs[-1:], ys[-1:], 'ro'
    )
    plt.plot(t1, f(t1), 'b--')
    plt.margins(0.1, 0.1)

    if title is not None:
        plt.title(title)

F = f
dF = df

plt.figure(1)
xs, ys = Bisection(-3, 5, F)
ploting(xs, ys, [2, 2, 1], F, title='Bisection')

xs, ys = Secant(-3, 5, F)
ploting(xs, ys, [2, 2, 2], F, title='Secant')

xs, ys = FalseP(-3, 5, F)
ploting(xs, ys, [2, 2, 3], F, title='False Position')

xs, ys = Newton(-3, F, dF)
ploting(xs, ys, [2, 2, 4], F, title='Newton')

plt.show()
