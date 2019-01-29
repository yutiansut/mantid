from mantid.kernel import *
from mantid.api import *
from mantid.simpleapi import *
import sys
import os


def trimfname(filename):
    splitfn=filename.split('_')
    if splitfn[-2]=='for' and splitfn[-1].split('.')[-2][-2:]=='hr':
        return '_'.join(splitfn[2:-2])
    else:
        return '_'.join(splitfn[2:]).replace('.nxs', '')


def findfile(RunNum,file_structure,root_dir,reduced_dir):

    if file_structure == "Flat":
        data_dir = root_dir
        if os.path.exists(data_dir):
            for filename in os.listdir(data_dir):
                if 'VIS_'+str(RunNum)+'_' in filename:
                    fullfilename=os.path.join(data_dir, filename)
                    return fullfilename
    else:
        for ipts in os.listdir(root_dir):
            data_dir=os.path.join(root_dir,ipts,reduced_dir)
            if os.access(data_dir, os.W_OK):
                for filename in os.listdir(data_dir):
                    if 'VIS_'+str(RunNum)+'_' in filename:
                        fullfilename=os.path.join(data_dir, filename)
                        return fullfilename
    print 'Error: Cannot find reduced file for the given run number '+str(RunNum)
    sys.exit()


def findsamerun(rnstring,data_dir):
    one_num = int(rnstring.strip('-'))
    for filename in os.listdir(data_dir):
        if 'VIS_'+str(one_num) in filename:
            break
    all_num = [one_num]
    keywords = filename.replace('VIS_'+str(one_num), '')
    if rnstring[0]=='-':
        new_num = one_num
        while True:
            new_num -= 1
            if 'VIS_'+str(new_num)+keywords in os.listdir(data_dir):
                all_num.append(new_num)
            else:
                break
    if rnstring[-1]=='-':
        new_num = one_num
        while True:
            new_num += 1
            if 'VIS_'+str(new_num)+keywords in os.listdir(data_dir):
                all_num.append(new_num)
            else:
                break
    return all_num


def extract_rn(rnstring,file_structure,root_dir,reduced_dir):
    ListGrp = []
    all_grp = rnstring.split(';')
    for grp in all_grp:
        ListRN = []
        rg = grp.split(',')
        for str in rg:
            if str[0]=='-' or str[-1]=='-':
                data_dir = os.path.dirname(findfile(int(str.strip('-')),file_structure,root_dir,reduced_dir))
                all_num = findsamerun(str,data_dir)
                ListRN += all_num
            else:
                num = str.split('-')
                if len(num) ==1:
                    ListRN.append(int(num[0]))
                elif len(num) == 2:
                    if num[0]>num[1]:
                        print "Error: ", num[0], " is greater than ", num[1], ". Check input run numbers"
                        sys.exit()
                    ListRN += range(int(num[0]), int(num[1])+1)
        ListRN.sort()
        ListGrp.append(ListRN)
    return ListGrp



