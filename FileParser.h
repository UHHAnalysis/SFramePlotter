#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <TH1.h>
#include <THStack.h>
#include <TH2.h>
#include <TString.h>
#include <TFile.h>
#include <TObjArray.h>

class FileParser
{

 public:
  
  FileParser();
  ~FileParser();

  void OpenFile(TString fname, TString cyclename);
  void CloseFile();
  void BrowseFile();
  
  bool FileExists(TString name);

  void SetInfo(TString legname, double weight, int colour, int marker);

  TObjArray* GetHists(){return m_hists;}
  void SetDebug(bool flag=true){debug = flag;}

 private:

  TObjArray* FindSubdirs();
  void StoreProcessName(TString name);

  TFile* m_file;
  TString m_process;
  bool debug;
  TObjArray* m_hists;
  

};

#endif
