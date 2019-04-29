# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
import sys
import unittest

from mantid.api import AnalysisDataService
from mantid.api import FileFinder
from mantid.dataobjects import Workspace2D

from Muon.GUI.Common.contexts.muon_context import MuonContext
from Muon.GUI.Common.contexts.muon_data_context import MuonDataContext
from Muon.GUI.Common.contexts.muon_group_pair_context import MuonGroupPairContext
from Muon.GUI.Common.contexts.muon_gui_context import MuonGuiContext
from Muon.GUI.Common.muon_load_data import MuonLoadData
from Muon.GUI.Common.utilities.load_utils import load_workspace_from_filename

if sys.version_info.major < 2:
    pass
else:
    pass


class MuonContextTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def setUp(self):
        AnalysisDataService.clear()
        self.filepath = FileFinder.findRuns('EMU00019489.nxs')[0]
        self.load_result, self.run_number, self.filename = load_workspace_from_filename(self.filepath)
        self.loaded_data = MuonLoadData()
        self.data_context = MuonDataContext(self.loaded_data)
        self.gui_context = MuonGuiContext()
        self.group_pair_context = MuonGroupPairContext()
        self.gui_context.update({'RebinType': 'None'})

        self.context = MuonContext(muon_data_context=self.data_context, muon_gui_context=self.gui_context,
                                   muon_group_context=self.group_pair_context)

        self.data_context.instrument = 'EMU'

        self.loaded_data.add_data(workspace=self.load_result, run=[self.run_number], filename=self.filename,
                                  instrument='EMU')
        self.data_context.current_runs = [[self.run_number]]
        self.data_context.update_current_data()
        self.group_pair_context.reset_group_and_pairs_to_default(self.load_result['OutputWorkspace'][0]._workspace,
                                                                 'EMU', '')

    def test_reset_groups_and_pairs_to_default(self):
        self.assertEquals(self.group_pair_context.group_names, ['fwd', 'bwd'])
        self.assertEquals(self.group_pair_context.pair_names, ['long'])

    def test_calculate_group_calculates_group_for_given_run(self):
        counts_workspace, asymmetry_workspace = self.context.calculate_group('fwd', run=[19489])

        self.assertEquals(type(counts_workspace), Workspace2D)
        self.assertEquals(type(counts_workspace), Workspace2D)

    def test_calculate_pair_calculates_pair_for_given_run(self):
        pair_asymmetry = self.context.calculate_pair('long', run=[19489])

        self.assertEquals(type(pair_asymmetry), Workspace2D)

    def test_show_all_groups_calculates_and_shows_all_groups(self):
        self.context.show_all_groups()

        self.assertEquals(AnalysisDataService.getObjectNames(),
                          ['EMU19489', 'EMU19489 Groups', 'EMU19489; Group; bwd; Asymmetry; #1',
                           'EMU19489; Group; bwd; Counts; #1', 'EMU19489; Group; fwd; Asymmetry; #1',
                           'EMU19489; Group; fwd; Counts; #1', 'Muon Data'])

    def test_that_show_all_calculates_and_shows_all_groups_with_rebin(self):
        self.gui_context['RebinType'] = 'Fixed'
        self.gui_context['RebinFixed'] = 2

        self.context.show_all_groups()

        self.assertEquals(AnalysisDataService.getObjectNames(),
                          ['EMU19489', 'EMU19489 Groups', 'EMU19489; Group; bwd; Asymmetry; #1',
                           'EMU19489; Group; bwd; Asymmetry; Rebin; #1',
                           'EMU19489; Group; bwd; Counts; #1', 'EMU19489; Group; bwd; Counts; Rebin; #1',
                           'EMU19489; Group; fwd; Asymmetry; #1', 'EMU19489; Group; fwd; Asymmetry; Rebin; #1',
                           'EMU19489; Group; fwd; Counts; #1', 'EMU19489; Group; fwd; Counts; Rebin; #1', 'Muon Data'])

    def test_show_all_pairs_calculates_and_shows_all_pairs(self):
        self.context.show_all_pairs()

        self.assertEquals(AnalysisDataService.getObjectNames(),
                          ['EMU19489', 'EMU19489 Pairs', 'EMU19489; Pair Asym; long; #1', 'Muon Data'])

    def test_that_show_all_calculates_and_shows_all_pairs_with_rebin(self):
        self.gui_context['RebinType'] = 'Fixed'
        self.gui_context['RebinFixed'] = 2

        self.context.show_all_pairs()

        self.assertEquals(AnalysisDataService.getObjectNames(),
                          ['EMU19489', 'EMU19489 Pairs', 'EMU19489; Pair Asym; long; #1',
                           'EMU19489; Pair Asym; long; Rebin; #1', 'Muon Data'])

    def test_update_current_data_sets_current_run_in_data_context(self):
        self.context.update_current_data()

        self.assertEquals(self.data_context.current_data, self.load_result)

    def test_update_current_data_sets_groups_and_pairs(self):
        self.context.update_current_data()

        self.assertEquals(self.group_pair_context.pair_names, ['long'])
        self.assertEquals(self.group_pair_context.group_names, ['fwd', 'bwd'])

    def test_show_raw_data_puts_raw_data_into_the_ADS(self):
        self.context.show_raw_data()

        self.assertEquals(AnalysisDataService.getObjectNames(),
                          ['EMU19489', 'EMU19489 Raw Data', 'EMU19489_raw_data', 'Muon Data'])

    def test_that_first_good_data_returns_correctly_when_from_file_chosen_option(self):
        self.gui_context.update({'FirstGoodDataFromFile': True})

        first_good_data = self.context.first_good_data([19489])

        self.assertEquals(first_good_data, 0.11)

    def test_first_good_data_returns_correctly_when_manually_specified_used(self):
        self.gui_context.update({'FirstGoodDataFromFile': False, 'FirstGoodData': 5})

        first_good_data = self.context.first_good_data([19489])

        self.assertEquals(first_good_data, 5)

    def test_that_last_good_data_returns_correctly_when_from_file_chosen_option(self):
        self.gui_context.update({'LastGoodDataFromFile': True})

        last_good_data = self.context.last_good_data([19489])

        self.assertEquals(last_good_data, 31.76)

    def test_last_good_data_returns_correctly_when_manually_specified_used(self):
        self.gui_context.update({'LastGoodDataFromFile': False, 'LastGoodData': 5})

        last_good_data = self.context.last_good_data([19489])

        self.assertEquals(last_good_data, 5)

    def test_that_dead_time_table_from_ADS_returns_table_name(self):
        self.gui_context.update({'DeadTimeSource': 'FromADS', 'DeadTimeTable': 'deadtime_table_name'})

        deadtime_table = self.context.dead_time_table([19489])

        self.assertEquals(deadtime_table, 'deadtime_table_name')

    def test_get_pre_processing_params_returns_correctly(self):
        pre_process_params = self.context.get_pre_processing_params([19489], False)

        self.assertEqual(pre_process_params, {'TimeMin': 0.11, 'TimeOffset': 0.0})

    def test_get_grouping_counts_params_returns_correctly(self):
        grouping_counts_params = self.context.get_muon_grouping_counts_params('fwd')

        self.assertEqual(grouping_counts_params, {'GroupName': 'fwd',
                                                  'Grouping': '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,'
                                                              '23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,'
                                                              '42,43,44,45,46,47,48',
                                                  'SubtractedPeriods': '',
                                                  'SummedPeriods': '1'})

    def test_get_muon_grouping_asymmetry_params(self):
        group_asymmetry_params = self.context.get_muon_grouping_asymmetry_params('fwd', [19489])

        self.assertEqual(group_asymmetry_params, {'AsymmetryTimeMax': 31.761001586914062,
                                                  'AsymmetryTimeMin': 0.11,
                                                  'GroupName': 'fwd',
                                                  'Grouping': '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,'
                                                              '23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,'
                                                              '42,43,44,45,46,47,48',
                                                  'SubtractedPeriods': '',
                                                  'SummedPeriods': '1'})

    def test_get_muon_pairing_asymmetry_params_returns_correctly(self):
        pair_asymmetry_params = self.context.get_muon_pairing_asymmetry_params('long')

        self.assertEqual(pair_asymmetry_params, {'Alpha': '1.0',
                                                 'Group1': '1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,'
                                                           '24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,'
                                                           '44,45,46,47,48',
                                                 'Group2': '49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,'
                                                           '69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,'
                                                           '89,90,91,92,93,94,95,96',
                                                 'PairName': 'long',
                                                 'SpecifyGroupsManually': True,
                                                 'SubtractedPeriods': '',
                                                 'SummedPeriods': '1'})


if __name__ == '__main__':
    unittest.main(buffer=False, verbosity=2)