def merge(ListRN,weight_tag,file_structure,root_dir,raw_dir,reduced_dir,logfile):

    if weight_tag=="pchg from reduced file":
        weight = 1
    elif weight_tag=="pchg from raw file":
        weight =0
    elif weight_tag=="pchg from logbook":
        weight =2
    elif weight_tag=="estimate from error":
        weight =3
        
    if weight==0 or weight==1:
        pcharges=[]
        for RunNum in ListRN:
            fullfilename=findfile(RunNum,file_structure,root_dir,reduced_dir)
            print fullfilename
            WS=os.path.basename(fullfilename)[:-4]
            Load(Filename=fullfilename, OutputWorkspace=WS, LoaderName='LoadNexusProcessed', LoaderVersion=1, LoadHistory=False)
            if weight==0:
                h5dir = os.path.dirname(fullfilename).replace(reduced_dir,raw_dir)
                h5filename = os.path.join(h5dir,'VIS_'+str(RunNum)+'.nxs.h5')
                if not os.path.isfile(h5filename):
                    print 'Error: Cannot file raw file for proton charge'
                    sys.exit()
                cmdline='nxdir '+h5filename+' --data-mode script -p /entry/proton_charge'
                f=os.popen(cmdline)
                pchg = f.read()
                pchg=pchg.split('=')[1]
                pchg=float(pchg.split('\n')[0])
            else:
                pchg=mtd[WS].run().getProperty('gd_prtn_chrg').value
            print 'pcharge for run number ', RunNum, ':', pchg
            pcharges.append(pchg)
            Scale(InputWorkspace=WS,OutputWorkspace=WS,Factor=pchg,Operation='Multiply')
            if RunNum==ListRN[0]:
                merged='VIS_'+str(ListRN[0])+'-'+str(ListRN[-1])+'_'+trimfname(os.path.basename(fullfilename))
                CloneWorkspace(InputWorkspace=WS,OutputWorkspace=merged)
            else:
                Plus(LHSWorkspace=merged,RHSWorkspace=WS,OutputWorkspace=merged)
            DeleteWorkspace(WS)
        tpchg=1.0/sum(pcharges)
        Scale(InputWorkspace=merged,OutputWorkspace=merged,Factor=tpchg,Operation='Multiply')
		
    elif weight==2:
        f = open(logfile, 'r')
        full_text = f.read()
        f.close()
        chopped = full_text.split('\n')
        rn_list = []
        pc_list = []
        for i in range(1,len(chopped)):
            line = chopped[i].split(',')
            if len(line)>=5:
                if line[4][-1].isdigit():
                    rn_list.append(int(line[0].strip()))
                    pc_list.append(float(line[4].strip()))
                else:
                    rn_list.append(int(line[0].strip()))
                    pc_list.append(float(line[3].strip()))
        pcharges=[]
        for RunNum in ListRN:
            pchg=pc_list[rn_list.index(RunNum)]
            print 'pcharge for run number ', RunNum, ':', pchg
            pcharges.append(pchg)
            fullfilename=findfile(RunNum,file_structure,root_dir,reduced_dir)
            print fullfilename
            WS=os.path.basename(fullfilename)[:-4]
            Load(Filename=fullfilename, OutputWorkspace=WS, LoaderName='LoadNexusProcessed', LoaderVersion=1, LoadHistory=False)
            Scale(InputWorkspace=WS,OutputWorkspace=WS,Factor=pchg,Operation='Multiply')
            if RunNum==ListRN[0]:
                merged='VIS_'+str(ListRN[0])+'-'+str(ListRN[-1])+'_'+trimfname(os.path.basename(fullfilename))
                CloneWorkspace(InputWorkspace=WS,OutputWorkspace=merged)
            else:
                Plus(LHSWorkspace=merged,RHSWorkspace=WS,OutputWorkspace=merged)
            DeleteWorkspace(WS)
        tpchg=1.0/sum(pcharges)
        Scale(InputWorkspace=merged,OutputWorkspace=merged,Factor=tpchg,Operation='Multiply')
				
    elif weight==3:
        for RunNum in ListRN:
            fullfilename=findfile(RunNum,file_structure,root_dir,reduced_dir)
            print fullfilename
            WS=os.path.basename(fullfilename)[:-4]
            Load(Filename=fullfilename, OutputWorkspace=WS, LoaderName='LoadNexusProcessed', LoaderVersion=1, LoadHistory=False)
            if RunNum==ListRN[0]:
                merged='VIS_'+str(ListRN[0])+'-'+str(ListRN[-1])+'_'+trimfname(os.path.basename(fullfilename))
                CloneWorkspace(InputWorkspace=WS,OutputWorkspace=merged)
            else:
                WeightedMean(InputWorkspace1=merged,InputWorkspace2=WS,OutputWorkspace=merged)
            DeleteWorkspace(WS)
            
    return merged



