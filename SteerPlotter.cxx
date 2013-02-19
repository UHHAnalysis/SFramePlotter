#include "SteerPlotter.h"

#include <iostream>
#include <iomanip>
#include <TObjString.h>

using namespace std;

ClassImp(SteerPlotter)

SteerPlotter::SteerPlotter()
{
  // Here you have to set the default values which will be used,
  // if this steering class is required but not given in the steering file
   
   fNumOfSamples = 0;  
   fOutputPsFile = "Plots/test.ps";
   bSubstractBkgd = false;
   bDrawEntries = false;
   bDrawLumi = false;
   bDrawLegend = true;
   fNumOfSamplesToStack = 0;
   fLumi = 0;
   fSysError = -1.;
}

SteerPlotter::~SteerPlotter()
{
   fLegStrings.Delete();
}

void SteerPlotter::Print(Option_t* opt) const
{
  // Prints all settings of the steering  

  // First: perform some sanity checks
  if (fNumOfSamples != fInputFiles.GetEntries()){
    cout << "Error: Number of input files is not the same as the number of samples to be plotted." << endl;
    exit(3);
  }
  if (fNumOfSamples != fSamplesWeight.GetSize()){
    cout << "Error: Number of given weights is not the same as number of samples." << endl;
    exit(3);
  }
  if (fNumOfSamples != fHistColors.GetSize()){
    cout << "Error: Number of colours given is not the same as number of samples." << endl;
    exit(3);
  }
  if (fNumOfSamples != fHistMarkers.GetSize()){
    cout << "Error: Number of markers given is not the same as number of samples." << endl;
    exit(3);
  }
  if (fNumOfSamples < fSamplesToStack.GetEntries() ){
    cout << "Error: Number of samples is smaller than the number of samples to stack." << endl;
    exit(3);
  }


  cout << endl;
  cout << "-------------------------------------------------------- SteerPlotter " << opt << "-----------------------------------------------" << endl;

  cout << "Number of analysis samples to be plotted: " << fNumOfSamples << endl;
  if (fCycleName.Length()>0){
    cout << "Cylcle Name (used as prefix for root files): " << fCycleName << endl;
  }
  for (Int_t i=0; i<fNumOfSamples; ++i){
    TString name(((TObjString*) fSampleNames.At(i))->GetName() );
    cout << "File of sample " << i << ":  " << setw(25) << ((TObjString*)fInputFiles.At(i))->GetName()
	 << "   name =  " << setw(15) << name
    	 << "   color = " << setw(4) << fHistColors.At(i) 
	 << "   marker = " << setw(4) << fHistMarkers.At(i) 
	 << "   with weight " << fSamplesWeight.At(i) << endl;
  }
  cout << "Output Ps File:                " << fOutputPsFile << endl;
  cout << endl;
  if (fNumOfSamplesToStack>0){
      cout << "These samples will be stacked:" << endl;
      for (Int_t i=0; i<fNumOfSamplesToStack; ++i){
          TString name(((TObjString*) fSamplesToStack.At(i))->GetName() );
          cout << "    Name of sample " << i << " in stack :      " << setw(15) << name << endl;
      }
      
  } else {
      cout << "No stacking will be plotted." << endl;
  }
  if (bSubstractBkgd){
    cout << "Background will be substracted from sample 0: " << (((TObjString*) fSampleNames.At(0))->GetName() ) << endl;
  } else {
    cout << "No background substraction" << endl;
  }
  cout << (bRatioPlot? "Ratios will be plotted." : "No ratios will be plotted") << endl;
  cout << (bDrawEntries? "Number of histogram entries will be plotted." : "Number of histogram entries will not be plotted") << endl;
  cout << (bDrawLumi? "Lumi inforamtion will be plotted." : "Lumi inforamtion will not be plotted") << endl;
  cout << "Integrated luminosity = " << fLumi << " fb-1" << endl;
  if (fSysError>0){
    cout << "Normalisation error of " << fSysError*100 << "% will be drawn." << endl;
  } else {
    cout << "No normalisation error will be drawn." << endl;
  }
  cout << (bDrawLegend? "Legend will be plotted everywhere." : "Legend will be plotted on first plot only") << endl;
  cout << (bShapeNorm? "Shape normalization" : "No shape normalization") << endl;
  cout << (bLumiNorm? "Luminosity normalization" : "No lumi normalization") << endl;
  cout << (bPortrait?  "Setting the page to portrait mode" : "Setting the page to landscape mode") << endl;
  cout << "--------------------------------------------------------------------------------------------------------------------" << endl;
  
}

