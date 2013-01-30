#ifndef SHIST_H
#define SHIST_H

#include <TH1.h>
#include <THStack.h>
#include <TH2.h>
#include <TString.h>
#include <TObject.h>

class SHist : public TObject
{

 public:
  
  SHist(TH1* hist);
  SHist(THStack* stack);
  ~SHist();

  const char* GetName() const;

  SHist* Duplicate();

  void SetProcessName(TString);
  TString GetProcessName();
  
  void SetLegName(TString);
  TString GetLegName();

  void SetDir(TString);
  TString GetDir();
  
  void SetIsUsedInStack(bool);
  bool IsUsedInStack();

  void SetIsStack(bool);
  bool IsStack();

  void SetIsYieldPlot(bool);
  bool IsYieldPlot();

  void SetWeight(double);
  double GetWeight();

  void SetDoDraw(bool);
  bool DoDraw();

  void SetDrawMarker(bool);
  bool DrawMarker();
  bool DrawLine();

  TH1* GetHist();
  THStack* GetStack();

  double GetMinimum();
  double GetMaximum(); 

  void NormaliseToArea();

  virtual void Draw(Option_t *option="");

 private:
  TH1* m_hist;
  THStack* m_stack;
  double m_weight;
  TString m_process;
  TString m_leg_name;
  TString m_dir;
  bool m_is_stack;
  bool m_is_used_in_stack;
  bool m_draw_marker;
  bool m_draw;
  bool m_is_yield_plot;

  ClassDef(SHist,0)  // SFrame histograms

};

#endif
