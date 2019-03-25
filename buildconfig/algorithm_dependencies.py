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
    
def dependencies(name, algs, all_used_algs, level=0):
    version = algs[name]
    used_algs = []
    vname = name
    if version[0] != 1:
        vname += str(version[0])
        
    full_path_python = python_algorithms_path+vname+ '.py'
    full_path_python_wkfl = python_workflow_algorithms_path+vname+ '.py'
    full_path_cpp = subprocess.check_output(['find','../Framework','-iname','{0}.cpp'.format(vname)])[:-1]

    is_python = False

    if os.path.exists(full_path_python):
        path = full_path_python
        is_python = True
    elif os.path.exists(full_path_python_wkfl):
        path = full_path_python_wkfl
        is_python = True
    elif os.path.exists(full_path_cpp):
        path = full_path_cpp
    else:
        return

    with open(path, 'r') as file:
        content = file.read()
        for alg in algs.keys():
            alg_object = AlgorithmManager.createUnmanaged(name)
            if alg != name and alg not in used_algs and alg not in alg_object.seeAlso():
                if is_python:
                    if content.count(alg+'(') > 0 or content.count('(\''+alg) > 0 or content.count('(\"'+alg) > 0:
                        used_algs.append(alg)
                else:
                    if content.count('(\"'+alg) > 0:
                        used_algs.append(alg)

    offset = ''
    l = 0
    while l < level:
       offset += '\t'
       l += 1
    for used in sorted(used_algs):
        if used not in all_used_algs:
            all_used_algs.append(used)
            dependencies(used, algs, all_used_algs, level + 1)
        
    return

if __name__== "__main__":
    
    if len(sys.argv) < 2:
        exit("Requires the algorithm name or names as arguments!")

    python_algorithms_path = '../Framework/PythonInterface/plugins/algorithms/'
    python_workflow_algorithms_path = '../Framework/PythonInterface/plugins/algorithms/WorkflowAlgorithms/'

    from mantid.simpleapi import *
    from mantid.api import AlgorithmFactory, AlgorithmManager
    algs = AlgorithmFactory.getRegisteredAlgorithms(True)
    all_used_algs = []
    for alg_name in sys.argv[1:]:
        if alg_name not in algs.keys():
            print("Provided algorithm {} is not registered in AlgorithmFactory".format(alg_name))
        else:
            dependencies(alg_name, algs, all_used_algs)

    print(sorted(all_used_algs))
    print("Total {}".format(len(all_used_algs)))
