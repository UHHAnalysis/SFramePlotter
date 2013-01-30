#include <iostream>
#include <iomanip>

#include <TObjArray.h>
#include <TObjString.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TPaveText.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TLatex.h>
#include "SPlotter.h"

using namespace std;

SPlotter::SPlotter()
{
  m_can = NULL;
  m_ps  = NULL;
  m_ps_name = "default.ps";

  m_pad1 = NULL;
  m_pad2 = NULL;

  m_rp1_top = NULL;
  m_rp1  = NULL;
  m_rp2_top = NULL;
  m_rp2  = NULL;

  m_page       = 0;
  debug        = false;
  bShapeNorm   = false;
  bPortrait    = true;
  bDrawEntries = false;
  bDrawLumi    = true;
  bDrawLegend  = true;
  bPlotRatio   = false;
  need_update  = true;

}

SPlotter::~SPlotter()
{
  Cleanup();
}

void SPlotter::SetPsFilename(TString name)
{
  if (!name.EndsWith(".ps")){
    cerr << "SPlotter::SetPsFilename, given filename: " << name
	 << " does not end with .ps, intention? Please correct steering." << endl;
    exit(EXIT_FAILURE);
  }
  m_ps_name = name;
  
}

void SPlotter::DoStacking(vector<TObjArray*>& hists, TObjArray* StackNames)
{
  if (hists.size()==0){
    cerr << "SPlotter::DoStacking: Empty array of histograms. Aborting." << endl;
    exit(EXIT_FAILURE);
  }

  if (!StackNames){ // trivial case: do nothing
    return;
  }

  // loop over all histogram arrays
  int narr = hists.size();
  for (int i=narr-1; i>=0; --i){
    TObjArray* ha = hists[i];
    if (ha->GetEntries()<1) continue;    
    TString proc = ((SHist*)ha->At(0))->GetProcessName();
    if (debug) cout << "SPlotter::DoStacking, hist array = " << i 
		    << " process name = " << proc << endl;

    // loop over all stack-names
    for (int j=0; j<StackNames->GetEntries(); ++j){
      TString sname = ((TObjString*)StackNames->At(j))->GetString();
      if (debug) cout << " stack name = " << sname << endl;
      if (proc.Contains(sname)){
	if (debug) cout << " -> found match, stacking this array." << endl;
	StackHists(hists, i);
	break;
      }
    }
  }

  if (debug) cout << "SPlotter::DoStacking: Done." << endl;

  return;

}

void SPlotter::StackHists(std::vector<TObjArray*>& hists, int index)
{
  // stack histograms at position 'index' with an existing array of stacks
  // in hists
  // if the stacks don't exist, they are created and added to the array
  
  // get the stack (create a new one if it doesn't exist yet)
  TObjArray* stacks = GetStacks(hists, index);

  // add the histograms at 'index' to the stack
  for (int i=0; i<stacks->GetEntries(); ++i){
    SHist* stack = (SHist*)stacks->At(i);
    SHist* hist = (SHist*)hists[index]->At(i);
    if (!stack || !hist){
      cerr << "SPlotter::StackHists: stack or hist at position " << i 
	   << " does not exist! Abort." << endl;
      exit(EXIT_FAILURE);
    }
    // sanity check: compare names
    TString stackname = stack->GetStack()->GetName();
    TString histname = hist->GetHist()->GetName();
    if (!stackname.Contains(histname)){
      cerr << "SPlotter::StackHists: incompatible histograms at position " << i 
	   << ", stackname = " << stackname << " histname = " << histname 
	   << ". Prefer to exit because of consistency." << endl;
      exit(EXIT_FAILURE);
    }
    // still here? do the stackin'!
    hist->GetHist()->SetFillColor(hist->GetHist()->GetLineColor());
    hist->GetHist()->SetFillStyle(1001);
    stack->GetStack()->Add(hist->GetHist());
    hist->SetIsUsedInStack(true);
    hist->SetDoDraw(false);
    if (debug) cout << "stacking hist " << histname << " on " << stackname 
		    << " (dir = " << stack->GetDir() << ")" << endl;
  }  

  return;

}

