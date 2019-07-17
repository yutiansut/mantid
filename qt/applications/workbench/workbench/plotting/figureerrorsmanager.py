# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
"""
Controls the dynamic displaying of errors for line on the plot
"""
from functools import partial
from matplotlib.container import ErrorbarContainer
from matplotlib.lines import Line2D
from qtpy.QtWidgets import QMenu

from mantid.plots import MantidAxes
from mantid.plots.helperfunctions import get_data_from_errorbar_container
from mantidqt.widgets.plotconfigdialog.curvestabwidget import (
    CurveProperties, curve_has_errors, remove_curve_from_ax, set_errorbars_hidden)


class FigureErrorsManager(object):
    ERROR_BARS_MENU_TEXT = "Error Bars"
    SHOW_ERROR_BARS_BUTTON_TEXT = "Show all errors"
    HIDE_ERROR_BARS_BUTTON_TEXT = "Hide all errors"

    AXES_NOT_MANTIDAXES_ERR_MESSAGE = "Plot axes are not MantidAxes. There is no way to automatically load error data."

    def __init__(self, canvas):
        self.canvas = canvas
        self.active_lines = []

    def add_error_bars_menu(self, parent_menu, ax):
        """
        Add menu actions to toggle the errors for all lines in the plot.

        Lines without errors are added in the context menu first,
        then lines containing errors are appended afterwards.

        This is done so that the context menu always has
        the same order of curves as the legend is currently showing - and the
        legend always appends curves with errors after the lines without errors.
        Relevant source, as of 10 July 2019:
        https://github.com/matplotlib/matplotlib/blob/154922992722db37a9d9c8680682ccc4acf37f8c/lib/matplotlib/legend.py#L1201

        :param parent_menu: The menu to which the actions will be added
        :type parent_menu: QMenu
        :param ax: The Axes containing lines to toggle errors on
        """
        # if the ax is not a MantidAxes, and there are no errors plotted,
        # then do not add any options for the menu
        if not isinstance(ax, MantidAxes) and len(ax.containers) == 0:
            return

        error_bars_menu = QMenu(self.ERROR_BARS_MENU_TEXT, parent_menu)
        error_bars_menu.addAction(self.SHOW_ERROR_BARS_BUTTON_TEXT,
                                  partial(self._update_plot_after, self._toggle_all_errors, ax, make_visible=True))
        error_bars_menu.addAction(self.HIDE_ERROR_BARS_BUTTON_TEXT,
                                  partial(self._update_plot_after, self._toggle_all_errors, ax, make_visible=False))
        parent_menu.addMenu(error_bars_menu)

        self.active_lines = self.get_curves_from_ax(ax)

        # if there's more than one line plotted, then
        # add a sub menu, containing an action to hide the
        # error bar for each line
        error_bars_menu.addSeparator()
        add_later = []
        for index, line in enumerate(self.active_lines):
            if curve_has_errors(line):
                curve_props = CurveProperties.from_curve(line)
                # Add lines without errors first, lines with errors are appended later. Read docstring for more info
                if not isinstance(line, ErrorbarContainer):
                    action = error_bars_menu.addAction(line.get_label(), partial(
                        self._update_plot_after, self.toggle_error_bars_for, ax, line))
                    action.setCheckable(True)
                    action.setChecked(not curve_props.hide_errors)
                else:
                    add_later.append((line.get_label(), partial(
                        self._update_plot_after, self.toggle_error_bars_for, ax, line),
                                      not curve_props.hide_errors))

        for label, function, visible in add_later:
            action = error_bars_menu.addAction(label, function)
            action.setCheckable(True)
            action.setChecked(visible)

    def _toggle_all_errors(self, ax, make_visible):
        for line in self.active_lines:
            if curve_has_errors(line):
                self.toggle_error_bars_for(ax, line, make_visible)

    @staticmethod
    def toggle_error_bars_for(ax, curve, make_visible=None):
        # get all curve properties
        curve_props = CurveProperties.from_curve(curve)
        # and remove the ones that matplotlib doesn't recognise
        plot_kwargs = curve_props.get_plot_kwargs()
        new_curve = FigureErrorsManager.replot_curve(ax, curve, plot_kwargs)

        # Inverts either the current state of hide_errors
        # or the make_visible kwarg that forces a state:
        # If make visible is True, then hide_errors must be False
        # for the intended effect
        curve_props.hide_errors = not curve_props.hide_errors if make_visible is None else not make_visible

        FigureErrorsManager.toggle_errors(new_curve, curve_props)
        FigureErrorsManager.update_limits_and_legend(ax)

    def _update_plot_after(self, func, *args, **kwargs):
        """
        Updates the legend and the plot after the function has been executed.
        Used to funnel through the updates through a common place

        :param func: Function to be executed, before updating the plot
        :param args: Arguments forwarded to the function
        :param kwargs: Keyword arguments forwarded to the function
        """
        func(*args, **kwargs)
        self.canvas.draw()

    @staticmethod
    def _supported_ax(ax):
        return hasattr(ax, 'creation_args')

    @staticmethod
    def toggle_errors(curve, view_props):
        setattr(curve, 'hide_errors', view_props.hide_errors)
        set_errorbars_hidden(curve, view_props.hide_errors)

    @staticmethod
    def get_errorbars_from_ax(ax):
        return [cont for cont in ax.containers if isinstance(cont, ErrorbarContainer)]

    @staticmethod
    def get_curves_from_ax(ax):
        return ax.get_lines() + FigureErrorsManager.get_errorbars_from_ax(ax)

    @classmethod
    def replot_curve(cls, ax, curve, plot_kwargs):
        if isinstance(ax, MantidAxes):
            try:
                new_curve = ax.replot_artist(curve, errorbars=True, **plot_kwargs)
            except ValueError:  # ValueError raised if Artist not tracked by Axes
                new_curve = cls._replot_mpl_curve(ax, curve, plot_kwargs)
        else:
            new_curve = cls._replot_mpl_curve(ax, curve, plot_kwargs)
        setattr(new_curve, 'errorevery', plot_kwargs.get('errorevery', 1))
        return new_curve

    @staticmethod
    def _replot_mpl_curve(ax, curve, plot_kwargs):
        """
        Replot the given matplotlib curve with new kwargs
        :param ax: The axis that the curve will be plotted on
        :param curve: The curve that will be replotted
        :param plot_kwargs: Kwargs for the plot that will be passed onto matplotlib
        """
        remove_curve_from_ax(curve)
        if isinstance(curve, Line2D):
            [plot_kwargs.pop(arg, None) for arg in
             ['capsize', 'capthick', 'ecolor', 'elinewidth', 'errorevery']]
            new_curve = ax.plot(curve.get_xdata(), curve.get_ydata(),
                                **plot_kwargs)[0]
        elif isinstance(curve, ErrorbarContainer):
            # Because of "error every" option, we need to store the original
            # error bar data on the curve or we will lose data on re-plotting
            x, y, xerr, yerr = getattr(curve, 'errorbar_data',
                                       get_data_from_errorbar_container(curve))
            new_curve = ax.errorbar(x, y, xerr=xerr, yerr=yerr, **plot_kwargs)
            setattr(new_curve, 'errorbar_data', [x, y, xerr, yerr])
        else:
            raise ValueError("Curve must have type 'Line2D' or 'ErrorbarContainer'. Found '{}'".format(type(curve)))
        return new_curve

    @staticmethod
    def update_limits_and_legend(ax):
        ax.relim()
        ax.autoscale()
        if ax.legend_:
            ax.legend().draggable()
