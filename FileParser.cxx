#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <TSystem.h>
#include <TObjString.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TClass.h>
#include <TMath.h>

#include "FileParser.h"
#include "SHist.h"

using namespace std;

FileParser::FileParser()
{
  m_file = NULL;
  m_hists = NULL;
  m_shapeSys = NULL;
  debug = false;
  m_do_cumulative = true;
}

FileParser::~FileParser()
{
  if (m_file){
    CloseFile();
  }
}

void FileParser::CloseFile()
{
  if (m_file){
    m_file->Close();
    delete m_file;
    m_file = NULL;
  }
}


void FileParser::Clear()
{
  // clear all arrays, but do not close file
  if (m_hists) m_hists->Clear();
  if (m_shapeSys) m_shapeSys->Clear();
}

bool FileParser::FileExists(TString filename)
{
  struct stat buf;
  if (stat(filename.Data(), &buf) != -1)
    {
      return true;
    }
  return false;
}


void FileParser::OpenFile(TString fname, TString cyclename)
{
  // open the root files with names given in the TObjArray

  if (m_file != NULL){
    cerr << "FileParser::OpenFile: Can not open new file, since file " 
	 << m_file->GetName() << " is still in memory. Abort." << endl;
    exit(EXIT_FAILURE);
  }

  if (cyclename.Sizeof()!=0){
    TString Prefix(cyclename);
    Prefix.Append(".");
    fname.Prepend(Prefix);
  }  

  // check if name consists of a wildcard, if so use hadd to combine histograms
  if (fname.Contains("*")){
    TString target(fname);
    target.ReplaceAll("*","");

    // check if target exists, delete if yes
    if (FileExists(target)){
      if (debug) cout << "Target exists, removing file " << target << endl;
      remove(target);
    }
      
    TString command = "hadd " + target + " " + fname;
    int res = gSystem->Exec(command);
    if(res != 0){
        cerr << "hadd command '" << command << "' failed with error code " << res << ", aborting." << endl;
        exit(EXIT_FAILURE);
    }
    fname = target;
  }

  // check if name consists of a wildcard, if so use hadd to combine histograms
  if (fname.Contains("?")){
    TString target(fname);
    target.ReplaceAll("?","");

    // check if target exists, delete if yes
    if (FileExists(target)){
      if (debug) cout << "Target exists, removing file " << target << endl;
      remove(target);
    }
      
    TString command = "hadd " + target + " " + fname;
    int res = gSystem->Exec(command);
    if(res != 0){
        cerr << "hadd command '" << command << "' failed with error code " << res << ", aborting." << endl;
        exit(EXIT_FAILURE);
    }
    fname = target;
  }

  if (debug) cout << "Opening file with name " << fname << "..." << endl;
  m_file = new TFile(fname, "READ");
  if (debug){
    cout << "... success! pointer = " << m_file << endl;
    cout << "name = " << m_file << endl;
    cout << " is open? " << m_file->IsOpen() << endl;
    m_file->ls();
  }
    
  if (!m_file->IsOpen()) {
    cout << endl << "FileParser: File " << fname << " does not exist!!!" << endl;
    exit(EXIT_FAILURE);
  } else { // success!
    cout << "FileParser: Successfully opened file " << fname << endl;
  }

  StoreProcessName(fname);

  // create a new TObjArray to store all histograms
  m_hists = new TObjArray();

  return;
}


void FileParser::OpenThetaFile(TString cyclename)
{
  // open the root files with names given in the TObjArray

  if (m_file != NULL){
    cerr << "FileParser::OpenFile: Can not open new file, since file " 
	 << m_file->GetName() << " is still in memory. Abort." << endl;
    exit(EXIT_FAILURE);
  }
  TString fname = "" ;
  if (cyclename.Sizeof()!=0){
    fname = cyclename;
      }  

  // fname = target;


  if (debug) cout << "Opening file with name " << fname << "..." << endl;
  m_file = new TFile(fname, "READ");
  if (debug){
    cout << "... success! pointer = " << m_file << endl;
    cout << "name = " << m_file << endl;
    cout << " is open? " << m_file->IsOpen() << endl;
    m_file->ls();
  }
    
  if (!m_file->IsOpen()) {
    cout << endl << "FileParser: File " << fname << " does not exist!!!" << endl;
    exit(EXIT_FAILURE);
  } else { // success!
    cout << "FileParser: Successfully opened file " << fname << endl;
  }

  StoreProcessName(fname);

  // create a new TObjArray to store all histograms
  m_hists = new TObjArray();
  m_shapeSys = new TObjArray();
  return;
}


