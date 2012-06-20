// Dear emacs, this is -*- c++ -*-
//______________________________________________________
//
// Plots.cxx
//
// All the output histograms of the SFrame analysis (also other interesting 
// information - can be included at a later time) are being saved into a 
// root file as genuine root objects.
//
// This program can be used to combine, sum and plot all histograms (data, 
// background, signal...) and save them to a postscript file. The program 
// can be steered with a steer file, usually called Plots.steer. 
//
// Authors    : Roman Kogler
// Created    : 2012
// Last update: 
//          by: 

#include <iostream>
#include <fstream>
#include <string>

#include <TROOT.h>
#include <TH1.h>
#include <TRint.h>
#include <TSystem.h>
#include <TStyle.h>

#include "SteerParser.h"
#include "SteerPlotter.h"
#include "RootPlotter.h"

using namespace std;

// Initialize the root framework
TROOT Plots("Plots","SFrame Plots");

int main(int argc, char** argv)
{

  // ________ parse the command line ___________
  TString filename;
  if (argc < 3) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
    cout << "Not enough arguments provided. Usage:\n"; // Inform the user of how to use the program
    cout << argv[0] << " -f <steerfile>\n";
    exit(0);
  } else { // we got enough parameters
    cout << "Executing: " << argv[0] << " ";
    for (int i = 1; i < argc; i++) { // iterate over argv[] to get the parameters
      std::cout << argv[i] << " ";
      if (i+1 != argc){ // Check that we haven't finished parsing already
	TString arg(argv[i]);
	if (arg == "-f") {
	  filename = argv[i+1];
	} else {
	  std::cout << "Not enough or invalid arguments, please try again.\n";
	  exit(0);
	}
      }
    }
    cout << endl;
  }

  TRint theApp("App",&argc,argv); 
  gROOT->Reset();

  // Stops ROOT keeping references to all histograms
  TH1::AddDirectory(kFALSE);
  // Run in batch mode by default - this supresses the canvas -> speed increase
  gROOT->SetBatch(kTRUE);   


  // ____________ process the steering ______________
  
  SteerParser parser;
  parser.ParseFile(filename);

  SteerPlotter* steerfile = (SteerPlotter*) parser.GetSteer(SteerPlotter::Class());
  //steerfile->Print();

  TString CycleName          = steerfile->GetCycleName();
  TObjArray* InputFilenames  = steerfile->GetInputFiles();
  TString PsFilename         = steerfile->GetOutputPsFile();
  Bool_t ShapeNorm           = steerfile->GetShapeNorm();
  Bool_t RatioPlot           = steerfile->GetRatioPlot();
  Bool_t PortraitMode        = steerfile->GetPortrait();
  Bool_t DrawEntries         = steerfile->GetDrawEntries();
  Bool_t PtBalanceFitOpt     = steerfile->GetFitPtBalanceHists();
  Bool_t SubstractBkgd       = steerfile->GetSubstractBkgd();
  Bool_t JetShapesPerSlice   = steerfile->GetJetShapesPerSlice();
  TObjArray* SampleNames     = steerfile->GetSampleNames();
  TObjArray* SamplesToStack  = steerfile->GetSamplesToStack(); 
  TArrayI HistColors         = steerfile->GetHistColors();
  TArrayI HistMarkers        = steerfile->GetHistMarkers();
  TArrayF SamplesWeight      = steerfile->GetSamplesWeight();


  
  // ____________ set up the plotter ______________
  
  RootPlotter* plotter = new RootPlotter();
  if (CycleName.Length()>0){
    plotter->OpenRootFiles(InputFilenames, CycleName);
  } else {
    plotter->OpenRootFiles(InputFilenames);
  }
  plotter->SetSampleNames(SampleNames);
  plotter->SetShapeNorm(ShapeNorm);
  plotter->SetPortraitMode(PortraitMode);
  plotter->SetDrawEntries(DrawEntries);
  plotter->SetHistColors(HistColors);
  plotter->SetHistMarkers(HistMarkers);
  plotter->PerformFit(PtBalanceFitOpt);
  plotter->SetSubstractBkgd(SubstractBkgd);
  plotter->SetJetShapesPerSlice(JetShapesPerSlice);
  plotter->SetSamplesToStack(SamplesToStack);
  plotter->SetSamplesWeight(SamplesWeight);
  plotter->SetPlotRatio(RatioPlot);
  
  
  // ____________ do the actual plotting ______________

  plotter->PlotHistos(PsFilename);

  // Done! Exit Root
  gSystem->Exit(0);

  theApp.Run(kTRUE);

  return 0;
}
