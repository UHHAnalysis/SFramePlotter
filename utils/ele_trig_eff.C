TGraphAsymmErrors* get_eff(TString sample, TString obs, TString hname, double &tot_eff, double &tot_err);
void draw_eff(TGraphAsymmErrors* eff, TString filename, TString sample, double tot_eff=1., double tot_err=1.);
TGraphAsymmErrors* get_ratio(TGraphAsymmErrors* top, TGraphAsymmErrors* bottom);

void ele_trig_eff()
{
  gStyle->SetOptFit(1111);

  //TString obs = "Jets";
  //TString hist = "pt_jet1_lx";
  //TString hist = "pt_jet2_lx";

  TString snames[] = {"Data", "TTbar", "Zprime_1TeV", "Zprime_2TeV", "Zprime_3TeV", "Zprime_4TeV" };
  //TString prefix = "corrected_";
  TString prefix = "";
  
  for (int i=0; i<6; ++i){
    TString sample = snames[i];

    TString obs = "Electron";
    double tot_eff, tot_err;
    TString hist = "pT";
    TString filename = prefix + sample + "_" + obs + "_" + hist + ".eps";
    TGraphAsymmErrors* eff = get_eff(sample, obs, hist, tot_eff, tot_err);
    draw_eff(eff, filename, sample, tot_eff, tot_err);

    hist = "eta";
    filename = prefix + sample + "_" + obs + "_" + hist + ".eps";
    eff = get_eff(sample, obs, hist, tot_eff, tot_err);
    draw_eff(eff, filename, sample, tot_eff, tot_err);

    hist = "isolation";
    filename = prefix + sample + "_" + obs + "_" + hist + ".eps";
    eff = get_eff(sample, obs, hist, tot_eff, tot_err);
    draw_eff(eff, filename, sample, tot_eff, tot_err);
  }
  
  TString obs = "Electron";
  TString hist = "pT";
  double tot_eff, tot_err;
  TGraphAsymmErrors* deff = get_eff("Data", obs, hist, tot_eff, tot_err);
  TGraphAsymmErrors* mceff = get_eff("TTbar", obs, hist, tot_eff, tot_err);
  TGraphAsymmErrors* ratio = get_ratio(deff, mceff);
  draw_ratio(ratio, prefix + "ratio_electron_pT.eps", "Data / MC");

  TString hist = "eta";
  TGraphAsymmErrors* deff = get_eff("Data", obs, hist, tot_eff, tot_err);
  TGraphAsymmErrors* mceff = get_eff("TTbar", obs, hist, tot_eff, tot_err);
  TGraphAsymmErrors* ratio = get_ratio(deff, mceff);
  draw_ratio(ratio, prefix + "ratio_electron_eta.eps", "Data / MC");

  TString hist = "isolation";
  TGraphAsymmErrors* deff = get_eff("Data", obs, hist, tot_eff, tot_err);
  TGraphAsymmErrors* mceff = get_eff("TTbar", obs, hist, tot_eff, tot_err);
  TGraphAsymmErrors* ratio = get_ratio(deff, mceff);
  draw_ratio(ratio, prefix + "ratio_electron_isolation.eps", "Data / MC");
  
  

}


