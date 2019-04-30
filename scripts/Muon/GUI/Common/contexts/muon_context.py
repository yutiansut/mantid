# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from Muon.GUI.Common.ADSHandler.workspace_naming import (get_raw_data_workspace_name, get_group_data_workspace_name,
                                                         get_pair_data_workspace_name, get_base_data_directory,
                                                         get_raw_data_directory, get_group_data_directory,
                                                         get_pair_data_directory, get_group_asymmetry_name)
from Muon.GUI.Common.calculate_pair_and_group import calculate_group_data, calculate_pair_data, \
    estimate_group_asymmetry_data
from Muon.GUI.Common.contexts.muon_data_context import MuonDataContext
from Muon.GUI.Common.contexts.muon_group_pair_context import MuonGroupPairContext
from Muon.GUI.Common.contexts.muon_gui_context import MuonGuiContext
from Muon.GUI.Common.contexts.phase_table_context import PhaseTableContext
from Muon.GUI.Common.utilities.run_string_utils import run_list_to_string
import Muon.GUI.Common.ADSHandler.workspace_naming as wsName
from Muon.GUI.Common.contexts.muon_data_context import get_default_grouping
import hashlib
from Muon.GUI.Common.ADSHandler.muon_workspace_wrapper import MuonWorkspaceWrapper

