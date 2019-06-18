# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#pylint: disable=no-init,invalid-name
from __future__ import (absolute_import, division, print_function)
import mantid.simpleapi
import mantid.api
from mantid.kernel import Direction, IntArrayProperty, StringListValidator
import numpy
from collections import defaultdict


class MaskBTP(mantid.api.PythonAlgorithm):
    """ Class to generate grouping file
    """

    # list of supported instruments
    INSTRUMENT_LIST = ['ARCS', 'BIOSANS', 'CG2', 'CNCS', 'CORELLI', 'EQ-SANS', 'HYSPEC', 'MANDI', 'NOMAD',
                       'POWGEN', 'REF_M', 'SEQUOIA', 'SNAP', 'SXD', 'TOPAZ', 'WAND', 'WISH']

    instname = None
    instrument = None
    bankmin = defaultdict(lambda: 1, {'SEQUOIA':23, 'TOPAZ':10})  # default is one
    bankmax = {'ARCS':115, 'BIOSANS':2, 'CG2':1, 'CNCS':50, 'CORELLI':91, 'EQ-SANS':1, 'HYSPEC':20, 'MANDI':59,
               'NOMAD':99, 'POWGEN':300, 'REF_M':1, 'SEQUOIA':150,'SNAP':64,'SXD':11,'TOPAZ':59,'WAND':8,'WISH':10}

    def category(self):
        """ Mantid required
        """
        return "Transforms\\Masking;Inelastic\\Utility"

    def seeAlso(self):
        return [ "MaskDetectors","MaskInstrument" ]

    def name(self):
        """ Mantid required
        """
        return "MaskBTP"

    def summary(self):
        """ Mantid required
        """
        return "Algorithm to mask detectors in particular banks, tube, or pixels."

    def PyInit(self):
        self.declareProperty(mantid.api.WorkspaceProperty("Workspace", "",direction=Direction.InOut,
                                                          optional = mantid.api.PropertyMode.Optional), "Input workspace (optional)")
        allowedInstrumentList=StringListValidator(['']+self.INSTRUMENT_LIST)
        self.declareProperty("Instrument","",validator=allowedInstrumentList,doc="One of the following instruments: "
                             + ', '.join(self.INSTRUMENT_LIST))
        self.declareProperty(IntArrayProperty(name="Bank", values=[]),
                             doc="Bank(s) to be masked. If empty, will apply to all banks")
        self.declareProperty("Tube","",doc="Tube(s) to be masked. If empty, will apply to all tubes")
        self.declareProperty("Pixel","",doc="Pixel(s) to be masked. If empty, will apply to all pixels")
        self.declareProperty(IntArrayProperty(name="MaskedDetectors", direction=Direction.Output),
                             doc="List of  masked detectors")

    #pylint: disable=too-many-branches
    def PyExec(self):
        ws = self.getProperty("Workspace").value
        self.instrument=None
        self.instname = self.getProperty("Instrument").value

        if ws is not None:
            self.instrument = ws.getInstrument()
            self.instname = self.instrument.getName()

        if self.instname not in self.INSTRUMENT_LIST:
            raise ValueError("Instrument '"+self.instname+"' not in the allowed list")

        if self.instrument is None:
            IDF=mantid.api.ExperimentInfo.getInstrumentFilename(self.instname)
            ws=mantid.simpleapi.LoadEmptyInstrument(IDF, OutputWorkspace=self.instname+"MaskBTP")
            self.instrument=ws.getInstrument()

        # get the ranges for banks, tubes and pixels - adding the minimum back in
        banks = self.getProperty("Bank").value
        if len(banks) == 0:
            banks = numpy.arange(self.bankmin[self.instname], self.bankmax[self.instname] + 1)

        tubeString = self.getProperty("Tube").value
        if tubeString.lower() == "edges":
            tubes = numpy.array([0, -1])
        else:
            tubes = self._parseBTPlist(tubeString, self._startsFrom())

        pixelString = self.getProperty("Pixel").value
        if pixelString.lower() == "edges":
            pixels = numpy.array([0, -1])
        else:
            pixels = self._parseBTPlist(pixelString, self._startsFrom())

        # convert bank numbers into names and remove ones that couldn't be named
        banks = [self._getBankName(bank) for bank in banks]
        banks = [bank for bank in banks if bank]

        compInfo = ws.componentInfo()
        bankIndices = self._getBankIndices(compInfo, banks)

        # generate the list of detector identifiers to mask
        detlist=[]
        fullDetectorIdList = ws.detectorInfo().detectorIDs()
        for bankIndex in bankIndices:
            tubeIndices = self._getChildIndices(compInfo, bankIndex, tubes)

            for tubeIndex in tubeIndices:
                pixelIndices = self._getChildIndices(compInfo, tubeIndex, pixels)

                for pixelIndex in pixelIndices:
                    detlist.append(fullDetectorIdList[pixelIndex])

        # mask the detectors
        detlist = numpy.array(detlist)
        if detlist.size > 0:
            mantid.simpleapi.MaskDetectors(Workspace=ws, DetectorList=detlist, EnableLogging=False)
        else:
            self.log().information("no detectors within this range")

        # set the outputs
        self.setProperty("Workspace",ws.name())
        self.setProperty("MaskedDetectors", detlist)

    def _startsFrom(self):
        '''Returns what the minimum tube/pixel index for the instrument'''
        if self.instname in ['ARCS', 'BIOSANS', 'CG2', 'CNCS', 'CORELLI', 'EQ-SANS', 'HYSPEC', 'NOMAD', 'SEQUOIA', 'WAND', 'WISH']:
            return 1
        else:
            return 0

    def _getChildIndices(self, compInfo, parentIndex, filterIndices):
        indices = compInfo.children(int(parentIndex))
        if len(indices) == 1:  # go down one level
            indices = compInfo.children(int(indices[0]))
        if len(filterIndices):
            reducedFilter = filterIndices[filterIndices < len(indices)]
            if len(reducedFilter) == 0:
                raise ValueError('None of the indices ({}) are in range'.format(filterIndices))
            indices = indices[reducedFilter]

        return indices

    def _parseBTPlist(self, value, min_value):
        if len(value) == 0:
            return list()  # empty list means use everything
        else:
            # let IntArrayProperty do the work and make sure that the result is valid
            prop = IntArrayProperty(name='temp', values=value)
            validationMsg = prop.isValid
            if validationMsg:
                raise RuntimeError(validationMsg)
            result = prop.value
            if len(result) == 0:
                raise RuntimeError('Could not generate values from "{}"'.format(value))
            return result - min_value

    def _getBankName(self, banknum):
        banknum=int(banknum)
        if not (self.bankmin[self.instname] <= banknum <= self.bankmax[self.instname]):
            raise ValueError("Out of range index={} for {} instrument bank numbers".format(banknum, self.instname))

        if self.instname == 'ARCS':
            if self.bankmin[self.instname] < banknum <= 38:
                label = 'B'
                # do nothing with banknum
            elif 38 < banknum <= 77:
                label = 'M'
                banknum = banknum - 38
            elif 77 < banknum < self.bankmax[self.instname]:
                label = 'T'
                banknum = banknum - 77
            else:
                raise ValueError("Out of range index for ARCS instrument bank numbers: {}".format(banknum))
            return '{}{}'.format(label, banknum)
        elif self.instname == 'SEQUOIA':
            # there are only banks 23-26 in A row
            if self.bankmin[self.instname] <= banknum <= 37:
                label = 'A'
                # do nothing with banknum
                if banknum > 26:  # not built yet
                    return None
            elif 37 < banknum <= 74:
                label = 'B'
                banknum = banknum - 37
            elif 74 < banknum <= 113:
                label = 'C'
                banknum = banknum-74
            elif 113 <banknum <= self.bankmax[self.instname]:
                label = 'D'
                banknum = banknum-113
            else:
                raise ValueError("Out of range index for SEQUOIA instrument bank numbers: {}".format(banknum))
            return '{}{}'.format(label, banknum)
        elif self.instname == "WISH":
            return "panel" + "%02d" % banknum
        elif self.instname in ['CG2', 'EQ-SANS', 'REF_M']:
            return "detector{}".format(banknum)
        elif self.instname == 'BIOSANS':
            if banknum == 1:
                return 'detector1'
            elif banknum == 2:
                return 'wing_detector'
            else:
                raise ValueError('Out of range index for BIOSANS instrument bank numbers: {}'.format(banknum))
        else:
            return "bank" + str(banknum)

    def _getBankIndices(self, compInfo, bankNames):
        '''This removes banks that don't exist'''
        if len(bankNames) == 0:
            return list()

        bankIndices = []
        known_banks = {}  # name: index
        for bank in bankNames:
            if bank in known_banks:  # use the known index
                bankIndices.append(known_banks[bank])
                continue
            try:
                bankIndex = int(compInfo.indexOfAny(bank))
                bankIndices.append(bankIndex)

                # get all of the bank's parent's children
                parentIndex = int(compInfo.parent(bankIndex))
                for index in compInfo.children(parentIndex):
                    index = int(index)
                    known_banks[compInfo.name(index)] = index
            except ValueError:
                continue  # bank wasn't found

        self.log().information('While determining banks, filtered from {} banks to {}'.format(len(bankNames), len(bankIndices)))
        if len(bankIndices) == 0:
            raise RuntimeError('Filtered out all detectors from list of things to mask')

        return bankIndices


mantid.api.AlgorithmFactory.subscribe(MaskBTP)