void SteerPlotter::SetShapeNorm(Bool_t flag){bShapeNorm = flag;}
Bool_t SteerPlotter::GetShapeNorm(){return bShapeNorm;}

void SteerPlotter::SetLumiNorm(Bool_t flag){bLumiNorm = flag;}
Bool_t SteerPlotter::GetLumiNorm(){return bLumiNorm;}

void SteerPlotter::SetRatioPlot(Bool_t flag){bRatioPlot = flag;}
Bool_t SteerPlotter::GetRatioPlot(){return bRatioPlot;}

void SteerPlotter::SetPortrait(Bool_t flag){bPortrait = flag;}
Bool_t SteerPlotter::GetPortrait(){return bPortrait;}

void SteerPlotter::SetFitPtBalanceHists(Bool_t flag){bFitPtBalanceHists = flag;}
Bool_t SteerPlotter::GetFitPtBalanceHists(){return bFitPtBalanceHists;}

void SteerPlotter::SetDrawEntries(Bool_t flag){bDrawEntries = flag;}
Bool_t SteerPlotter::GetDrawEntries(){return bDrawEntries;}

void SteerPlotter::SetDrawLumi(Bool_t flag){bDrawLumi = flag;}
Bool_t SteerPlotter::GetDrawLumi(){return bDrawLumi;}

void SteerPlotter::SetDrawLegend(Bool_t flag){bDrawLegend = flag;}
Bool_t SteerPlotter::GetDrawLegend(){return bDrawLegend;}

void SteerPlotter::SetJetShapesPerSlice(Bool_t flag){bJetShapesPerSlice = flag;}
Bool_t SteerPlotter::GetJetShapesPerSlice(){return bJetShapesPerSlice;}

void SteerPlotter::SetLumi(Float_t lumi){fLumi = lumi;}
Float_t SteerPlotter::GetLumi(){return fLumi;}

void SteerPlotter::SetSysError(Float_t err){fSysError = err;}
Float_t SteerPlotter::GetSysError(){return fSysError;}

void SteerPlotter::SetSampleNames(const char* in) {
    this->SplitString(in,",",&fSampleNames);
    fNumOfSamples = fSampleNames.GetEntries();
}
TObjArray* SteerPlotter::GetSampleNames(){return &fSampleNames;}

void SteerPlotter::SetInputFiles(const char* in){ this->SplitString(in,",",&fInputFiles);}
TObjArray* SteerPlotter::GetInputFiles() {return &fInputFiles;}

void SteerPlotter::SetOutputPsFile(const char* in) {fOutputPsFile = in;}
const char* SteerPlotter::GetOutputPsFile() {return fOutputPsFile.Data();}

void SteerPlotter::SetCycleName(const char* in) {fCycleName = in;}
const char* SteerPlotter::GetCycleName() {return fCycleName.Data();}

void SteerPlotter::SetLegStrings(const char* in){this->SplitString(in,",",&fLegStrings);}
TObjArray* SteerPlotter::GetLegStrings() {return &fLegStrings;}

void SteerPlotter::SetHistColors(const char* in){this->StringToArray(in, fHistColors);}
TArrayI SteerPlotter::GetHistColors(){return fHistColors;}

void SteerPlotter::SetHistMarkers(const char* in){this->StringToArray(in, fHistMarkers);}
TArrayI SteerPlotter::GetHistMarkers(){return fHistMarkers;}

void SteerPlotter::SetSamplesToStack(const char* in){ this->SplitString(in,",",&fSamplesToStack); fNumOfSamplesToStack = fSamplesToStack.GetEntries();}
TObjArray* SteerPlotter::GetSamplesToStack(){return &fSamplesToStack;}

void SteerPlotter::SetSamplesWeight(const char* in){ this->StringToArray(in, fSamplesWeight);}
TArrayF SteerPlotter::GetSamplesWeight(){return fSamplesWeight;}

void SteerPlotter::SetSubstractBkgd(Bool_t flag){ bSubstractBkgd = flag; }
Bool_t SteerPlotter::GetSubstractBkgd(){ return bSubstractBkgd; }
