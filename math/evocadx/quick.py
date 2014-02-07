"""Various quick tools for data analysis."""

import pandas
import scipy
import scikits.bootstrap as bootstrap
import matplotlib.pyplot as mp

def quick_ci(g, x, fun=scipy.mean, alpha=0.05, n=200):
    import warnings
    warnings.simplefilter("ignore",bootstrap.InstabilityWarning)
    l,h = bootstrap.ci(data=g, statfunction=fun, alpha=alpha, n_samples=n)
    return pandas.DataFrame({x:g.name, 'mean':g.mean(), 'low':l, 'high':h}, index=[g.name])

def quick_ciplot(x, y, D):
    R = pandas.DataFrame(D.groupby(x)[y].apply(quick_ci, x))
    l, = mp.plot(R[x], R['mean'])
    mp.fill_between(R[x], R['low'], R['high'], facecolor=l.get_color(), alpha=0.5)
    return l

def quick_mean(x, y, D):
    R = pandas.DataFrame(D.groupby(x)[y].apply(quick_ci, x))
    mp.plot(R[x], R['mean'])