TObjArray* SPlotter::GetStacks(std::vector<TObjArray*>& hists, int index)
{
  // get the array of stacks from the input hists
  // if it doesn't exist, a new array will be created if index>0
  // and the hists at position 'index' will be used as blue-print

  // try to find a stack in the array
  TObjArray* arr = NULL;
  int narr = hists.size();
  for (int i=0; i<narr; ++i){
    if (hists[i]->GetEntries()==0){
      cerr << "SPlotter::GetStacks: Got no histograms in array " << i 
	   << " unexpected behaviour - abort." << endl;
      exit(EXIT_FAILURE);
    }

    arr = hists[i];
    SHist* sh = (SHist*) arr->At(i);
    if (sh->IsStack()){
      if (debug) cout << "SPlotter::GetStacks: Found stack at position " << i << endl;
      return arr;
    }
  }

  // no stack found, create a new array with THStacks -> use position 'index'
  // in the array as a blue-print
  if (index>-1){
    if (debug) cout << "SPlotter::GetStacks: Creating new array of THStacks " 
		    << "using position " << index << " as blueprint." << endl;
    if (index>narr){
      cerr << "SPlotter::GetStacks: Can not create an array of stacks from array"
	   << " index " << index << ", since size is only " << hists.size() 
	   << ". Unexpected behaviour - abort." << endl;
      exit(EXIT_FAILURE);
    }

    arr = new TObjArray();
    for (int i=0; i<hists[index]->GetEntries();++i){      
      TString hname = ((SHist*)hists[index]->At(i))->GetHist()->GetName();
      TString name = hname + "_stack";
      THStack* st = new THStack(name, "");
      SHist* sh = new SHist(st);
      sh->SetDir(((SHist*)hists[index]->At(i))->GetDir());
      sh->SetProcessName("SM");
      sh->SetDoDraw(true);
      arr->Add(sh);
      if (debug) cout << "SPlotter::GetStacks: Adding stack with name " << name
		      << " in directory " << sh->GetDir() << " at position " << i <<  endl;
    }

    hists.push_back(arr);
    if (debug) cout << "SPlotter::GetStacks: Added stack array to collection. " 
		    << "New size = " << hists.size() << ", old = " << narr << endl;
  }

  return arr;

}

void SPlotter::CopyStyle(TH1& h1, TH1* h2)
{
  // copy the style from hist2 to hist1
  h1.SetMarkerStyle(h2->GetMarkerStyle());
  h1.SetMarkerSize(h2->GetMarkerSize());
  h1.SetMarkerColor(h2->GetMarkerColor());
  h1.SetLineWidth(h2->GetLineWidth());   
  h1.SetLineStyle(h2->GetLineStyle());
  h1.SetLineColor(h2->GetLineColor());
  h1.SetFillColor(h2->GetFillColor());
  return;
}

