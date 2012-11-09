#ifndef SPLOTTER_H
#define SPLOTTER_H

#include <cstdlib>
#include <TH1.h>
#include <THStack.h>
#include <TH2.h>
#include <TString.h>
#include <TObject.h>
#include <TCanvas.h>
#include <TPostScript.h>
#include "SHist.h"

class SPlotter
{

 public:
  
  SPlotter();
  ~SPlotter();

  // main functions
  void ProcessAndPlot(std::vector<TObjArray*> hists);
  std::vector<SHist*> GetPlottableHists(std::vector<TObjArray*> histarr, int index);
  std::vector<SHist*> GetHistsAtIndex(std::vector<TObjArray*> histarr, int index);
  void PlotHists(std::vector<SHist*> hists, int ipad);
  void PlotRatios(std::vector<SHist*> hists, int ipad);

  // collect all histograms
  void DoStacking(std::vector<TObjArray*>& hists, TObjArray* StackNames);
  TObjArray* GetStacks(std::vector<TObjArray*>& hists, int index=-1);

  // utilities
  void Cleanup();
  void SetupGlobalStyle();
  void SetupCanvas();
  void OpenPostscript(TString dir);
  void ClosePostscript();
  int GetCurrentPad(int);
  void DrawPageNum();
  std::vector<SHist*> CalcRatios(std::vector<SHist*> hists);
  void DrawLegend(std::vector<SHist*> hists);
  void DrawLumi();
  
  // cosmetics
  void DoCosmetics(std::vector<SHist*> hists);
  void GeneralCosmetics(TH1* hist);
  void PortraitCosmetics(TH1* hist);
  void LandscapeCosmetics(TH1* hist);
  void RatioCosmetics(TH1* hist);
  void CopyStyle(TH1& h1, TH1* h2);
  bool SetMinMax(std::vector<SHist*> hists);
  void SetLogAxes(std::vector<SHist*> hists);
  
  // select a few histograms
  SHist* SelStack(std::vector<SHist*> hists);
  SHist* SelData(std::vector<SHist*> hists);

  // setters
  void SetDebug(bool flag=true){debug=flag;}
  void SetShapeNorm(Bool_t flag = true){bShapeNorm = flag;}
  void SetPortraitMode(Bool_t flag = true){bPortrait = flag;}
  void SetDrawEntries(Bool_t flag = true){bDrawEntries = flag;}
  void SetPlotRatio(Bool_t flag=true){bPlotRatio = flag;}
  void SetDrawLumi(Bool_t flag=true){bDrawLumi = flag;}
  void SetDrawLegend(Bool_t flag=true){bDrawLegend = flag;}
  void SetPsFilename(TString name);

 private:

  // do the stacking
  void StackHists(std::vector<TObjArray*>& hists, int index);

  TCanvas* m_can; 
  TPostScript* m_ps; 
  TString m_ps_name;

  TPad* m_pad1;
  TPad* m_rp1;
  TPad* m_pad2;
  TPad* m_rp2;
  
  int   m_page;             // page number in ps file
  bool  debug;              // output of debugging information
  bool  bShapeNorm;         // use shape normalization
  bool  bPortrait;          // portrait or landscape mode
  bool  bDrawEntries;       // display the number of entries 
  bool  bDrawLumi;          // display the lumi information 
  bool  bDrawLegend;        // display legend?
  bool  bPlotRatio;         // should a ratio be plotted?
  bool  need_update;        // should the canvas get an update?
  

};

#endif
