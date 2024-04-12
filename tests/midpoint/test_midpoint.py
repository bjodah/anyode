#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import numpy as np
import pyximport
pyximport.install()

from _midpoint import predefined

_Ak = [2, 3]

def sine(t, y, fout, Ak=None):
    # x = A*sin(k*t)
    # x' = A*k*cos(k*t)
    # x'' = -A*k**2*sin(k*t)
    A, k = Ak or _Ak
    fout[0] = y[1]
    fout[1] = -k**2 * y[0]

def test_sine():
    tout = np.linspace(0, 4, 128)
    A, k = _Ak
    y0 = np.array([0., A*k])
    for params in [None, dict(Ak=_Ak)]:
        for i in range(70000):
            yout, info = predefined(sine, y0, tout, params=params)
            time_wall = info['time_wall']
            yref0 = A*np.sin(k*tout)
            yref1 = A*np.cos(k*tout)*k
            yref = np.vstack([yref0, yref1]).T
            #print(sys.getrefcount(None))
            assert np.allclose(yref, yout, atol=0.1)
            assert 1e-9 < time_wall < 0.010  # takes about <1 ms on 2023 desktop computer


if __name__ == '__main__':
    test_sine()