TGraphAsymmErrors* get_eff(TString sample, TString obs, TString hname, double &tot_eff, double &tot_err)
{

  TFile* file;
  TFile* file2;
  TFile* file3;
  if (sample.CompareTo("data", TString::kIgnoreCase) == 0){
    file = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.DATA.DATA.root", "READ");
  } else if (sample.CompareTo("ttbar", TString::kIgnoreCase) == 0){
    file = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.TTbar_0to700.root", "READ");
    file2 = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.TTbar_700to1000.root", "READ");
    file3 = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.TTbar_1000toInf.root", "READ");
  } else if (sample.Contains("zprime", TString::kIgnoreCase)){
    if (sample.Contains("1tev", TString::kIgnoreCase)){
      file  = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP1000w10.root", "READ");
      file2 = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP1000w100.root", "READ");
    } else if (sample.Contains("2tev", TString::kIgnoreCase)){
      file  = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP2000w20.root", "READ");
      file2 = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP2000w200.root", "READ");
    } else if (sample.Contains("3tev", TString::kIgnoreCase)){
      file  = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP3000w30.root", "READ");
      file2 = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP3000w300.root", "READ");
    } else if (sample.Contains("4tev", TString::kIgnoreCase)){
      file  = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP4000w40.root", "READ");
      file2 = new TFile("/scratch/hh/dust/naf/cms/user/kogler/Zprime/EleTrigStudy/ZprimeEleTrigCycle.MC.ZP4000w400.root", "READ");
    }
  }
	
  TString effdir = "_Electrontrig/";  
  //TString effdir = "_ElectrontrigWithWeight/";
					
  TString name = obs + "_Electronsel/" + hname; 
  TH1F* d_all = (TH1F*) file->Get(name);
  name = obs + effdir + hname; 
  TH1F* d_trig = (TH1F*) file->Get(name);

  if (sample.CompareTo("ttbar", TString::kIgnoreCase) == 0){
    TString name = obs + "_Electronsel/" + hname; 
    TH1F* h2 = (TH1F*) file2->Get(name);
    TH1F* h3 = (TH1F*) file3->Get(name);
    d_all->Add(h2);
    d_all->Add(h3);

    name = obs + effdir + hname; 
    h2 = (TH1F*) file2->Get(name);
    h3 = (TH1F*) file3->Get(name);
    d_trig->Add(h2);
    d_trig->Add(h3);
  }
  
  if (sample.CompareTo("zprime", TString::kIgnoreCase) == 0){
    TString name = obs + "_Electronsel/" + hname; 
    TH1F* h2 = (TH1F*) file2->Get(name);
    d_all->Add(h2);

    name = obs + effdir + hname; 
    h2 = (TH1F*) file2->Get(name);
    d_trig->Add(h2);
  }  

  // calculate total efficiency
  Double_t bx[] = {d_all->GetXaxis()->GetXmin(), d_all->GetXaxis()->GetXmax()};    
  TH1F* tot_all = (TH1F*)d_all->Rebin(1, "tot_all", bx);
  TH1F* tot_trig = (TH1F*)d_trig->Rebin(1, "tot_trig", bx);
  // add underflow and overflow bins
  tot_all->AddBinContent(1, d_all->GetBinContent(0)+d_all->GetBinContent(d_all->GetNbinsX()+1));
  tot_trig->AddBinContent(1, d_trig->GetBinContent(0)+d_trig->GetBinContent(d_trig->GetNbinsX()+1));


  TGraphAsymmErrors* toteff = new TGraphAsymmErrors( tot_trig, tot_all, "cl=0.683 b(1,1) mode" );
  Double_t x, y;
  toteff->GetPoint(0, x, y);
  cout << "\n------------------------------ efficiency calculation -----------------------------------" << endl;
  cout << "sample = " << sample << " observable = " << obs << " histogram = " << hname << endl;
  cout << "considering all bins, together with underflow and overflow! " << endl;
  cout << "content of underflow bin for all events: " << d_all->GetBinContent(0) << ", triggered = " << d_trig->GetBinContent(0) << endl;
  cout << "content of overflow bin for all events: " << d_all->GetBinContent(d_all->GetNbinsX()+1) << ", triggered = " << d_trig->GetBinContent(d_trig->GetNbinsX()+1) << endl;
  cout << "total eff = " << y << " + " << toteff->GetErrorYhigh(0) << " - " << toteff->GetErrorYlow(0) << endl << endl;
  tot_eff = y;
  tot_err = (toteff->GetErrorYhigh(0)+toteff->GetErrorYlow(0))/2.;
  
  // rebin if pt
  if (hname.Contains("pT")){
    Double_t binsx[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 160, 170, 180, 190, 200, 220, 240, 260, 280, 300, 350, 400, 500};
    d_all = (TH1F*)d_all->Rebin(43, "all_rebinned", binsx);
    d_trig = (TH1F*)d_trig->Rebin(43, "trig_rebinned", binsx);
  }

  // rebin if reliso
  if (hname.Contains("isolation")){
    //cout << "bins: ";
    //for (int i=1; i<d_all->GetNbinsX(); ++i){
    //  cout << d_all->GetXaxis()->GetBinLowEdge(i) << ", ";
    //}
    //cout << endl;
    Double_t binsx[] = {0, 0.005, 0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05, 0.055, 0.06, 0.065, 0.07, 0.075, 0.08, 0.09, 0.1, 0.11, 0.12, 0.14, 0.16, 0.18, 0.2, 0.24, 0.28, 0.35, 0.5};
    d_all = (TH1F*)d_all->Rebin(28, "all_rebinned", binsx);
    d_trig = (TH1F*)d_trig->Rebin(28, "trig_rebinned", binsx);
  }
  

  TGraphAsymmErrors* eff = new TGraphAsymmErrors( d_trig, d_all, "cl=0.683 b(1,1) mode" );
  if (sample.CompareTo("data", TString::kIgnoreCase) == 0){
    eff->SetMarkerStyle(20);
    eff->SetMarkerColor(kBlack);
    eff->SetLineColor(kBlack);
  }
  if (sample.CompareTo("ttbar", TString::kIgnoreCase) == 0){
    eff->SetMarkerStyle(22);
    eff->SetMarkerColor(kRed+1);
    eff->SetLineColor(kRed+1);
  }
  if (sample.Contains("zprime", TString::kIgnoreCase)){
    eff->SetMarkerStyle(23);
    eff->SetMarkerColor(kBlue+1);
    eff->SetLineColor(kBlue+1);
  }
  
  return eff;

}