class MuonContext(object):
    def __init__(self, muon_data_context=MuonDataContext(), muon_gui_context=MuonGuiContext(),
                 muon_group_context=MuonGroupPairContext(), base_directory='Muon Data', muon_phase_context= PhaseTableContext()):
        self._data_context = muon_data_context
        self._gui_context = muon_gui_context
        self._group_pair_context = muon_group_context
        self.phase_context = muon_phase_context
        self.base_directory = base_directory

        self.gui_context.update({'DeadTimeSource': 'None', 'LastGoodDataFromFile': True})

    @property
    def data_context(self):
        return self._data_context

    @property
    def gui_context(self):
        return self._gui_context

    @property
    def group_pair_context(self):
        return self._group_pair_context

    def calculate_group(self, group_name, run, rebin=False):
        pre_processing_params = self.get_pre_processing_params(run, rebin)
        grouping_counts_params = self.get_muon_grouping_counts_params(group_name)
        grouping_asymmetry_params = self.get_muon_grouping_asymmetry_params(group_name, run)

        existing_counts_workspace = self.group_pair_context[group_name].get_group_counts_workspace(run, rebin)
        counts_param_string = str({'pre_processing_params': pre_processing_params, 'grouping_counts_params': grouping_counts_params})
        group_counts_hash = hashlib.sha224(counts_param_string.encode("utf8")).hexdigest()

        if existing_counts_workspace and existing_counts_workspace.params_hash == group_counts_hash:
            counts_workspace = existing_counts_workspace
        else:
            counts_workspace = MuonWorkspaceWrapper(calculate_group_data(pre_processing_params, grouping_counts_params), group_counts_hash)

        existing_asymmetry_workspace = self.group_pair_context[group_name].get_group_asymmetry_workspace(run, rebin)
        asymmetry_params_string = str({'pre_processing_params': pre_processing_params, 'grouping_asymmetry_params': grouping_asymmetry_params})
        group_asymmetry_hash = hashlib.sha224(asymmetry_params_string.encode("utf8")).hexdigest()

        if existing_asymmetry_workspace and existing_asymmetry_workspace.params_hash == group_asymmetry_hash:
            asymmetry_workspace = existing_asymmetry_workspace
        else:
            asymmetry_workspace = MuonWorkspaceWrapper(estimate_group_asymmetry_data(pre_processing_params, grouping_asymmetry_params), group_asymmetry_hash)

        return counts_workspace, asymmetry_workspace

    def calculate_pair(self, pair_name, run, rebin=False):
        pre_processing_params = self.get_pre_processing_params(run, rebin)
        pair_asymmetry_params = self.get_muon_pairing_asymmetry_params(pair_name)
        params_string = str({'pre_processing_params': pre_processing_params, 'pair_asymmetry_params': pair_asymmetry_params})
        pair_asym_params_hash = hashlib.sha224(params_string.encode("utf8")).hexdigest()
        existing_workspace = self.group_pair_context[pair_name].get_pair_workspace(run, rebin)

        if existing_workspace and pair_asym_params_hash == existing_workspace.params_hash:
            return existing_workspace
        else:
            return MuonWorkspaceWrapper(calculate_pair_data(pre_processing_params, pair_asymmetry_params), pair_asym_params_hash)

    def show_all_groups(self):
        self.calculate_all_groups()
        for run in self._data_context.current_runs:
            for group_name in self._group_pair_context.group_names:
                run_as_string = run_list_to_string(run)
                directory = get_base_data_directory(self, run_as_string) + get_group_data_directory(self, run_as_string)

                name = get_group_data_workspace_name(self, group_name, run_as_string, rebin=False)
                asym_name = get_group_asymmetry_name(self, group_name, run_as_string, rebin=False)

                self.group_pair_context[group_name].show_raw(run, directory + name, directory + asym_name)

                if self._do_rebin():
                    name = get_group_data_workspace_name(self, group_name, run_as_string, rebin=True)
                    asym_name = get_group_asymmetry_name(self, group_name, run_as_string, rebin=True)
                    self.group_pair_context[group_name].show_rebin(run, directory + name, directory + asym_name)

    def show_all_pairs(self):
        self.calculate_all_pairs()
        for run in self._data_context.current_runs:
            for pair_name in self._group_pair_context.pair_names:
                run_as_string = run_list_to_string(run)
                name = get_pair_data_workspace_name(self, pair_name, run_as_string, rebin=False)
                directory = get_base_data_directory(self, run_as_string) + get_pair_data_directory(self, run_as_string)

                self.group_pair_context[pair_name].show_raw(run, directory + name)

                if self._do_rebin():
                    name = get_pair_data_workspace_name(self, pair_name, run_as_string, rebin=True)
                    self.group_pair_context[pair_name].show_rebin(run, directory + name)

    def calculate_all_pairs(self):
        for run in self._data_context.current_runs:
            for pair_name in self._group_pair_context.pair_names:
                pair_asymmetry_workspace = self.calculate_pair(pair_name, run)
                self.group_pair_context[pair_name].update_asymmetry_workspace(pair_asymmetry_workspace, run)

                if self._do_rebin():
                    pair_asymmetry_workspace = self.calculate_pair(pair_name, run, rebin=True)
                    self.group_pair_context[pair_name].update_asymmetry_workspace(pair_asymmetry_workspace, run, rebin=True)

    def calculate_all_groups(self):
        for run in self._data_context.current_runs:
            for group_name in self._group_pair_context.group_names:
                group_workspace, group_asymmetry = self.calculate_group(group_name, run)
                self.group_pair_context[group_name].update_workspaces(run, group_workspace, group_asymmetry, rebin=False)

                if self._do_rebin():
                    group_workspace, group_asymmetry = self.calculate_group(group_name, run, rebin=True)
                    self.group_pair_context[group_name].update_workspaces(run, group_workspace, group_asymmetry, rebin=True)

    def update_current_data(self):
        # Update the current data; resetting the groups and pairs to their default values
        if len(self.data_context.current_runs) > 0:
            self.data_context.update_current_data()

            if not self.group_pair_context.groups:
                self.group_pair_context.reset_group_and_pairs_to_default(self.data_context.current_workspace,
                                                                         self.data_context.instrument,
                                                                         self.data_context.main_field_direction)
        else:
            self.data_context.clear()

    def show_raw_data(self):
        for run in self.data_context.current_runs:
            run_string = run_list_to_string(run)
            loaded_workspace = self.data_context._loaded_data.get_data(run=run, instrument=self.data_context.instrument)['workspace'][
                'OutputWorkspace']
            directory = get_base_data_directory(self, run_string) + get_raw_data_directory(self, run_string)

            if len(loaded_workspace) > 1:
                # Multi-period data
                for i, single_ws in enumerate(loaded_workspace):
                    name = directory + get_raw_data_workspace_name(self, run_string, period=str(i + 1))
                    single_ws.show(name)
            else:
                # Single period data
                name = directory + get_raw_data_workspace_name(self, run_string)
                loaded_workspace[0].show(name)

    def _do_rebin(self):
        return (self.gui_context['RebinType'] == 'Fixed' and
                'RebinFixed' in self.gui_context and self.gui_context['RebinFixed']) or\
               (self.gui_context['RebinType'] == 'Variable' and
                'RebinVariable' in self.gui_context and self.gui_context['RebinVariable'])

    def get_workspace_names_for_FFT_analysis(self, use_raw=True):
        pair_names = list(self.group_pair_context.pair_names)
        group_names = list(self.group_pair_context.group_names)
        run_numbers = self.data_context.current_runs
        workspace_options = []

        for run in run_numbers:
            workspace_options += self.phase_context.get_phase_quad(self.data_context.instrument, run_list_to_string(run))
            for name in pair_names:
                workspace_options.append(
                    wsName.get_pair_data_workspace_name(self,
                                                        str(name),
                                                        run_list_to_string(run), not use_raw))
            for group_name in group_names:
                workspace_options.append(
                    wsName.get_group_asymmetry_name(self, str(group_name), run_list_to_string(run),
                                                    not use_raw))
        return workspace_options

    def get_detectors_excluded_from_default_grouping_tables(self):
        groups, _ = get_default_grouping(
            self.data_context.current_workspace, self.data_context.instrument,
            self.data_context.main_field_direction)
        detectors_in_group = []
        for group in groups:
            detectors_in_group += group.detectors
        detectors_in_group = set(detectors_in_group)

        return [det for det in range(1, self.data_context.num_detectors) if det not in detectors_in_group]

    # Get the groups/pairs for active WS
    def getGroupedWorkspaceNames(self):
        run_numbers = self.data_context.current_runs
        runs = [
            wsName.get_raw_data_workspace_name(self, run_list_to_string(run_number), period=str(period + 1))
            for run_number in run_numbers for period in range(self.data_context.num_periods(run_number))]
        return runs

    def first_good_data(self, run):
        if not self.data_context.get_loaded_data_for_run(run):
            return 0.0

        if self.gui_context['FirstGoodDataFromFile']:
            return self.data_context.get_loaded_data_for_run(run)["FirstGoodData"]
        else:
            if 'FirstGoodData' in self.gui_context:
                return self.gui_context['FirstGoodData']
            else:
                self.gui_context['FirstGoodData'] = self.data_context.get_loaded_data_for_run(run)["FirstGoodData"]
                return self.gui_context['FirstGoodData']

    def last_good_data(self, run):
        if not self.data_context.get_loaded_data_for_run(run):
            return 0.0

        if self.gui_context['LastGoodDataFromFile']:
            return round(max(self.data_context.get_loaded_data_for_run(run)["OutputWorkspace"][0].workspace.dataX(0)), 2)
        else:
            if 'LastGoodData' in self.gui_context:
                return self.gui_context['LastGoodData']
            else:
                self.gui_context['LastGoodData'] = round(max(self.data_context.get_loaded_data_for_run(run)
                                                             ["OutputWorkspace"][0].workspace.dataX(0)), 2)
                return self.gui_context['LastGoodData']

    def dead_time_table(self, run):
        if self.gui_context['DeadTimeSource'] == 'FromADS':
            return self.gui_context['DeadTimeTable']
        elif self.gui_context['DeadTimeSource'] == 'FromFile':
            return self.data_context.get_loaded_data_for_run(run)["DataDeadTimeTable"]
        elif self.gui_context['DeadTimeSource'] == 'None':
            return None

    def get_pre_processing_params(self, run, rebin):
        pre_process_params = {}

        pre_process_params["InputWorkspace"] = self.data_context.loaded_workspace_as_group(run)

        pre_process_params["TimeMin"] = self.first_good_data(run)

        if self.gui_context['TimeZeroFromFile']:
            time_offset = 0.0
        else:
            time_offset = self.data_context.get_loaded_data_for_run(run)["TimeZero"] - self.gui_context[
                'TimeZero']
        pre_process_params["TimeOffset"] = time_offset

        dead_time_table = self.dead_time_table(run)
        if dead_time_table is not None:
            pre_process_params["DeadTimeTable"] = dead_time_table

        if rebin:
            if self.gui_context['RebinType'] == 'Variable' and self.gui_context["RebinVariable"]:
                pre_process_params["RebinArgs"] = self.gui_context["RebinVariable"]

            if self.gui_context['RebinType'] == 'Fixed' and self.gui_context["RebinFixed"]:
                x_data = \
                self.data_context._loaded_data.get_data(run=run, instrument=self.data_context.instrument
                                                           )['workspace']['OutputWorkspace'][0].workspace.dataX(0)
                original_step = x_data[1] - x_data[0]
                pre_process_params["RebinArgs"] = float(self.gui_context["RebinFixed"]) * original_step

        return pre_process_params

    def get_muon_grouping_counts_params(self, group_name):
        params = {}
        if self.data_context.is_multi_period() and 'SummedPeriods' in self.gui_context:
            summed_periods = self.gui_context["SummedPeriods"]
            params["SummedPeriods"] = summed_periods
        else:
            params["SummedPeriods"] = "1"

        if self.data_context.is_multi_period() and 'SubtractedPeriods' in self.gui_context:
            subtracted_periods = self.gui_context["SubtractedPeriods"]
            params["SubtractedPeriods"] = subtracted_periods
        else:
            params["SubtractedPeriods"] = ""

        group = self.group_pair_context[group_name]
        if group:
            params["GroupName"] = group_name
            params["Grouping"] = ",".join([str(i) for i in group.detectors])

        return params

    def get_muon_grouping_asymmetry_params(self, group_name, run):
        params = {}

        if 'GroupRangeMin' in self.gui_context:
            params['AsymmetryTimeMin'] = self.gui_context['GroupRangeMin']
        else:
            params['AsymmetryTimeMin'] = self.data_context.get_loaded_data_for_run(run)["FirstGoodData"]

        if 'GroupRangeMax' in self.gui_context:
            params['AsymmetryTimeMax'] = self.gui_context['GroupRangeMax']
        else:
            params['AsymmetryTimeMax'] = max(
                self.data_context.get_loaded_data_for_run(run)['OutputWorkspace'][0].workspace.dataX(0))

        if self.data_context.is_multi_period() and 'SummedPeriods' in self.gui_context:
            summed_periods = self.gui_context["SummedPeriods"]
            params["SummedPeriods"] = summed_periods
        else:
            params["SummedPeriods"] = "1"

        if self.data_context.is_multi_period() and 'SubtractedPeriods' in self.gui_context:
            subtracted_periods = self.gui_context["SubtractedPeriods"]
            params["SubtractedPeriods"] = subtracted_periods
        else:
            params["SubtractedPeriods"] = ""

        group = self.group_pair_context[group_name]
        if group:
            params["GroupName"] = group_name
            params["Grouping"] = ",".join([str(i) for i in group.detectors])

        return params

    def get_muon_pairing_asymmetry_params(self, pair_name):
        params = {}
        if self.data_context.is_multi_period() and 'SummedPeriods' in self.gui_context:
            summed_periods = self.gui_context["SummedPeriods"]
            params["SummedPeriods"] = summed_periods
        else:
            params["SummedPeriods"] = "1"

        if self.data_context.is_multi_period() and 'SubtractedPeriods' in self.gui_context:
            subtracted_periods = self.gui_context["SubtractedPeriods"]
            params["SubtractedPeriods"] = subtracted_periods
        else:
            params["SubtractedPeriods"] = ""

        pair = self.group_pair_context[pair_name]

        if pair:
            params["SpecifyGroupsManually"] = True
            params["PairName"] = str(pair_name)
            detectors1 = ",".join([str(i) for i in self.group_pair_context[pair.forward_group].detectors])
            detectors2 = ",".join([str(i) for i in self.group_pair_context[pair.backward_group].detectors])
            params["Group1"] = detectors1
            params["Group2"] = detectors2
            params["Alpha"] = str(pair.alpha)

        return params