void SPlotter::SetupGlobalStyle()
{
  // general appearance and style

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle -> SetPadTickX(1);
  gStyle -> SetPadTickY(1);

  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadGridX(false);
  gStyle->SetPadGridY(false);
  gStyle->SetGridColor(0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);

  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(1);
  gStyle->SetFrameFillColor(0);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetFrameLineColor(1);
  gStyle->SetFrameLineStyle(1);
  gStyle->SetFrameLineWidth(1);

  gStyle->SetTitleFont(42, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");

  gStyle->SetAxisColor(1, "XYZ");
  gStyle->SetStripDecimals(kTRUE);
  gStyle->SetTickLength(0.03, "XYZ");
  gStyle->SetNdivisions(510, "XYZ");

  gStyle->UseCurrentStyle();

}

void SPlotter::Cleanup()
{
  // do what the name suggests

  ClosePostscript();
  if (m_can){
    delete m_can;
    m_can = NULL;
  }
}

void SPlotter::SetupCanvas()
{
  // set up a canvas, different possibilities 
  // to take into account portrait/landscape 
  // and ratio/no ratio plots

  Int_t CanWidth;
  Int_t CanHeight;
  if (bPortrait){
    CanWidth = 600;
    CanHeight = 830;
  } else {
    CanWidth =  800;
    CanHeight = 600;
  }

  // set up the canvas
  m_can = new TCanvas("canvas","Control Plots", CanWidth, CanHeight);

  Float_t yplot = 0.3;
  Float_t yratio = 0.17;

                                                //  coordinates:
                                                //  
  // set up the coordinates of the two pads:    //  y6 +-------------+
  Float_t y1, y2, y3, y4, y5, y6;               //     |             |
  y6 = 0.97;                                    //     |     pad1    |
  y5 = y6-yplot;                                //  y5 |-------------|
  y4 = y5-yratio;                               //     |     rp1     |
  y3 = 0.49;                                    //  y4 +-------------+
  y2 = y3-yplot;                                //  
  y1 = y2-yratio;                               //  y3 +-------------+
  Float_t x1, x2;                               //     |             |
  x1 = 0.01;                                    //     |     pad2    |
  x2 = 0.99;                                    //  y2 |-------------|
                                                //     |     rp2     |
                                                //  y1 +-------------+
                                                //     x1            x2


      
  m_rp1_top = new TPad("pad1", "Control Plots 1", x1, y5, x2, y6);
  m_rp1 = new TPad("rp1", "Ratio1", x1, y4, x2, y5);
  
  m_rp2_top = new TPad("pad2", "Control Plots 2", x1, y2, x2, y3);
  m_rp2 = new TPad("rp2", "Ratio2", x1, y1, x2, y2);
  

  m_pad1 = new TPad("pad1", "Control Plots 1", x1, y4, x2, y6);
  m_pad2 = new TPad("pad2", "Control Plots 2", x1, y1, x2, y3);
  
 
  // set margins for portrait mode
  if (bPortrait){

    m_pad1->SetTopMargin(0.05); m_pad1->SetBottomMargin(0.13);  m_pad1->SetLeftMargin(0.19); m_pad1->SetRightMargin(0.05);
    m_pad2->SetTopMargin(0.05); m_pad2->SetBottomMargin(0.13);  m_pad2->SetLeftMargin(0.19); m_pad2->SetRightMargin(0.05);

    m_rp1_top->SetTopMargin(0.02); m_rp1_top->SetBottomMargin(0.0);  m_rp1_top->SetLeftMargin(0.19); m_rp1_top->SetRightMargin(0.05);
    m_rp2_top->SetTopMargin(0.02); m_rp2_top->SetBottomMargin(0.0);  m_rp2_top->SetLeftMargin(0.19); m_rp2_top->SetRightMargin(0.05);
    m_rp1->SetTopMargin(0.0);    m_rp1->SetBottomMargin(0.35);  m_rp1->SetLeftMargin(0.19);  m_rp1->SetRightMargin(0.05);
    m_rp2->SetTopMargin(0.0);    m_rp2->SetBottomMargin(0.35);  m_rp2->SetLeftMargin(0.19);  m_rp2->SetRightMargin(0.05);    
	    
  // margins for landscape
  } else {

    m_rp1_top->SetTopMargin(0.02); m_rp1_top->SetBottomMargin(0.0);  m_rp1_top->SetLeftMargin(0.13); m_rp1_top->SetRightMargin(0.05);        
    m_rp2_top->SetTopMargin(0.02); m_rp2_top->SetBottomMargin(0.0);  m_rp2_top->SetLeftMargin(0.13); m_rp2_top->SetRightMargin(0.05);
    
    if (bPlotRatio){
      m_rp1->SetTopMargin(0.0);    m_rp1->SetBottomMargin(0.35);  m_rp1->SetLeftMargin(0.13);  m_rp1->SetRightMargin(0.05);
      m_rp2->SetTopMargin(0.0);    m_rp2->SetBottomMargin(0.35);  m_rp2->SetLeftMargin(0.13);  m_rp2->SetRightMargin(0.05);
    }
  }
  

  
  if (debug){
    m_rp1_top->SetFillColor(kYellow);
    m_rp2_top->SetFillColor(kOrange);
    if (bPlotRatio){
      m_rp1->SetFillColor(kGray);
      m_rp2->SetFillColor(kGray);
    }
  }

  m_pad1->Draw();
  m_pad2->Draw();

  m_rp1_top->Draw(); 
  m_rp2_top->Draw();
  
  if (bPlotRatio){
    m_rp1->Draw();
    m_rp2->Draw();
  }

  return;

}

void SPlotter::OpenPostscript(TString dir)
{
  // create a new ps file with the directory in the name

  TString filename(m_ps_name);
  filename.ReplaceAll(".ps","");
  filename.Append("_");
  filename.Append(dir);
  filename.Append(".ps");

  TString text(dir);
  text.Prepend("Plotting all histograms in directory ");
  
  cout << "\n+-------------------------- SFrame Plotter ---------------------------+" << endl;
  cout <<   "| " << setw(60)<< text                                    << "        |" << endl;
  cout <<   "+---------------------------------------------------------------------+" << endl;
  m_page = 0;
      
  m_ps = NULL;
  if (bPortrait){
    m_ps = new TPostScript(filename, 111);  // ps output
    m_ps->Range(20.0, 30.0);
  } else {
    m_ps = new TPostScript(filename, 112);  // ps output
    m_ps->Range(27.0, 18.0);
  }

}

void SPlotter::ClosePostscript()
{
  // close the ps file and set page number to 0
  if (m_ps){
    m_ps->Close();
    delete m_ps;
    m_ps = NULL;
  }
  m_page = 0;
}

void SPlotter::ProcessAndPlot(std::vector<TObjArray*> histarr)
{
  // loop over all arrays in the input array and plot them 

  if (histarr.size()<1){
    cerr << "SPlotter::ProcessAndPlot: No arrays of histograms given. Abort." << endl;
    exit(EXIT_FAILURE);
  }

  if (histarr[0]->GetEntries()<1){
    cerr << "SPlotter::ProcessAndPlot: No histograms given. Abort." << endl;
    exit(EXIT_FAILURE);
  }
  
  if (bPlotRatio && histarr.size()==1){
    cerr << "SPlotter::ProcessAndPlot: Only one process given, can not plot " 
	 << " ratio. Steering correct?" << endl;
    exit(EXIT_FAILURE);
  }

  SetupGlobalStyle();

  TString psname = m_ps_name;
  TString current_dir = "";

  // loop over all histograms and plot them!
  int iplot = 1;
  bool bleg = true;
  for (int i=0; i<histarr[0]->GetEntries(); ++i){

    // get the histograms for the different processes
    vector<SHist*> hists = GetPlottableHists(histarr, i);

    // no plottable hists found at position i
    if (debug) cout << "Number of plottable hists at index " << i << " = " << hists.size() << endl;
    if (hists.size()==0) continue;

    if (bShapeNorm) ShapeNormalise(hists);

    // new directory? create new ps file!
    TString dir = hists[0]->GetDir();
    if (dir.CompareTo(current_dir)!=0){
      if (iplot!=1) DrawPageNum();
      Cleanup();
      SetupCanvas();
      OpenPostscript(dir);
      current_dir = dir;
      iplot = 1;
      bleg = true;
    }

    int ipad = GetCurrentPad(iplot);
    if (debug) cout << "Plotting histograms " << hists[0]->GetName() 
		    << " iplot = " << iplot << " ipad = " << ipad << endl;

    // new page every second plot
    if (iplot%2==1){
      if (debug) cout << "Creating new page with number " << m_page << endl;
      DrawPageNum();
      if (need_update) m_can->Update();
      m_ps->NewPage();
      ++m_page;
    }

    // cosmetics
    DoCosmetics(hists);

    // ---------- do what we set out to do: plot! ----------------

    if (hists[0]->IsYieldPlot()){  // special treatment for lumi yield plot
    
      PlotLumiYield(hists[0], ipad);

    } else { // usual plots

      PlotHists(hists, ipad);
      // draw a legend     
      if (bleg){
	DrawLegend(GetHistsAtIndex(histarr, i));
	if (!bDrawLegend) bleg = false;
      }
      // draw lumi information
      if (bDrawLumi) DrawLumi();
      // draw the ratio
      if (bPlotRatio) PlotRatios(hists, ipad);
    
    }


    ++iplot;
  }

  // done!
  DrawPageNum();
  if (need_update) m_can->Update();
  Cleanup(); 
  
}

void SPlotter::PlotLumiYield(SHist* hist, int ipad)
{
  // plot the lumi yield histogram

  if (ipad==1) m_pad1->cd();
  if (ipad==2) m_pad2->cd();

  hist->Draw();
  return;

}


void SPlotter::PlotHists(vector<SHist*> hists, int ipad)
{
  // plot all histograms in the array

  if (ipad==1){
    if (bPlotRatio) m_rp1_top->cd();
    else m_pad1->cd();
  }
  if (ipad==2){
    if (bPlotRatio) m_rp2_top->cd();
    else m_pad2->cd();
  }

  bool isok = SetMinMax(hists);
  if (isok) SetLogAxes(hists);

  // first get some basic histograms
  SHist* sstack = SelStack(hists);
  SHist* sdata  = SelData(hists);

  // first, draw data if it exists
  int ndrawn = 0;
  if (sdata){
    sdata->Draw();
    ++ndrawn;
  }

  // first round
  int nh = hists.size();

  for (int i=0; i<nh; ++i){
    SHist* sh = hists[i];
    if (sh->IsStack()) continue;
    if (sh==sdata) continue;
    if (ndrawn==0) sh->Draw();
    else sh->Draw("same");
    ++ndrawn;
  }
 
  // now draw the stack
  if (sstack){
    if (ndrawn==0){
      sstack->Draw();
      need_update = false;    
    } else {
      sstack->Draw("same");
    }
  }
 
  // second round
  for (int i=0; i<nh; ++i){
    SHist* sh = hists[i];
    if (sh->IsStack()) continue;
    if (sh==sdata) continue;
    sh->Draw("same");
  }

  // draw data on top
  if (sdata) sdata->Draw("same");

  gPad->RedrawAxis();
  
}

void SPlotter::PlotRatios(vector<SHist*> hists, int ipad)
{
  // plot all histograms in the array

  if (ipad==1) m_rp1->cd();
  if (ipad==2) m_rp2->cd();

  // calculate ratios
  vector<SHist*> ratios = CalcRatios(hists);

  gPad->SetLogx(0);
  gPad->SetLogy(0);

  int ndrawn = 0;
  int nh = ratios.size();
  for (int i=0; i<nh; ++i){
    SHist* rh = ratios[i];
    TString name = rh->GetName();
    if (name.Contains("_lx")) gPad->SetLogx(1);
    if (ndrawn==0) rh->Draw();
    else rh->Draw("same");
    ++ndrawn;
  }
 
  gPad->RedrawAxis();
  
}

vector<SHist*> SPlotter::CalcRatios(vector<SHist*> hists)
{
  // build ratios from the array 'hists'
  // by default it is checked if a data histogram exists,
  // which is then divided by the stack
  // steerable: which histograms should be calculated for the ratio

  // first get the basic histograms
  SHist* sstack = SelStack(hists);
  SHist* sdata  = SelData(hists);
  
  vector<SHist*> ratios;

  // TODO: ratio if neither stack nor data exist
  if (!sstack || !sdata){    
    return ratios;
  }
  
  SHist* rd = (SHist*) sdata->Duplicate();
  TH1D*  rdhist = (TH1D*) rd->GetHist();

  // get the denominator: the last element in the stack is the sum of all
  TObjArray* arr = sstack->GetStack()->GetStack();
  TH1D* denom = (TH1D*) arr->At(arr->GetEntries()-1);

  rdhist->Divide(denom);
  rdhist->GetYaxis()->SetTitle(rd->GetProcessName() + " / MC");
  RatioCosmetics(rdhist);

  // one histogram for the MC statistical error
  SHist* mcerr = new SHist(rdhist);
  mcerr->GetHist()->SetName("MCstat");
  mcerr->SetProcessName("MCstat");
  TH1D* MCstat = (TH1D*)mcerr->GetHist();

  for (Int_t ibin=1;ibin<denom->GetNbinsX()+1; ++ibin){
    Double_t val = denom->GetBinContent(ibin);
    Double_t err = denom->GetBinError(ibin);
    MCstat->SetBinContent(ibin,  1.0);
    MCstat->SetBinError(ibin,  err/val);
  }
  MCstat->SetMarkerStyle(0);
  MCstat->SetMarkerSize(0);
  MCstat->SetLineColor(kGray);
  MCstat->SetFillColor(kGray);

  ratios.push_back(mcerr);
  ratios.push_back(rd);	  
 
  return ratios;

}

void SPlotter::DrawLegend(vector<SHist*> hists)
{
  // draw a legend  

  int narr = hists.size();
  float yfrac = 0.06;
  if (!bPlotRatio) yfrac = 0.05;
  float top = 0.92;
  if (!bPlotRatio && bDrawLumi) top = 0.86;
  float ysize = yfrac*narr;
  float xleft = 0.7;
  float xright = 0.92;
  if (!bPortrait){
    top = 0.99;
    ysize = 0.07*narr;
    xleft = 0.72;
    xright = 0.96;
  }
	
  TLegend *leg = new TLegend(xleft,top-ysize,xright,top, NULL,"brNDC");
  leg->SetFillColor(0);
  leg->SetLineColor(1);
  leg->SetBorderSize(0);
  leg->SetTextFont(42);
  leg->SetFillStyle(0);

  for (Int_t i=0; i<narr; ++i){
    SHist* sh = hists[i];
    if (sh->IsStack()) continue;

    TString legname = TString::Format("leg_entry_%i",i);
    TString legtitle = sh->GetLegName();
    TLegendEntry* entry = NULL;
    int marker = sh->GetHist()->GetMarkerStyle();
    int lstyle = sh->GetHist()->GetLineStyle();

    if (marker>0){
      entry = leg->AddEntry(legname, legtitle, "lp");
      entry->SetLineWidth(1);
      entry->SetLineColor(sh->GetHist()->GetLineColor());
      entry->SetMarkerColor(sh->GetHist()->GetLineColor());
      entry->SetMarkerStyle(marker);
      entry->SetMarkerSize(1.0);
    } else {
      
      if (sh->IsUsedInStack()){
	entry = leg->AddEntry(legname, legtitle, "f");
	entry->SetLineWidth(1);
	entry->SetLineColor(sh->GetHist()->GetLineColor());
	entry->SetFillColor(sh->GetHist()->GetLineColor());
	entry->SetFillStyle(1001);

      } else {
	entry = leg->AddEntry(legname, legtitle, "l");
	entry->SetLineColor(sh->GetHist()->GetLineColor());
	entry->SetMarkerStyle(0);
	entry->SetMarkerSize(0);
	entry->SetMarkerColor(sh->GetHist()->GetLineColor());
	entry->SetLineWidth(2);
	entry->SetLineStyle(lstyle);
      
      }
      entry->SetTextAlign(12);
      //entry->SetTextColor(fSampleColors.At(i));
    }
  }
  leg->Draw();
  
}


void SPlotter::DrawLumi()
{
  TString infotext = TString::Format("CMS Preliminary, %3.1f fb^{-1} at #sqrt{s} = 8 TeV", m_lumi);
  TLatex *text1 = new TLatex(3.5, 24, infotext);
  text1->SetNDC();
  text1->SetTextAlign(13);
  text1->SetX(0.22);
  text1->SetTextFont(42);
  if (bPlotRatio){ 
    text1->SetTextSize(0.06);
    text1->SetY(0.94);
  } else {
    text1->SetTextSize(0.05);
    text1->SetY(0.92);
  }
  text1->Draw();
  
}

void SPlotter::DoCosmetics(vector<SHist*> hists)
{

  // loop over all histograms and make them pretty
  int nh = hists.size();
  for (int i=0; i<nh; ++i){
    SHist* sh = hists[i];
    if (sh->IsStack()) continue;
    GeneralCosmetics(sh->GetHist());
    if (bPortrait) PortraitCosmetics(sh->GetHist());
    if (!bPortrait) LandscapeCosmetics(sh->GetHist());
    if (sh->IsYieldPlot()) YieldCosmetics(sh->GetHist());
  }

}

SHist* SPlotter::SelStack(vector<SHist*> hists)
{
  // select the stack histogram from the array
  int narr = hists.size();
  SHist* h = NULL;
  for (int i=0; i<narr; ++i){
    if (hists[i]->IsStack()){
      h=hists[i];
      break;
    }
  }
  return h;
}

SHist* SPlotter::SelData(vector<SHist*> hists)
{
  // select the data histogram from the array
  int narr = hists.size();
  SHist* h = NULL;
  TString process;
  for (int i=0; i<narr; ++i){
    process = hists[i]->GetProcessName();
    if (process.Contains("data", TString::kIgnoreCase)){
      h = hists[i];
      break;
    }
  }
  return h;
}

bool SPlotter::SetMinMax(vector<SHist*> hists)
{
  // set minimum and maximum of all histograms
  int narr = hists.size();
  TString name = hists[0]->GetName();
  double max = 0;
  double min = FLT_MAX;
  for (int i=0; i<narr; ++i){
    if (max<hists[i]->GetMaximum()) max = hists[i]->GetMaximum();
    if (min>hists[i]->GetMinimum()) min = hists[i]->GetMinimum();
  }

  bool isok = true;
  if (max<1e-6){
    isok = false;
    return isok;
  }

  bool islog = false;
  double uscale = 1.2;
  if (name.Contains("_lxy") || name.Contains("_ly")){
    islog = true;
    uscale = 12.;
  }

  for (int i=0; i<narr; ++i){
    SHist* h = hists[i];
    if (h->IsStack()){ 
      if (!islog) h->GetStack()->SetMinimum(0.001);
      h->GetStack()->SetMaximum(uscale*max);
    } else {
      if (!islog) h->GetHist()->SetMinimum(0.001);
      h->GetHist()->SetMaximum(uscale*max);
    }
  }  

  return isok;
}

void SPlotter::SetLogAxes(vector<SHist*> hists)
{
  // set log axes 
  TString name = hists[0]->GetName();
  gPad->SetLogx(0);
  gPad->SetLogy(0);
  if (name.Contains("_lxy")){
    gPad->SetLogx(1);
    gPad->SetLogy(1);
  } else if (name.Contains("_lx")){
    gPad->SetLogx(1);
  } else if (name.Contains("_ly")){
    gPad->SetLogy(1);
  } else {
    // do nothing, all fine
  }
  return;
}

int SPlotter::GetCurrentPad(int np)
{
  // get the current pad, depending on the number of 
  // already processed plots
  int ipad = 1;
  int rest = np%2;
  if (rest==0) ipad=2;      
  return ipad;
}

vector<SHist*> SPlotter::GetHistsAtIndex(std::vector<TObjArray*> histarr, int index)
{
  // fill an array with histograms at position index. 

  vector<SHist*> hists;
  int narr = histarr.size();
  for (int i=0; i<narr; ++i){
    SHist* hist = (SHist*)histarr[i]->At(index);
    hists.push_back(hist);
  }
  
  return hists;

}

vector<SHist*> SPlotter::GetPlottableHists(std::vector<TObjArray*> histarr, int index)
{
  // fill an array with plottable histograms at position index. 
  // if the first histogram in the array should be plotted (DoPlot flag), 
  // then take at first position in the array
  // otherwise look for the stack and plot it first
  // only then all other histograms are added

  if (debug) cout << "\nSPlotter: Collecting plottable hists for index " << index << endl;
  vector<SHist*> hists;
  bool gotstack = false;
  SHist* hist = (SHist*)histarr[0]->At(index);

  // check if the histogram is a 2D or 3D histogram, 
  // plotting not supported yet, to come
  if (hist->GetHist()->InheritsFrom(TH2::Class())){
    if (debug) cout << "Hist inherits from TH2, return without adding any to the array " << endl;
    return hists;
  }

  TString name = hist->GetName();
  TString process = hist->GetProcessName();
  if (process.Contains("data",TString::kIgnoreCase) 
      && name.Contains("_perlumibin", TString::kIgnoreCase)){
    hist->SetIsYieldPlot(true);
    hists.push_back(hist);
    return hists;
  }

  if (hist->DoDraw()){ // take first hist
    hists.push_back(hist);
    gotstack = false;
    if (debug) cout << "Adding hist " << hist->GetHist()->GetName()
		    << " from process " << hist->GetProcessName() 
		    << " and directory " << hist->GetDir() << " to array." << endl;
  } else { // try if stack exists
    TObjArray* stacks = GetStacks(histarr);
    if (stacks){
      hist = (SHist*)stacks->At(index);
      hists.push_back(hist);
      gotstack = true;
      if (debug) cout << "Adding stack " << hist->GetStack()->GetName()
		      << " from process " << hist->GetProcessName() 
		      << " and directory " << hist->GetDir() << " to array." << endl;
    }
  }
  
  // loop over the rest and add them to the array
  int narr = histarr.size();
  for (int i=1; i<narr; ++i){

    SHist* hist = (SHist*)histarr[i]->At(index);

    if (hist->DoDraw()){ // take it if it should be drawn

      if (hist->IsStack()){
	if (!gotstack){ // take the stack only if not already added
	  hists.push_back(hist);
	  if (debug) cout << "Adding stack " << hist->GetStack()->GetName()
			  << " from process " << hist->GetProcessName() 
			  << " and directory " << hist->GetDir() << " to array." << endl;
	}
      } else { // take the histogram if it's not the stack hist
	hists.push_back(hist);
	if (debug) cout << "Adding hist " << hist->GetHist()->GetName()
			<< " from process " << hist->GetProcessName() 
			<< " and directory " << hist->GetDir() << " to array." << endl;
      }
    }
  }

  if (debug) cout << "SPlotter: Done with collecting plottable hists for index " 
		  << index << ", got " << hists.size() << " histograms" << endl;
  
  return hists;

}

void SPlotter::DrawPageNum()
{
  
  m_can->cd();
  TPaveText* text; 
  TString s;
  s.Form("%i",m_page);
  if (bPortrait){
    text = new TPaveText(0.93, 0.00, 0.97, 0.03, "NDC");
  } else {
    text = new TPaveText(0.03,0.00, 0.06, 0.03, "NDC");
  }
  text->SetBorderSize(0);
  text->SetFillColor(0);
  text->AddText(s.Data());
  text->Draw("same");
  
}

void SPlotter::GeneralCosmetics(TH1* hist)
{
  // set Y-axis title
  hist->GetYaxis()->SetTitle("Entries");  
  
  // set X-axis title
  hist->GetXaxis()->SetTitle(hist->GetTitle()); 

  hist->SetTitle("");

  if (bShapeNorm) {
    hist->GetYaxis()->SetTitle("#DeltaN/N");
  }

  hist->GetXaxis()->SetTitleFont(42);
  hist->GetXaxis()->SetLabelFont(42);
  hist->GetYaxis()->SetTitleFont(42);
  hist->GetYaxis()->SetLabelFont(42);

}

void SPlotter::PortraitCosmetics(TH1* hist)
{

  // top histogram of the ratio plot
  if (bPlotRatio){
    
    // x-axis
    hist->GetXaxis()->SetTickLength(0.05);

    // y-axis
    hist->GetYaxis()->SetTitleSize(0.07);
    hist->GetYaxis()->SetLabelSize(0.062);
    hist->GetYaxis()->SetLabelOffset(0.01);   
    hist->GetYaxis()->SetTitleOffset(0.8);
    hist->GetYaxis()->SetTickLength(0.02);
  
  // only this histogram
  } else {

    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetXaxis()->SetLabelOffset(0.008);
    hist->GetXaxis()->SetTickLength(0.03);
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetTitleOffset(1.2);
    
    hist->GetYaxis()->SetTitleOffset(1.2);
    hist->GetYaxis()->SetTitleSize(0.06);
    hist->GetYaxis()->SetLabelSize(0.045);
    hist->GetYaxis()->SetTickLength(0.02);
    hist->GetYaxis()->SetLabelOffset(0.011);

  }
  
}

void SPlotter::YieldCosmetics(TH1* hist)
{
  // cosmetics for the lumi yield histogram
    hist->GetXaxis()->SetLabelSize(0.05);
    hist->GetXaxis()->SetLabelOffset(0.008);
    hist->GetXaxis()->SetTickLength(0.03);
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetTitleOffset(1.2);
    
    hist->GetYaxis()->SetTitleOffset(1.2);
    hist->GetYaxis()->SetTitleSize(0.06);
    hist->GetYaxis()->SetLabelSize(0.045);
    hist->GetYaxis()->SetTickLength(0.02);
    hist->GetYaxis()->SetLabelOffset(0.011);

    hist->GetXaxis()->SetTitle("time (constant luminosity)");
    hist->GetYaxis()->SetTitle("events per luminosity");
  
}

void SPlotter::LandscapeCosmetics(TH1* hist)
{



}

void SPlotter::RatioCosmetics(TH1* hist)
{

  hist->GetYaxis()->SetRangeUser(0.3, 1.7);
  hist->SetMarkerSize(0.7);

  // cosmetics for portrait mode 
  if (bPortrait){
    hist->SetTitle("");
    
    // x-axis
    hist->GetXaxis()->SetLabelSize(0.12);
    hist->GetXaxis()->SetTickLength(0.08);
    hist->GetXaxis()->SetTitleSize(0.12);
    hist->GetXaxis()->SetTitleOffset(1.25);
	  
    // y-axis
    hist->GetYaxis()->CenterTitle();
    hist->GetYaxis()->SetTitleSize(0.12);
    hist->GetYaxis()->SetTitleOffset(0.46);
    hist->GetYaxis()->SetLabelSize(0.11);
    hist->GetYaxis()->SetNdivisions(210);
    hist->GetYaxis()->SetTickLength(0.02);
    hist->GetYaxis()->SetLabelOffset(0.011);

    // cosmetics for landscape mode 
  } else {
    
    hist->SetTitle("");
    hist->SetTitleOffset(1.1, "X");
    hist->SetTitleOffset(0.5, "Y");
    hist->SetLabelOffset(0.02, "X");
    hist->SetLabelOffset(0.01, "Y");
	  
    hist->GetXaxis()->SetLabelSize(0.14);
    hist->GetXaxis()->SetTickLength(0.07);
    hist->GetXaxis()->SetTitleSize(0.15);
	  
    hist->GetYaxis()->CenterTitle();
    hist->GetYaxis()->SetTitleSize(0.11);
    hist->GetYaxis()->SetLabelSize(0.12);
    hist->GetYaxis()->SetNdivisions(505);
    hist->GetYaxis()->SetTickLength(0.03);
    
  }

}

void SPlotter::ShapeNormalise(std::vector<SHist*> hists)
{
  for (unsigned int i=0; i<hists.size(); ++i){
    hists[i]->NormaliseToArea();
  }
}