void draw_eff(TGraphAsymmErrors* eff, TString epsfilename, TString sample, double tot_eff, double tot_err)
{

  eff->Draw("AP");
  double x,y;

  TH1F* painter = eff->GetHistogram();
  TString xtitle = painter->GetTitle();
  if (xtitle.Contains("reliso", TString::kIgnoreCase)){
    xtitle = "electron relative isolation";
  }
  painter->GetXaxis()->SetTitle(painter->GetTitle());
  painter->GetYaxis()->SetTitle("Electron trigger efficiency");
  painter->SetTitle(sample);
  painter->GetXaxis()->SetTitleSize(0.05);
  painter->GetYaxis()->SetTitleSize(0.05);
  painter->GetXaxis()->SetLabelSize(0.04);
  painter->GetYaxis()->SetLabelSize(0.04);

  painter->GetYaxis()->SetRangeUser(0, 1.1);

  gPad->SetTopMargin(0.08);
  gPad->SetRightMargin(0.05);
  gPad->SetBottomMargin(0.15);
  gPad->SetLeftMargin(0.15);

  TLine* l = new TLine(painter->GetXaxis()->GetXmin(), 1., painter->GetXaxis()->GetXmax(), 1.);
  l->SetLineColor(kBlue);
  l->Draw();

  eff->Draw("Psame");

  TString info = TString::Format("< #varepsilon > = %4.3f #pm %4.3f", tot_eff, tot_err);
  TLatex* text = new TLatex();
  text->SetTextFont(62);
  text->SetNDC();
  text->SetTextColor(eff->GetLineColor());
  text->SetTextSize(0.04);
  text->DrawLatex(0.67, 0.2, info.Data());

  gPad->SaveAs(epsfilename);

}

void draw_ratio(TGraphAsymmErrors* ratio, TString epsfilename, TString rname)
{

  ratio->Draw("AP");
  double x,y;

  TH1F* painter = ratio->GetHistogram();
  painter->GetXaxis()->SetTitle(painter->GetTitle());
  painter->GetYaxis()->SetTitle(rname);
  painter->GetYaxis()->CenterTitle(true);
  painter->SetTitle("");
  painter->GetXaxis()->SetTitleSize(0.05);
  painter->GetYaxis()->SetTitleSize(0.05);
  painter->GetXaxis()->SetLabelSize(0.04);
  painter->GetYaxis()->SetLabelSize(0.04);

  painter->GetYaxis()->SetRangeUser(0.6, 1.4);

  gPad->SetTopMargin(0.08);
  gPad->SetRightMargin(0.05);
  gPad->SetBottomMargin(0.15);
  gPad->SetLeftMargin(0.15);

  TLine* l = new TLine(painter->GetXaxis()->GetXmin(), 1., painter->GetXaxis()->GetXmax(), 1.);
  l->SetLineColor(kBlue);
  l->SetLineStyle(kDotted);
  l->Draw();

  ratio->Draw("Psame");

  TF1* func;
  if (epsfilename.Contains("isolation", TString::kIgnoreCase)){
    func = new TF1("Ratio", "[0]+[1]*x", painter->GetXaxis()->GetXmin(),  painter->GetXaxis()->GetXmax());
    func->SetParName(0, "p0");
    func->SetParName(0, "p1");
  } else {
    func = new TF1("Ratio", "[0]", painter->GetXaxis()->GetXmin(),  painter->GetXaxis()->GetXmax());
    func->SetParName(0, "Ratio");
  }
  ratio->Fit(func);

  gPad->SaveAs(epsfilename);

}

TGraphAsymmErrors* get_ratio(TGraphAsymmErrors* top, TGraphAsymmErrors* bottom)
{
  TGraphAsymmErrors* ratio = new TGraphAsymmErrors(*top);

  for (int i=0; i<top->GetN(); ++i){
    double xt,yt,xb,yb;
    top->GetPoint(i,xt,yt);
    bottom->GetPoint(i,xb,yb);
    double r = yt / yb;
    ratio->SetPoint(i,xt,r);
    
    // up error
    double ut = top->GetErrorYhigh(i)/yb;
    double ub = (bottom->GetErrorYlow(i)*yt)/(yb*yb);
    double up = TMath::Sqrt(ut*ut+ub*ub);

    ratio->SetPointEYhigh(i, up*r);

    // down error
    double dt = top->GetErrorYlow(i)/yb;
    double db = (bottom->GetErrorYhigh(i)*yt)/(yb*yb);
    double down = TMath::Sqrt(dt*dt+db*db);
    ratio->SetPointEYlow(i, down*r);
  }

  return ratio;

}