void FileParser::StoreProcessName(TString name)
{
  
  TObjArray* pieces = name.Tokenize(".");
  for (int i=0; i<pieces->GetEntries(); ++i){
    TString piece = ((TObjString*)pieces->At(i))->GetString();
    if (piece.CompareTo("root")==0){
      m_process = ((TObjString*)pieces->At(i-1))->GetString();
      if (debug) cout << "Process in file = " << m_process << endl;
    }
  }
}


TObjArray* FileParser::FindSubdirs()
{
  // find all subdirectories (former histogram collections) in the open file
  // returns a TObjArray with the names of the subdirectories 

  m_file->cd();
  TObjArray* dirnames = new TObjArray();
  TString dirname(""); // empty directory, to stay in home dir first
  dirnames->Add(new TObjString(dirname));

  TKey *key;
  TIter nextkey( gDirectory->GetListOfKeys() );
  while ( (key = (TKey*)nextkey())) {
    TObject *obj = key->ReadObj();
    if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {    // found a subdirectory! 
      TString dirname(((TDirectory*) obj)->GetName());
      dirnames->Add(new TObjString(dirname));
      if (debug) cout << "Found directory " << dirname << endl;
    }
  }
  return dirnames;

}

void FileParser::BrowseFile()
{

  if (!m_file){
    cerr << "FileParser::BrowseFile: No file open. Abort." << endl;
    exit(EXIT_FAILURE);
  }
  TObjArray* dirs = FindSubdirs();

  // loop over all directories and get the histograms
  for (Int_t i=0; i<dirs->GetEntries(); ++i){

    TString dirname = ((TObjString*)dirs->At(i))->GetString();
    if (debug) cout << "Getting all histograms from directory " << dirname << endl;

    m_file->cd();
    gDirectory->Cd(dirname);

    // loop over all histograms in the directory and store them
    TKey *key;
    TIter nextkey( gDirectory->GetListOfKeys() );
    while ( (key = (TKey*)nextkey())) {

      TObject *obj = key->ReadObj();

      if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {

	// histogram found
	TH1* thist = (TH1*) obj;
	if (m_do_cumulative) MakeCumulativeHist(thist);
	TH1* rebinned = Rebin(thist, dirname);
	SHist* shist = NULL;
	if (rebinned){
	  shist = new SHist(rebinned);
	} else {
	  shist = new SHist(thist);
	}
	shist->SetProcessName(m_process);
	if (dirname==""){
	  shist->SetDir("Main");
	} else {
	  shist->SetDir(dirname);
	}
	if (debug) cout << "Adding hist " << shist->GetHist()->GetName() 
			<< " (process = " << m_process << ")" << endl;
	m_hists->Add(shist);	
      }
      
      delete obj;

    }

  }
  
  return;

}


void FileParser::BrowseThetaFile(TString sample)
{

  if (!m_file){
    cerr << "FileParser::BrowseFile: No file open. Abort." << endl;
    exit(EXIT_FAILURE);
  }
   
  m_file->cd();
  TKey *key;
  TIter nextkey( m_file->GetListOfKeys() );

  while ( (key = (TKey*)nextkey())) {
    
    TString histName = key->GetName();
    histName.ReplaceAll("__", "#");
    TObjArray* pieces = histName.Tokenize("#");

    if (((TObjString*)pieces->At(1))->GetString() == sample){

      TObject *obj = key->ReadObj();
      
      if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {

	// histogram found
	TH1* thist = (TH1*) obj;
	if (m_do_cumulative) MakeCumulativeHist(thist);
	TH1* rebinned = Rebin(thist, "");
	SHist* shist = NULL;
	if (rebinned){
	  shist = new SHist(rebinned);
	}
	else {
	  shist = new SHist(thist);
	}
	TString proc_name = ((TObjString*)pieces->At(1))->GetString();
	shist->SetProcessName(proc_name);
	SetProcessName(proc_name);
	TString hname = ((TObjString*)pieces->At(0))->GetString();
	shist->SetName(hname);
	  
	shist->SetDir("Main");
	
	if (pieces->GetEntries()>2){
	  m_shapeSys->Add(shist);
	  proc_name = ((TObjString*)pieces->At(1))->GetString() + "__" + ((TObjString*)pieces->At(2))->GetString() + "__" + ((TObjString*)pieces->At(3))->GetString();
	  shist->SetProcessName(proc_name);	  
	  SetProcessName(proc_name);
	  if (debug) cout << "Adding hist to systematic sample: " << shist->GetHist()->GetName() 
	  		  << " (process = " << m_process << ")" << endl;
	} else {
	  m_hists->Add(shist);
	  if (debug) cout << "Adding hist " << shist->GetHist()->GetName() 
			  << " (process = " << m_process << ")" << endl;
	}
      }
      
      delete obj;
    }
  }

  
  return;

}

