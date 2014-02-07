"""This module contains common routines useful for research with ipython notebook."""

import os
import re
import pandas
import subprocess
import StringIO

from quick import *

def find_files(top, pattern):
    """Recursively find files from top whose full name matches pattern."""
    matched_files=[]
    for dirname, dirnames, filenames in os.walk(top):
        for filename in filenames:
            name = os.path.join(dirname, filename)
            if re.search(pattern, name) is not None:
                matched_files.append(name)
    return matched_files


def load_files(files, treatment=None, trial=None):
    """Load data from files into a pandas.DataFrame."""

    D = None
    count = 0
    for i in files:
        # we have to do all this subprocess junk because the python
        # gzip module is buggy...
        cmd = 'gunzip -cqf ' + i
        p = subprocess.Popen(cmd, shell=True, 
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
        text, stderr = p.communicate()
        fh = StringIO.StringIO(text)
        d = pandas.read_table(fh, sep=r"\s+(?!$)")
       
        if treatment is None:
            m = re.search(r".*\/(\w+)_(\d+)\/[\w\.]*$", i)
            if m is not None:
                d['treatment'] = m.group(1)
                d['trial'] = m.group(2)
        else:
            d['treatment'] = treatment

        if (trial is not None) and (trial in ("series")):
            d['trial'] = str(count)
            count += 1

        if D is None:
            D = d
        else:
            D = D.append(d, ignore_index=True)

    return D