class VisionLoadMultipleFiles(PythonAlgorithm):
    
    def category(self):
        return "DataHandling;PythonAlgorithms"

    def name(self):
        return "VisionLoadMultipleFiles"

    def summary(self):
        return "This algorithm loads multiple reduced nxs files and merge them."

    def PyInit(self):
        units = ["meV", "cm-1"]
        file_structure_types = ["IPTS", "Flat"]
        weight_method = ["pchg from reduced file", "pchg from raw file", "pchg from logbook", "estimate from error"]
        defaultdir = config.getDataSearchDirs()[0]
        self.declareProperty(name="Run_numbers", defaultValue="", validator=StringMandatoryValidator(), doc="Run numbers to load")
        self.declareProperty("Weighting_method", "pchg from reduced file", StringListValidator(weight_method), "How data files are weighted", direction=Direction.Input)
        self.declareProperty("File_structure", "IPTS", StringListValidator(file_structure_types), "How data files are organized", direction=Direction.Input)
        self.declareProperty(name="Root_dir", defaultValue=defaultdir, validator=StringMandatoryValidator(), doc="Root path for all data")
        self.declareProperty(name="Raw_subdir", defaultValue="nexus", validator=StringMandatoryValidator(), doc="Sub-folder for raw data, only needed for PCharge weighting")
        self.declareProperty(name="Reduced_subdir", defaultValue="shared/manualreduce", validator=StringMandatoryValidator(), doc="Sub-folder for reduced data, only needed for IPTS file structure")
        self.declareProperty(name="Logbook", defaultValue="/SNS/VIS/shared/VIS_team/VIS_logbook.csv", validator=StringMandatoryValidator(), doc="Logbook to read proton charge from")
        self.declareProperty("Rebin_data", False, "If checked, spectra will be rebinned")
        self.declareProperty(name="Rebin_param", defaultValue="-2,0.025,5,-0.005,1000", validator=StringMandatoryValidator(), doc="Rebin parameter")
        self.declareProperty("Smooth_data", False, "If checked, spectra will be smoothed")
        self.declareProperty(name="Smooth_param", defaultValue="5", validator=StringMandatoryValidator(), doc="Smooth parameter")
        self.declareProperty("Unit", "meV", StringListValidator(units), "Choose unit for OutputWorkspace", direction=Direction.Input)
        self.declareProperty("RenameOutputWorkspace", False, "If checked, output workspace will have the following name")
        self.declareProperty("Output_name", "out", validator=StringMandatoryValidator(), doc="Name of output workspace")

    def PyExec(self):
        rn_string = self.getProperty("Run_numbers").value
        weight = self.getProperty("Weighting_method").value
        file_structure = self.getProperty("File_structure").value
        root_dir = self.getProperty("Root_dir").value
        raw_dir = self.getProperty("Raw_subdir").value 
        reduced_dir = self.getProperty("Reduced_subdir").value 
        logbook = self.getProperty("Logbook").value 
        reb_flag = self.getProperty("Rebin_data").value
        reb_param = self.getProperty("Rebin_param").value
        smooth_flag = self.getProperty("Smooth_data").value
        smooth_param = self.getProperty("Smooth_param").value
        unit = self.getProperty("Unit").value
        ren_flag = self.getProperty("RenameOutputWorkspace").value
        out_name = self.getProperty("Output_name").value
        ListGrp = extract_rn(rn_string,file_structure,root_dir,reduced_dir)
        for ListRN in ListGrp:
            merged=merge(ListRN,weight,file_structure,root_dir,raw_dir,reduced_dir,logbook)
            if reb_flag:
                Rebin(InputWorkspace=merged,OutputWorkspace=merged,Params=reb_param,PreserveEvents=0)
            if smooth_flag:
                SmoothData(InputWorkspace=merged,OutputWorkspace=merged,NPoints=smooth_param)
            if unit == "cm-1":
                ConvertUnits(InputWorkspace=merged, OutputWorkspace=merged, Target='DeltaE_inWavenumber', EMode='Indirect', EFixed=3.5)
        if ren_flag:
            RenameWorkspace(InputWorkspace=merged,OutputWorkspace=out_name)
        else:
            out_name=merged
        self.declareProperty(WorkspaceProperty("OutputWorkspace", "", direction=Direction.Output))
        self.setProperty("OutputWorkspace", out_name)
        RemoveLogs(out_name)

AlgorithmFactory.subscribe(VisionLoadMultipleFiles)