void FileParser::MakeCumulativeHist(TH1* hist)
{
  for (Int_t i=1; i<hist->GetNbinsX()+1; ++i){
    Double_t sum = 0;
    Double_t sumw2 = 0;
    for (int j=i; j<hist->GetNbinsX()+1; ++j){
      sum += hist->GetBinContent(j);
      sumw2 += hist->GetSumw2()->At(j);
    }
    hist->SetBinContent(i, sum);
    hist->SetBinError(i, TMath::Sqrt(sumw2));
  }

}


TH1* FileParser::Rebin(TH1* hist, TString dirname)
{						
  TString name(hist->GetName());
  TString title(hist->GetTitle());
  //cout << "name = " << name << " title = " << title << endl;
  if (name.CompareTo("toptags")==0){// && dirname.Contains("cutflow6") && title.Contains("electron")){
   
    Double_t binsx[] = {0, 960, 1020, 1080, 1140, 1200, 1260, 1320, 1380, 1440, 1500, 1560, 1620, 1680, 1740, 1800, 1860, 1920, 1980, 2040, 2100, 2400, 3000};
    name.Append("_rebin_lx");
    TH1* rebinned = hist->Rebin(22, name, binsx);
    rebinned->SetTitle("HT [GeV]");
    return rebinned;

  } else if (name.BeginsWith("mu_0top0btag_mttbar")) {
    
    TH1* rebinned = hist->Rebin(2);
    rebinned->GetXaxis()->SetRangeUser(0,3500);
    return rebinned;

  } else if (name.BeginsWith("mu_0top1btag_mttbar")) {
    
    TH1* rebinned = hist->Rebin(2);
    rebinned->GetXaxis()->SetRangeUser(0,3500);
    return rebinned;

  } else if (name.BeginsWith("mu_1top_mttbar")) {
    
    TH1* rebinned = hist->Rebin(4);
    rebinned->GetXaxis()->SetRangeUser(0,3500);
    return rebinned;

  } else if (name.BeginsWith("el_0top0btag_mttbar")) {
    
    TH1* rebinned = hist->Rebin(2);
    rebinned->GetXaxis()->SetRangeUser(0,3500);
    return rebinned;

  } else if (name.BeginsWith("el_0top1btag_mttbar")) {
    
    TH1* rebinned = hist->Rebin(2);
    rebinned->GetXaxis()->SetRangeUser(0,3500);
    return rebinned;

  } else if (name.BeginsWith("el_1top_mttbar")) {
    
    TH1* rebinned = hist->Rebin(4);
    rebinned->GetXaxis()->SetRangeUser(0,3500);
    return rebinned;



  } else {
    return NULL;
  }

}

void FileParser::SetInfo(TString legname, double weight, int colour, int marker, float unc)
{
  
  for (int i=0; i<m_hists->GetEntries(); ++i){
    SHist* sh = (SHist*)m_hists->At(i);
    sh->SetLegName(legname);
    sh->SetWeight(weight);
    sh->SetUnc(unc);
    if (weight>0) sh->GetHist()->Scale(weight);
    sh->GetHist()->SetMarkerColor(colour);
    sh->GetHist()->SetLineColor(colour);

    if (marker > 1 ){
      sh->SetDrawMarker(true);
      sh->GetHist()->SetMarkerStyle(marker);
    } else {
      sh->SetDrawMarker(false);
      sh->GetHist()->SetMarkerStyle(0);
      sh->GetHist()->SetMarkerSize(0);
      sh->GetHist()->SetLineWidth(2);   
    }

    // histogram is transparent if marker < 0  
    if (marker < 0 ){
      // change line style
      if (marker==-1) sh->GetHist()->SetLineStyle(kDashed);
      if (marker==-2) sh->GetHist()->SetLineStyle(kDotted);
      if (marker==-3) sh->GetHist()->SetLineStyle(kDashDotted);
      if (marker==-4) sh->GetHist()->SetLineStyle(kDashDotted);    
    }
  }
}
