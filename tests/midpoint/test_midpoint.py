#!/usr/bin/env python
# -*- coding: utf-8 -*-

import gc
import sys
import numpy as np
import pyximport
import pytest
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


@pytest.mark.parametrize("params", [None, dict(Ak=_Ak)])
def test_sine(params):
    tout = np.linspace(0, 4, 128)
    A, k = _Ak
    y0 = np.array([0., A*k])
    nIter = 70000

    gc.collect()
    nNone = sys.getrefcount(None)
    for i in range(nIter):
        yout, info = predefined(sine, y0, tout, params=params)
        time_wall = info['time_wall']
        yref0 = A*np.sin(k*tout)
        yref1 = A*np.cos(k*tout)*k
        yref = np.vstack([yref0, yref1]).T
        assert np.allclose(yref, yout, atol=0.1)
        assert 1e-9 < time_wall < 0.050  # takes about <1 ms on 2023 desktop computer
    gc.collect()
    nNone = sys.getrefcount(None) - nNone
    assert -nIter//10 < nNone < nIter//10


if __name__ == '__main__':
    test_sine(None)
    test_sine(dict(Ak=_Ak))
