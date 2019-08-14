# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.

from __future__ import (absolute_import, unicode_literals)

from mantid.plots import MantidAxes
from mantidqt.widgets.plotconfigdialog import curve_in_ax
from workbench.plugins.editor import DEFAULT_CONTENT
from workbench.plotting.plotscriptgenerator.axes import generate_add_subplot_command, generate_axis_limit_commands
from workbench.plotting.plotscriptgenerator.figure import generate_figure_command
from workbench.plotting.plotscriptgenerator.lines import generate_plot_command
from workbench.plotting.plotscriptgenerator.utils import generate_workspace_retrieval_commands

FIG_VARIABLE = "fig"
AXES_VARIABLE = "ax"


def generate_script(fig, exclude_headers=False):
    """
    Generate a script to recreate a figure.

    This currently only supports recreating artists that were plotted
    from a Workspace. The format of the outputted script is as follows:

        <Default Workbench script contents (imports)>
        <Workspace retrieval from ADS>
        fig = plt.figure()
        ax = fig.add_subplot()
        ax.plot() or ax.errorbar()
        ax.legend().draggable()     (if legend present)
        plt.show()

    :param fig: A matplotlib.pyplot.Figure object you want to create a script from
    :param exclude_headers: Boolean. Set to True to ignore imports/headers
    :return: A String. A script to recreate the given figure
    """
    plot_commands = []
    for ax in fig.get_axes():
        if not isinstance(ax, MantidAxes) or not curve_in_ax(ax):
            continue
        plot_commands.append("{} = {}.{}"
                             "".format(AXES_VARIABLE, FIG_VARIABLE,
                                       generate_add_subplot_command(ax)))
        for artist in ax.get_tracked_artists():
            plot_commands.append("{}.{}".format(AXES_VARIABLE, generate_plot_command(artist)))
        axis_limit_cmds = generate_axis_limit_commands(ax)
        plot_commands += ["{}.{}".format(AXES_VARIABLE, cmd) for cmd in axis_limit_cmds]
        if ax.legend_:
            plot_commands.append("{}.legend().draggable()".format(AXES_VARIABLE))
    if not plot_commands:
        return
    cmds = [] if exclude_headers else [DEFAULT_CONTENT]
    cmds += generate_workspace_retrieval_commands(fig) + ['']
    cmds.append("{} = {}".format(FIG_VARIABLE, generate_figure_command(fig)))
    cmds += plot_commands
    cmds.append("plt.show()")
    return '\n'.join(cmds)
