.. _DetectorInfo:

============
DetectorInfo
============

.. contents::
  :local:

Introduction
------------
:py:obj:`~mantid.geometry.DetectorInfo` provides faster and simpler access to instrument/beamline detector geometry and metadata as required by Mantid :ref:`Algorithms <Algorithm>` than was possible using :ref:`Instrument`. :py:obj:`~mantid.geometry.DetectorInfo` and :py:obj:`~mantid.geometry.ComponentInfo` are designed as full replacements to :ref:`Instrument`.

:ref:`Instrument Access Layers <InstrumentAccessLayers>` provides details on how :py:obj:`~mantid.geometry.DetectorInfo` interacts with other geometry access layers.

Python Interface
----------------

Example of using :py:obj:`~mantid.geometry.DetectorInfo` in python

**Mask detectors at some distance from the source**


.. testcode:: mask_detectors

   from mantid.simpleapi import CreateSampleWorkspace

   # Test workspace with instrument
   ws = CreateSampleWorkspace()
   det_info = ws.detectorInfo();
   mask_count = 0
   for item in det_info:
       if not item.isMonitor and item.l2 > 2.0:
           item.setMasked(True)
           mask_count += 1
   print('masked {} detectors'.format(mask_count))

Output:

.. testoutput:: mask_detectors

   masked 200 detectors

.. categories:: Concepts
