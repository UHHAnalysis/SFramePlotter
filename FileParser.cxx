#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <TSystem.h>
#include <TObjString.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TClass.h>

#include "FileParser.h"
#include "SHist.h"

using namespace std;

FileParser::FileParser()
{
  m_file = NULL;
  m_hists = NULL;
  debug = false;
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
    gSystem->Exec(command);
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
	TH1* rebinned = Rebin(thist, dirname);
	SHist* shist = NULL;
	if (rebinned){
	  shist = new SHist(rebinned);
	} else {
	  shist = new SHist(thist);
	}
	shist->SetProcessName(m_process);
	shist->SetDir(dirname);
	if (debug) cout << "Adding hist " << shist->GetHist()->GetName() 
			<< " (process = " << m_process << ")" << endl;
	m_hists->Add(shist);	
      }
      
      delete obj;

    }

  }
  
  return;

}

TH1* FileParser::Rebin(TH1* hist, TString dirname)
{						
  TString name(hist->GetName());
  TString title(hist->GetTitle());
  if (name.CompareTo("pT")==0 && dirname.Contains("Electron") && title.Contains("electron")){
    //Double_t binsx[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 160, 170, 180, 190, 200, 220, 240, 260, 280, 300, 350, 400, 500};
    Double_t binsx[] = {30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 160, 170, 180, 190, 200, 220, 240, 260, 280, 300, 350, 400, 500};
    name.Append("_rebin_lx");
    TH1* rebinned = hist->Rebin(37, name, binsx);
    rebinned->SetTitle("electron P_{T} [GeV]");
    return rebinned;
  } else {
    return NULL;
  }

}

void FileParser::SetInfo(TString legname, double weight, int colour, int marker)
{
  
  for (int i=0; i<m_hists->GetEntries(); ++i){
    SHist* sh = (SHist*)m_hists->At(i);
    sh->SetLegName(legname);
    sh->SetWeight(weight);
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
