#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
import pyximport
pyximport.install()

from _eulerfw import EulerForward

def sine(t, y, fout, Ak=None):
    # x = A*sin(k*t)
    # x' = A*k*cos(k*t)
    # x'' = -A*k**2*sin(k*t)
    A, k = Ak
    fout[0] = y[1]
    fout[1] = -k**2 * y[0]

def test_sine():
    A, k = [2, 3]
    ef = EulerForward(2, sine, dict(Ak=[A, k]))
    tout = np.linspace(0, 4, 8192)
    y0 = np.array([0., A*k])
    yout = ef.integrate(tout, y0)
    yref0 = A*np.sin(k*tout)
    yref1 = A*np.cos(k*tout)*k
    assert np.allclose(np.vstack([yref0, yref1]).T, yout, atol=0.05)

if __name__ == '__main__':
    test_sine()
