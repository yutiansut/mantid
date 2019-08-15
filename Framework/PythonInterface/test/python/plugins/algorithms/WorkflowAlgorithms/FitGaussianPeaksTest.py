# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +

from __future__ import (absolute_import, division, print_function, unicode_literals)

import numpy as np
import unittest

from mantid.simpleapi import CreateEmptyTableWorkspace, CreateWorkspace, DeleteWorkspace, FitGaussianPeaks
from mantid.api import *
from mantid.py3compat import mock

import plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks as _FitGaussianPeaks


class FitGaussianPeaksTest(unittest.TestCase):
    data_ws = None
    peak_guess_table = None
    peak_table_header = [
        'centre', 'error centre', 'height', 'error height', 'sigma', 'error sigma', 'area',
        'error area'
    ]
    alg_instance = None
    x_values = None
    y_values = None

    def setUp(self):
        # Creating two peaks on an exponential background with gaussian noise
        self.x_values = np.linspace(0, 100, 1001)
        centre = [25, 75]
        height = [35, 20]
        width = [10, 5]
        self.y_values = self.gaussian(self.x_values, centre[0], height[0], width[0])
        self.y_values += self.gaussian(self.x_values, centre[1], height[1], width[1])
        background = 10 * np.ones(len(self.x_values))
        self.y_values += background

        # Generating a table with a guess of the position of the centre of the peaks
        peak_table = CreateEmptyTableWorkspace()
        peak_table.addColumn(type='float', name='Approximated Centre')
        peak_table.addRow([centre[0] + 2])
        peak_table.addRow([centre[1] - 3])

        # Generating a workspace with the data and a flat background
        data_ws = CreateWorkspace(DataX=np.concatenate((self.x_values, self.x_values)),
                                  DataY=np.concatenate((self.y_values, background)),
                                  DataE=np.sqrt(np.concatenate((self.y_values, background))),
                                  NSpec=2)

        self.data_ws = data_ws
        self.peak_guess_table = peak_table

        self.alg_instance = _FitGaussianPeaks.FitGaussianPeaks()

    def tearDown(self):
        self.delete_if_present('data_ws')
        self.delete_if_present('peak_guess_table')
        self.delete_if_present('peak_table')
        self.delete_if_present('refit_peak_table')
        self.delete_if_present('fit_cost')
        self.delete_if_present('fit_result_NormalisedCovarianceMatrix')
        self.delete_if_present('fit_result_Parameters')
        self.delete_if_present('fit_result_Workspace')
        self.delete_if_present('fit_table')
        self.delete_if_present('data_table')
        self.delete_if_present('refit_data_table')
        self.delete_if_present('fit_result')

        self.alg_instance = None
        self.peak_guess_table = None
        self.data_ws = None

    @staticmethod
    def delete_if_present(workspace):
        if workspace in mtd:
            DeleteWorkspace(workspace)

    @staticmethod
    def gaussian(xvals, centre, height, sigma):
        exp_val = (xvals - centre) / (np.sqrt(2) * sigma)

        return height * np.exp(-exp_val * exp_val)

    @staticmethod
    def simulate_fit_parameter_output(values, cost):
        table = CreateEmptyTableWorkspace()
        table.addColumn(type='str', name='Name')
        table.addColumn(type='float', name='Value')
        table.addColumn(type='float', name='Error')

        for i, (val, err) in enumerate(values):
            name = ''
            if i % 3 == 0:
                name = 'centre'
            elif i % 3 == 1:
                name = 'height'
            elif i % 3 == 2:
                name = 'sigma'

            table.addRow([name, val, err])

        return table

    def test_algorithm_with_bad_input_workspace_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace='ws-that-does-not-exist')

    def test_algorithm_with_negative_centre_tolerance_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace=self.data_ws,
                             PeakGuessTable=self.peak_guess_table,
                             CentreTolerance=-1.0)

    def test_algorithm_with_negative_estimated_peak_sigma_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace=self.data_ws,
                             PeakGuessTable=self.peak_guess_table,
                             EstimatedPeakSigma=-1.0)

    def test_algorithm_with_negative_min_peak_sigma_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace=self.data_ws,
                             PeakGuessTable=self.peak_guess_table,
                             MinPeakSigma=-1.0)

    def test_algorithm_with_negative_max_peak_sigma_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace=self.data_ws,
                             PeakGuessTable=self.peak_guess_table,
                             MaxPeakSigma=-1.0)

    def test_algorithm_with_negative_general_fit_tolerance_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace=self.data_ws,
                             PeakGuessTable=self.peak_guess_table,
                             GeneralFitTolerance=-1.0)

    def test_algorithm_with_negative_refit_tolerance_throws(self):
        with self.assertRaises(Exception):
            FitGaussianPeaks(InputWorkspace=self.data_ws,
                             PeakGuessTable=self.peak_guess_table,
                             RefitTolerance=-1.0)

    def test_algorithm_creates_correct_tables(self):
        FitGaussianPeaks(InputWorkspace=self.data_ws, PeakGuessTable=self.peak_guess_table)

        self.assertIn('peak_table', mtd)
        self.assertIn('refit_peak_table', mtd)
        self.assertIn('fit_cost', mtd)

        self.assertEqual(list(mtd['peak_table'].getColumnNames()), self.peak_table_header)
        self.assertEqual(list(mtd['refit_peak_table'].getColumnNames()), self.peak_table_header)
        self.assertEqual(list(mtd['fit_cost'].getColumnNames()), ['Chi2', 'Poisson'])

    def test_parse_fit_table_correctly_formats_the_table(self):
        peaks = [(35.2, 0.4), (25.03, 0.1), (10.03, 0.05)]
        peaks += [(20.003, 0.004), (75.15, 0.2), (5.2, 0.05)]
        fit_table = self.simulate_fit_parameter_output(peaks, 100.034)
        data_table = CreateEmptyTableWorkspace()

        to_refit = self.alg_instance.parse_fit_table(fit_table, data_table, refit=False)

        self.assertEqual(to_refit, [])
        self.assertEqual(data_table.getColumnNames(), self.peak_table_header)
        np.testing.assert_almost_equal(data_table.column(0), [25.03, 75.15], 5)
        np.testing.assert_almost_equal(data_table.column(1), [0.1, 0.2], 5)
        np.testing.assert_almost_equal(data_table.column(2), [35.2, 20.003], 5)
        np.testing.assert_almost_equal(data_table.column(3), [0.4, 0.004], 5)
        np.testing.assert_almost_equal(data_table.column(4), [10.03, 5.2], 5)
        np.testing.assert_almost_equal(data_table.column(5), [0.05, 0.05], 5)

    def test_parse_fit_table_marks_peaks_for_refitting_if_error_larger_than_value(self):
        peaks = [(35.2, 0.4), (25.03, 0.1), (10.03, 0.05)]
        peaks += [(20.003, 40.22), (75.15, 0.2), (5.2, np.NaN)]
        fit_table = self.simulate_fit_parameter_output(peaks, 100.034)
        data_table = CreateEmptyTableWorkspace()

        to_refit = self.alg_instance.parse_fit_table(fit_table, data_table, refit=True)

        self.assertEqual(data_table.getColumnNames(), self.peak_table_header)
        np.testing.assert_almost_equal(data_table.column(0), [25.03], 5)
        np.testing.assert_almost_equal(data_table.column(1), [0.1], 5)
        np.testing.assert_almost_equal(data_table.column(2), [35.2], 5)
        np.testing.assert_almost_equal(data_table.column(3), [0.4], 5)
        np.testing.assert_almost_equal(data_table.column(4), [10.03], 5)
        np.testing.assert_almost_equal(data_table.column(5), [0.05], 5)
        np.testing.assert_almost_equal(to_refit, [(75.15, 20.003, 5.2)], 5)

    def test_parse_fit_table_does_not_refit_if_error_lower_than_value(self):
        peaks = [(35.2, 0.4), (25.03, 0.1), (10.03, 0.05)]
        peaks += [(20.003, 0.004), (75.15, 0.2), (5.2, 0.05)]
        fit_table = self.simulate_fit_parameter_output(peaks, 100.034)
        data_table = CreateEmptyTableWorkspace()

        to_refit = self.alg_instance.parse_fit_table(fit_table, data_table, refit=True)

        self.assertEqual(to_refit, [])

    def test_gaussian_peak_returns_correct_values(self):
        params = [20, 10, 3]

        expected = self.gaussian(self.x_values, *params)
        actual = self.alg_instance.gaussian_peak(self.x_values, *params)

        np.testing.assert_almost_equal(expected, actual)

    def test_gaussian_peak_background_returns_correct_value(self):
        params = [2.0, 0.5, 20, 10, 3]

        hyp = np.ones(len(self.x_values))
        expected = params[0] + self.x_values * params[1]
        expected += self.gaussian(self.x_values, *params[2:])
        expected -= hyp
        actual = self.alg_instance.gaussian_peak_background(params, self.x_values, hyp)

        np.testing.assert_almost_equal(expected, actual)

    def test_multi_peak_returns_correct_value(self):
        params = [20, 10, 3, 60, 20, 2]

        hyp = np.ones(len(self.x_values))
        expected = self.gaussian(self.x_values, *params[:3])
        expected += self.gaussian(self.x_values, *params[3:])
        expected -= hyp
        actual = self.alg_instance.multi_peak(params, self.x_values, hyp)

        np.testing.assert_almost_equal(expected, actual)

    def test_function_difference_returns_correct_value(self):
        hyp = np.ones(len(self.x_values))

        expected = np.sum(np.power(self.y_values - hyp, 2)) / len(self.x_values)
        actual = self.alg_instance.function_difference(self.y_values, hyp)

        self.assertAlmostEqual(expected, actual)

    def test_poisson_cost_returns_correct_value(self):
        hyp = np.ones(len(self.x_values))
        yvals = np.array(
            [max(x, y) for x, y in zip(0.001 * np.ones(len(self.x_values)), self.y_values)])

        expected = np.sum(-yvals + hyp * np.log(yvals))
        actual = self.alg_instance.poisson_cost(hyp, yvals)

        self.assertAlmostEqual(expected, actual)

    def test_evaluate_cost_returns_the_expected_value_with_chi2(self):
        peaks = [(35.2, 0.4), (25.03, 0.1), (10.03, 0.05)]
        peaks += [(20.003, 0.004), (75.15, 0.2), (5.2, 0.05)]
        params = [25.03, 35.2, 10.03, 75.15, 20.003, 5.2]
        fit_table = self.simulate_fit_parameter_output(peaks, 100.034)
        data_table = CreateEmptyTableWorkspace()
        refit_data_table = CreateEmptyTableWorkspace()

        _ = self.alg_instance.parse_fit_table(fit_table, data_table, refit=False)

        cost = self.alg_instance.evaluate_cost(self.x_values,
                                               self.data_ws.readY(0),
                                               self.data_ws.readY(1),
                                               peak_param=data_table,
                                               refit_peak_param=refit_data_table,
                                               use_poisson=False)
        model = self.alg_instance.multi_peak(params, self.x_values, np.zeros(len(self.x_values)))
        expected = self.alg_instance.function_difference(model, self.data_ws.readY(0))

        self.assertAlmostEqual(cost, expected, 5)

    def test_evaluate_cost_returns_the_expected_value_with_poisson(self):
        peaks = [(35.2, 0.4), (25.03, 0.1), (10.03, 0.05)]
        peaks += [(20.003, 0.004), (75.15, 0.2), (5.2, 0.05)]
        params = [25.03, 35.2, 10.03, 75.15, 20.003, 5.2]
        fit_table = self.simulate_fit_parameter_output(peaks, 100.034)
        data_table = CreateEmptyTableWorkspace()
        refit_data_table = CreateEmptyTableWorkspace()

        _ = self.alg_instance.parse_fit_table(fit_table, data_table, refit=False)

        cost = self.alg_instance.evaluate_cost(self.x_values,
                                               self.data_ws.readY(0),
                                               self.data_ws.readY(1),
                                               peak_param=data_table,
                                               refit_peak_param=refit_data_table,
                                               use_poisson=True)
        model = self.alg_instance.multi_peak(params, self.x_values, np.zeros(len(self.x_values)))
        expected = self.alg_instance.poisson_cost(
            self.data_ws.readY(0) + self.data_ws.readY(1), model + self.data_ws.readY(1))

        self.assertAlmostEqual(cost, expected, 3)

    def test_estimate_single_parameters_returns_correct_number_of_parameters(self):
        ret = self.alg_instance.estimate_single_parameters(self.x_values, self.y_values,
                                                           np.argmax(self.y_values), 10)

        self.assertEqual(len(ret), 3)

    def test_estimate_parameters_returns_correct_number_parameters(self):
        idx = [250, 750]

        ret = self.alg_instance.estimate_parameters(self.x_values, self.y_values, idx, 10)

        self.assertEqual(len(ret), 6)

    def test_general_fit_returns_if_given_no_peaks(self):
        yvals, params = self.alg_instance.general_fit(self.x_values, self.y_values, [])

        np.testing.assert_equal(np.zeros(len(self.y_values)), yvals)
        self.assertEqual(None, params)

    def test_fit_function_is_called_correctly_when_given_one_peak(self):
        with mock.patch('plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.Fit') as mock_fit:
            self.alg_instance.estimate_single_parameters = mock.Mock(return_value=[1, 2, 3])
            self.alg_instance.getPropertyValue = mock.Mock(return_value='ws')
            ret_ws = CreateWorkspace(DataX=[1, 2, 3, 4], DataY=[5, 6, 7, 8], NSpec=2)
            mock_fit.return_value = (None, None, None, 'parameters', ret_ws, None, None)

            yvals, params = self.alg_instance.general_fit(self.x_values, self.y_values, [10])

            self.assertEqual(params, 'parameters')
            np.testing.assert_equal(yvals, [7, 8])
            mock_fit.assert_called_with(
                Function='name=Gaussian,PeakCentre=1.000000,Height=2.000000,Sigma=3.000000;',
                InputWorkspace='ws',
                Output='fit_result',
                Minimizer='Levenberg-MarquardtMD',
                OutputCompositeMembers=True,
                StartX=min(self.x_values),
                EndX=max(self.x_values),
                Constraints=
                '0.900000<PeakCentre<1.100000,10.768245<Height<13.161188,0.000000<Sigma<30,')

    # Separating the cases 1/multiple peaks tests that the constraints are named correctly in both cases
    def test_fit_function_is_called_correctly_when_given_multiple_peaks(self):
        with mock.patch('plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.Fit') as mock_fit:
            self.alg_instance.estimate_single_parameters = mock.Mock(return_value=[1, 2, 3])
            self.alg_instance.getPropertyValue = mock.Mock(return_value='ws')
            ret_ws = CreateWorkspace(DataX=[1, 2, 3, 4], DataY=[5, 6, 7, 8], NSpec=2)
            mock_fit.return_value = (None, None, None, 'parameters', ret_ws, None, None)

            yvals, params = self.alg_instance.general_fit(self.x_values, self.y_values, [10, 20])

            self.assertEqual(params, 'parameters')
            np.testing.assert_equal(yvals, [7, 8])
            mock_fit.assert_called_with(
                Function='name=Gaussian,PeakCentre=1.000000,Height=2.000000,Sigma=3.000000;'
                'name=Gaussian,PeakCentre=1.000000,Height=2.000000,Sigma=3.000000;',
                InputWorkspace='ws',
                Output='fit_result',
                Minimizer='Levenberg-MarquardtMD',
                OutputCompositeMembers=True,
                StartX=min(self.x_values),
                EndX=max(self.x_values),
                Constraints=
                '0.900000<f0.PeakCentre<1.100000,10.768245<f0.Height<13.161188,0.000000<f0.Sigma<30,'
                '1.800000<f1.PeakCentre<2.200000,11.236669<f1.Height<13.733706,0.000000<f1.Sigma<30,'
            )

    @mock.patch(
        'plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.FitGaussianPeaks.getProperty')
    def test_refit_peaks_uses_xvalue_from_input_workspace(self, mock_get_property):
        self.alg_instance.refit_peaks([])
        calls = [
            mock.call('InputWorkspace'),
            mock.call().value.readX(0),
            mock.call().value.readX().copy()
        ]
        mock_get_property.assert_has_calls(calls, any_order=True)

    @mock.patch(
        'plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.FitGaussianPeaks.getProperty')
    def test_refit_peaks_returns_if_given_no_parameters(self, mock_get_property):
        self.alg_instance.refit_peaks([])
        mock_get_property().value.readX().copy.return_value = [1, 2, 3, 4]
        yvals, params = self.alg_instance.refit_peaks([])

        self.assertEqual(params, None)
        np.testing.assert_equal(yvals, np.zeros(4))

    @mock.patch('plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.Fit')
    @mock.patch(
        'plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.FitGaussianPeaks.getProperty')
    def test_refit_peaks_is_called_correctly_when_given_one_peak(self, mock_get_property, mock_fit):
        mock_get_property().value.readX().copy.return_value = self.x_values
        self.alg_instance.getPropertyValue = mock.Mock(return_value='ws')
        ret_ws = CreateWorkspace(DataX=[1, 2, 3, 4], DataY=[5, 6, 7, 8], NSpec=2)
        mock_fit.return_value = (None, None, None, 'parameters', ret_ws, None, None)

        yvals, params = self.alg_instance.refit_peaks([(1, 2, 3)])

        self.assertEqual(params, 'parameters')
        np.testing.assert_equal(yvals, [7, 8])
        mock_fit.assert_called_with(
            Function='name=Gaussian,PeakCentre=1.000000,Height=2.000000,Sigma=3.000000;',
            InputWorkspace='ws',
            Output='fit_result',
            Minimizer='Levenberg-MarquardtMD',
            OutputCompositeMembers=True,
            StartX=min(self.x_values),
            EndX=max(self.x_values),
            Constraints='0.999000<PeakCentre<1.001000,1.998000<Height<2.002000,0.000000<Sigma<30,')

    # Separating the cases 1/multiple peaks tests that the constraints are named correctly in both cases
    @mock.patch('plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.Fit')
    @mock.patch(
        'plugins.algorithms.WorkflowAlgorithms.FitGaussianPeaks.FitGaussianPeaks.getProperty')
    def test_refit_peaks_is_called_correctly_when_given_multiple_peaks(
            self, mock_get_property, mock_fit):
        mock_get_property().value.readX().copy.return_value = self.x_values
        self.alg_instance.getPropertyValue = mock.Mock(return_value='ws')
        ret_ws = CreateWorkspace(DataX=[1, 2, 3, 4], DataY=[5, 6, 7, 8], NSpec=2)
        mock_fit.return_value = (None, None, None, 'parameters', ret_ws, None, None)

        yvals, params = self.alg_instance.refit_peaks([(1, 2, 3), (4, 5, 6)])

        self.assertEqual(params, 'parameters')
        np.testing.assert_equal(yvals, [7, 8])
        mock_fit.assert_called_with(
            Function='name=Gaussian,PeakCentre=1.000000,Height=2.000000,Sigma=3.000000;'
            'name=Gaussian,PeakCentre=4.000000,Height=5.000000,Sigma=6.000000;',
            InputWorkspace='ws',
            Output='fit_result',
            Minimizer='Levenberg-MarquardtMD',
            OutputCompositeMembers=True,
            StartX=min(self.x_values),
            EndX=max(self.x_values),
            Constraints=
            '0.999000<f0.PeakCentre<1.001000,1.998000<f0.Height<2.002000,0.000000<f0.Sigma<30,'
            '3.996000<f1.PeakCentre<4.004000,4.995000<f1.Height<5.005000,0.000000<f1.Sigma<30,')


if __name__ == '__main__':
    unittest.main()
