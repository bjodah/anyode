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
    assert ef.get_ny() == 2
    tout = np.linspace(0, 4, 8192)
    y0 = np.array([0., A*k])
    assert ef.get_dx0(0, y0) == 0.0
    assert ef.get_dx_max(0, y0) == float('inf')
    yout, time_wall = ef.integrate(tout, y0)
    yref0 = A*np.sin(k*tout)
    yref1 = A*np.cos(k*tout)*k
    assert np.allclose(np.vstack([yref0, yref1]).T, yout, atol=0.05)
    assert 1e-9 < time_wall < 2.0  # takes about 20 ms on modern 2012 desktop computer

if __name__ == '__main__':
    test_sine()
