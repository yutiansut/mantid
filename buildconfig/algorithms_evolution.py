#!/usr/bin/python
# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
""" Utility for generating the dependency tree for an algorithm """
from __future__ import (absolute_import, division, print_function)
import sys, os, subprocess
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from dateutil.relativedelta import relativedelta

python_algorithms_path = '../Framework/PythonInterface/plugins/algorithms/'
python_workflow_algorithms_path = '../Framework/PythonInterface/plugins/algorithms/WorkflowAlgorithms/'
python_workflow_algorithms_sans_path = '../Framework/PythonInterface/plugins/algorithms/WorkflowAlgorithms/SANS/'

def diff_month(d1, d2):
    return (d1.year - d2.year) * 12 + d1.month - d2.month

if __name__== "__main__":
    from mantid.simpleapi import *
    from mantid.api import AlgorithmFactory, AlgorithmManager
    algs = AlgorithmFactory.getRegisteredAlgorithms(True)

    algs_dates = dict()

    for alg in sorted(algs):
        version = algs[alg]
        name = alg
        if version[0] != 1:
            name += str(version[0])

        full_path_python = python_algorithms_path + name + '.py'
        full_path_python_wkfl = python_workflow_algorithms_path + name + '.py'
        full_path_python_wkfl_sans = python_workflow_algorithms_sans_path + name + '.py'
        full_path_cpp = subprocess.check_output(['find', '../Framework', '-iname', '{0}.cpp'.format(name)])[:-1]

        path = ''

        if os.path.exists(full_path_python):
            path = full_path_python
        elif os.path.exists(full_path_python_wkfl):
            path = full_path_python_wkfl
        elif os.path.exists(full_path_python_wkfl_sans):
            path = full_path_python_wkfl_sans
        elif os.path.exists(full_path_cpp):
            path = full_path_cpp
        else:
            continue

        logs = subprocess.Popen(['git', 'log', '--format="%ad"', '--date=short', path], stdout=subprocess.PIPE)
        date = subprocess.check_output(['tail','-1'], stdin=logs.stdout)[:-1]
        algs_dates[alg] = mdates.date2num(datetime.strptime(date[1:-1],'%Y-%m-%d'))
        print(alg, '\t', date[1:-1])

    # check how many months it is since mantid is on github
    start_time = datetime(2015, 9, 1)
    months = diff_month(datetime.now(), start_time)

    fig, ax = plt.subplots(1, 1)
    ax.hist(algs_dates.values(), bins=months, cumulative=True, histtype='bar')
    ax.xaxis.set_minor_locator(mdates.MonthLocator())
    ax.xaxis.set_minor_formatter(mdates.DateFormatter('%m'))
    ax.xaxis.set_major_locator(mdates.YearLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y'))
    ax.set(ylabel="# of algorithms")
    ax.set(title="Algorithm evolution since moving to GitHub")
    plt.xticks(rotation=45)
    plt.show()
