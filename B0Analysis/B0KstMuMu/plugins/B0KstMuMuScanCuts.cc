// ##########################################################################
// # Program to tune the selection cuts for the B0 --> K*0 mu+ mu- analysis #
// ##########################################################################
// # Author: Mauro Dinardo                                                  #
// ##########################################################################

#include <TApplication.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include "Utils.h"
#include "B0KstMuMuTreeContent.h"

using std::cout;
using std::flush;
using std::setprecision;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;


// ####################
// # Global constants #
// ####################
#define ParameterFILE "../python/ParameterFile.txt"
#define DoTrigCheck 1
#define SpecialHighq2Bin2 7.3 // [GeV/c2]2
#define nEvPrint 200000
#define SETBATCH true // Set batch mode


// ####################
// # Global variables #
// ####################
Utils* Utility;
vector<double> q2Bins;
vector<double> cosThetaKBins;
vector<double> cosThetaLBins;
vector<double> phiBins;


// #######################
// # Function Definition #
// #######################
void PrintCurrentTime ();
void CutOptimization  (unsigned int scanType, unsigned int q2Region, string MCFile, string DataFile);


// ###########################
// # Function Implementation #
// ###########################
void PrintCurrentTime ()
{
  time_t rawT;
  struct tm* localT;

  time(&rawT);
  localT = localtime(&rawT);
  cout << "The current date/time is: " << asctime(localT) << endl;
}


void CutOptimization (unsigned int scanType, unsigned int q2Region, string MCFile, string DataFile)
// #####################################
// # scanType = 0 = "CL"               #
// # scanType = 1 = "L/sigma"          #
// # scanType = 2 = "cos(alpha)"       #
// # scanType = 3 = "had pT"           #
// # scanType = 4 = "mass(K*)"         #
// # scanType = 5 = "hadDCA/sigma"     #
// #####################################
// # q2Region = 0 = q2 bin 0,1         #
// # q2Region = 1 = q2 bin 7           #
// # q2Region = 2 = q2 bin 0,1,7       #
// # q2Region = 3 = q2 bin 0,1,special 2 (high edge q2 bin 2 = SpecialHighq2Bin2),7 #
// # q2Region = 4 = q2 bin 2,4,6       #
// # q2Region = 5 = q2 bin all but 3,5 #
// #####################################
{
  stringstream myString;
  double timeEnd;
  double timeStart;
  unsigned int countCands;
  bool B0notB0bar;
  int BestCandIndx;
  int TrigCat;


  // ##############
  // # Parameters #
  // ##############
  double signalSigma = sqrt(atof(Utility->GetGenericParam("FRACMASSS").c_str()) * atof(Utility->GetGenericParam("SIGMAS1").c_str()) * atof(Utility->GetGenericParam("SIGMAS1").c_str()) +
			    (1. - atof(Utility->GetGenericParam("FRACMASSS").c_str())) * atof(Utility->GetGenericParam("SIGMAS2").c_str()) * atof(Utility->GetGenericParam("SIGMAS2").c_str()));
  double MCtoDataRescale = 20.466442/(5951.1*49.1/33.2); // [fb-1(Data) / fb-1(MC) (corrected by the PYTHIA overestimation of the cross-section)]


  // #################
  // # Cut selection #
  // #################
  double LowEdge  = 0.0;
  double HighEdge = 0.0;
  unsigned int nBins = 0;
  double cutValue;
  string fileName = "";
  if (scanType == 0)
    {
      LowEdge  = 0.0;
      HighEdge = 0.3;
      nBins    = 30;

      fileName = "CL";
    }
  else if (scanType == 1)
    {
      LowEdge  = 0.0;
      HighEdge = 40.0;
      nBins    = 40;

      fileName = "LS";
    }
  else if (scanType == 2)
    {
      LowEdge  = 0.999;
      HighEdge = 1.0;
      nBins    = 20;

      fileName = "cos";
    }
  else if (scanType == 3)
    {
      LowEdge  = 0.4;
      HighEdge = 2.0;
      nBins    = 32;

      fileName = "hadpT";
    }
  else if (scanType == 4)
    {
      LowEdge  = 0.0;
      HighEdge = 0.15;
      nBins    = 30;

      fileName = "kstM";
    }
  else if (scanType == 5)
    {
      LowEdge  = 0.8;
      HighEdge = 4.8;
      nBins    = 40;

      fileName = "hadDCAS";
    }
  else
    {
      cout << "[B0KstMuMuScanCuts::CutOptimization]\tWrong scan type: " << scanType << endl;
      exit (EXIT_FAILURE);
    }


  myString.str("");
  myString << "c0_" << fileName;
  TCanvas* c0 = new TCanvas(myString.str().c_str(),myString.str().c_str(),10,10,1600,900);

  TH1D* histoR1 = new TH1D("histoR1","histoR1",nBins,LowEdge,HighEdge);
  histoR1->SetXTitle("Cut value");
  histoR1->SetYTitle("S");
  histoR1->SetLineColor(kBlack);
  histoR1->SetFillColor(kAzure+6);
  TH1D* histoR2 = new TH1D("histoR2","histoR2",nBins,LowEdge,HighEdge);
  histoR2->SetXTitle("Cut value");
  histoR2->SetYTitle("B");
  histoR2->SetLineColor(kBlack);
  histoR2->SetFillColor(kAzure+6);
  TH1D* histoR3 = new TH1D("histoR3","histoR3",nBins,LowEdge,HighEdge);
  histoR3->SetXTitle("Cut value");
  histoR3->SetYTitle("S/#sqrt{B}");
  histoR3->SetLineColor(kBlack);
  histoR3->SetFillColor(kAzure+6);
  TH1D* histoR4 = new TH1D("histoR4","histoR4",nBins,LowEdge,HighEdge);
  histoR4->SetXTitle("Cut value");
  histoR4->SetYTitle("S/#sqrt{(S+B)}");
  histoR4->SetLineColor(kBlack);
  histoR4->SetFillColor(kAzure+6);


  cout << "\n@@@ Settings @@@" << endl;
  cout << "Signal sigma: " << signalSigma << endl;
  cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
  cout << setprecision (4) << "@ MCtoDataRescale: " << MCtoDataRescale << " @" << endl;
  cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;


  cout << "\n--> Scanning variable: " << fileName << endl;
  cout << "--> LowEdge: " << LowEdge << "\tHighEdge: " << HighEdge << "\tSteps: " << nBins << endl;
  cout << "--> Scanning ";
  if      (q2Region == 0) cout << "bins 0,1" << endl;
  else if (q2Region == 1) cout << "bin 7" << endl;
  else if (q2Region == 2) cout << "bins 0,1,7" << endl;
  else if (q2Region == 3) cout << "bins 0,1,special 2 (high edge q2 bin 2 = " << SpecialHighq2Bin2 << "),7" << endl;
  else if (q2Region == 4) cout << "bins 2,4,6" << endl;
  else if (q2Region == 5) cout << "all bins but 3,5" << endl;


  // #################
  // # Signal ntuple #
  // #################
  TFile* NtplFileInS = new TFile(MCFile.c_str(),"READ");
  TTree* theTreeInS = (TTree*)NtplFileInS->Get("B0KstMuMu/B0KstMuMuNTuple");
  B0KstMuMuTreeContent* NTupleS = new B0KstMuMuTreeContent();
  NTupleS->Init();
  NTupleS->ClearNTuple();
  NTupleS->SetBranchAddresses(theTreeInS);
  int nEntriesS = theTreeInS->GetEntries();
  cout << "\n@@@ Total number of events in the Signal tree: " << nEntriesS << " @@@" << endl;
  PrintCurrentTime();

  vector<double> countS;
  countS.assign(nBins,0);
  timeStart = time(NULL);
  for (int entry = 0; entry < nEntriesS; entry++)
    {
      theTreeInS->GetEntry(entry);

      for (unsigned int i = 0; i < nBins; i++)
	{
	  cutValue = LowEdge + static_cast<double>(i) * (HighEdge-LowEdge) / static_cast<double>(nBins);

	  if      (scanType == 0) Utility->SetSeleCut("B0VtxCL",    cutValue);
	  else if (scanType == 1) Utility->SetSeleCut("B0LsBS",     cutValue);
	  else if (scanType == 2) Utility->SetSeleCut("B0cosAlpha", cutValue);
	  else if (scanType == 3) Utility->SetSeleCut("HadpT",      cutValue);
	  else if (scanType == 4) Utility->SetSeleCut("KstMass",    cutValue);
	  else if (scanType == 5) Utility->SetSeleCut("HadDCASBS",  cutValue);

	  if ((Utility->ChooseBestCand(NTupleS, DoTrigCheck, static_cast<double>(entry)/static_cast<double>(nEntriesS), &BestCandIndx, &B0notB0bar, &TrigCat, &countCands) == true) &&

	      (((B0notB0bar == true) && (fabs(NTupleS->bMass->at(BestCandIndx) - Utility->B0Mass) < atof(Utility->GetGenericParam("NSigmaB0S").c_str())*signalSigma)) ||
	       ((B0notB0bar == false) && (fabs(NTupleS->bBarMass->at(BestCandIndx) - Utility->B0Mass) < atof(Utility->GetGenericParam("NSigmaB0S").c_str())*signalSigma))) &&
	      (NTupleS->genSignal == true) && (NTupleS->truthMatchSignal->at(BestCandIndx) == true) &&
	      
	      ((NTupleS->mumuMass->at(BestCandIndx) < (Utility->JPsiMass - atof(Utility->GetGenericParam("NSigmaPsiBig").c_str())*NTupleS->mumuMassE->at(BestCandIndx))) ||
	       (NTupleS->mumuMass->at(BestCandIndx) > (Utility->PsiPMass + atof(Utility->GetGenericParam("NSigmaPsiSmall").c_str())*NTupleS->mumuMassE->at(BestCandIndx))) ||
	       ((NTupleS->mumuMass->at(BestCandIndx) > (Utility->JPsiMass + atof(Utility->GetGenericParam("NSigmaPsiSmall").c_str())*NTupleS->mumuMassE->at(BestCandIndx))) &&
		(NTupleS->mumuMass->at(BestCandIndx) < (Utility->PsiPMass - atof(Utility->GetGenericParam("NSigmaPsiSmall").c_str())*NTupleS->mumuMassE->at(BestCandIndx))))) &&
	      
	      ((((q2Region == 0) || (q2Region == 2) || (q2Region == 3) || (q2Region == 5)) &&
		(NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) > q2Bins[0]) && (NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) < q2Bins[2])) ||
	       (((q2Region == 1) || (q2Region == 2) || (q2Region == 3) || (q2Region == 5)) &&
		(NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) > q2Bins[7]) && (NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) < q2Bins[8])) ||
	       ((q2Region == 3) &&
		(NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) > q2Bins[2]) && (NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) < SpecialHighq2Bin2)) ||
	       (((q2Region == 4) || (q2Region == 5)) &&
		(((NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) > q2Bins[2]) && (NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) < q2Bins[3])) ||
		 ((NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) > q2Bins[4]) && (NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) < q2Bins[5])) ||
		 ((NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) > q2Bins[6]) && (NTupleS->mumuMass->at(BestCandIndx)*NTupleS->mumuMass->at(BestCandIndx) < q2Bins[7]))))))
		  
	    countS[i]++;
	}

      if (entry%nEvPrint == 0)
	{
	  timeEnd = (time(NULL) - timeStart) * static_cast<double>(nEntriesS) / static_cast<double>(entry) - (time(NULL) - timeStart);
	  cout << "- Analyzed " << entry << " events (" << static_cast<double>(entry) / static_cast<double>(nEntriesS) * 100.0 << "%) --> " << timeEnd / 60.0 << " minutes to end\r" << flush;
	}
    }


  // #####################
  // # Background ntuple #
  // #####################
  TFile* NtplFileInB = new TFile(DataFile.c_str(),"READ");
  TTree* theTreeInB = (TTree*)NtplFileInB->Get("B0KstMuMu/B0KstMuMuNTuple");
  B0KstMuMuTreeContent* NTupleB = new B0KstMuMuTreeContent();
  NTupleB->Init();
  NTupleB->ClearNTuple();
  NTupleB->SetBranchAddresses(theTreeInB);
  int nEntriesB = theTreeInB->GetEntries();
  cout << "\n@@@ Total number of events in the Background tree: " << nEntriesB << " @@@" << endl;
  PrintCurrentTime();

  vector<double> countB;
  countB.assign(nBins,0);
  timeStart = time(NULL);
  for (int entry = 0; entry < nEntriesB; entry++)
    {
      theTreeInB->GetEntry(entry);
      
      for (unsigned int i = 0; i < nBins; i++)
	{
	  cutValue = LowEdge + static_cast<double>(i) * (HighEdge-LowEdge) / static_cast<double>(nBins);
	  
	  if      (scanType == 0) Utility->SetSeleCut("B0VtxCL",    cutValue);
	  else if (scanType == 1) Utility->SetSeleCut("B0LsBS",     cutValue);
	  else if (scanType == 2) Utility->SetSeleCut("B0cosAlpha", cutValue);
	  else if (scanType == 3) Utility->SetSeleCut("HadpT",      cutValue);
	  else if (scanType == 4) Utility->SetSeleCut("KstMass",    cutValue);
	  else if (scanType == 5) Utility->SetSeleCut("HadDCASBS",  cutValue);

	  if ((Utility->ChooseBestCand(NTupleB, DoTrigCheck, static_cast<double>(entry)/static_cast<double>(nEntriesB), &BestCandIndx, &B0notB0bar, &TrigCat, &countCands) == true) &&

	      (((B0notB0bar == true) &&
		(((NTupleB->bMass->at(BestCandIndx) > Utility->B0Mass-(atof(Utility->GetGenericParam("NSigmaB0B").c_str())+atof(Utility->GetGenericParam("NSigmaB0S").c_str()))*signalSigma) &&
		  (NTupleB->bMass->at(BestCandIndx) < Utility->B0Mass-atof(Utility->GetGenericParam("NSigmaB0B").c_str())*signalSigma)) ||
		 ((NTupleB->bMass->at(BestCandIndx) < Utility->B0Mass+(atof(Utility->GetGenericParam("NSigmaB0B").c_str())+atof(Utility->GetGenericParam("NSigmaB0S").c_str()))*signalSigma) &&
		  (NTupleB->bMass->at(BestCandIndx) > Utility->B0Mass+atof(Utility->GetGenericParam("NSigmaB0B").c_str())*signalSigma)))) ||
	       ((B0notB0bar == false) &&
		(((NTupleB->bBarMass->at(BestCandIndx) > Utility->B0Mass-(atof(Utility->GetGenericParam("NSigmaB0B").c_str())+atof(Utility->GetGenericParam("NSigmaB0S").c_str()))*signalSigma) &&
		  (NTupleB->bBarMass->at(BestCandIndx) < Utility->B0Mass-atof(Utility->GetGenericParam("NSigmaB0B").c_str())*signalSigma)) ||
		 ((NTupleB->bBarMass->at(BestCandIndx) < Utility->B0Mass+(atof(Utility->GetGenericParam("NSigmaB0B").c_str())+atof(Utility->GetGenericParam("NSigmaB0S").c_str()))*signalSigma) &&
		  (NTupleB->bBarMass->at(BestCandIndx) > Utility->B0Mass+atof(Utility->GetGenericParam("NSigmaB0B").c_str())*signalSigma))))) &&

	      ((NTupleB->mumuMass->at(BestCandIndx) < (Utility->JPsiMass - atof(Utility->GetGenericParam("NSigmaPsiBig").c_str())*NTupleB->mumuMassE->at(BestCandIndx))) ||
	       (NTupleB->mumuMass->at(BestCandIndx) > (Utility->PsiPMass + atof(Utility->GetGenericParam("NSigmaPsiSmall").c_str())*NTupleB->mumuMassE->at(BestCandIndx))) ||
	       ((NTupleB->mumuMass->at(BestCandIndx) > (Utility->JPsiMass + atof(Utility->GetGenericParam("NSigmaPsiSmall").c_str())*NTupleB->mumuMassE->at(BestCandIndx))) &&
		(NTupleB->mumuMass->at(BestCandIndx) < (Utility->PsiPMass - atof(Utility->GetGenericParam("NSigmaPsiSmall").c_str())*NTupleB->mumuMassE->at(BestCandIndx))))) &&
	      
	      ((((q2Region == 0) || (q2Region == 2) || (q2Region == 3) || (q2Region == 5)) &&
		(NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) > q2Bins[0]) && (NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) < q2Bins[2])) ||
	       (((q2Region == 1) || (q2Region == 2) || (q2Region == 3) || (q2Region == 5)) &&
		(NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) > q2Bins[7]) && (NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) < q2Bins[8])) ||
	       ((q2Region == 3) &&
		(NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) > q2Bins[2]) && (NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) < SpecialHighq2Bin2)) ||
	       (((q2Region == 4) || (q2Region == 5)) &&
		(((NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) > q2Bins[2]) && (NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) < q2Bins[3])) ||
		 ((NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) > q2Bins[4]) && (NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) < q2Bins[5])) ||
		 ((NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) > q2Bins[6]) && (NTupleB->mumuMass->at(BestCandIndx)*NTupleB->mumuMass->at(BestCandIndx) < q2Bins[7]))))))
	    
	    countB[i]++;
	}
      
      if (entry%nEvPrint == 0)
	{
	  timeEnd = (time(NULL) - timeStart) * static_cast<double>(nEntriesB) / static_cast<double>(entry) - (time(NULL) - timeStart);
	  cout << "- Analyzed " << entry << " events (" << static_cast<double>(entry) / static_cast<double>(nEntriesB) * 100.0 << "%) --> " << timeEnd / 60.0 << " minutes to end\r" << flush;
	}
    }


  // ############################
  // # Compute figures of merit #
  // ############################
  for (unsigned int i = 0; i < nBins; i++)
    {
      countS[i] = countS[i] * MCtoDataRescale;
      histoR1->SetBinContent(i+1,countS[i]);
      histoR2->SetBinContent(i+1,countB[i]);
      histoR3->SetBinContent(i+1,countB[i] > 0 ? countS[i] / sqrt(countB[i]) : 0);
      histoR4->SetBinContent(i+1,(countS[i]+countB[i]) > 0 ? countS[i] / sqrt(countS[i]+countB[i]) : 0);
      cout << "Signal: " << countS[i] << "\tBackground: " << countB[i] << "\tS/sqrt(B): " << countS[i] / sqrt(countB[i]);
      cout << "\ts/sqrt(S+B): " << countS[i] / sqrt(countS[i]+countB[i]) << endl;
    }


  cout << "\n--> Scanning variable: " << fileName << endl;

  cout << "\nMaximuml value for S/B: " << histoR3->GetMaximum() << " at X = " << histoR3->GetXaxis()->GetBinLowEdge(histoR3->GetMaximumBin()) << endl;
  cout << "Signal at maximum for S/B: " << countS[histoR3->GetMaximumBin()-1] << endl;
  if (scanType != 4) cout << "Percentage of signal at maximum for S/B: " << histoR1->GetBinContent(histoR3->GetMaximumBin()) / histoR1->GetBinContent(1) * 100.0 << endl;
  else cout << "Percentage of signal at maximum for S/B: " << histoR1->GetBinContent(histoR3->GetMaximumBin()) / histoR1->GetBinContent(histoR1->GetNbinsX()) * 100.0 << endl;

  cout << "\nMaximuml value for S/sqrt(S+B): " << histoR4->GetMaximum() << " at X = " << histoR4->GetXaxis()->GetBinLowEdge(histoR4->GetMaximumBin()) << endl;
  cout << "Signal at maximum for S/sqrt(S+B): " << countS[histoR4->GetMaximumBin()-1] << endl;
  if (scanType != 4) cout << "Percentage of signal at maximum for S/sqrt(S+B): " << histoR1->GetBinContent(histoR4->GetMaximumBin()) / histoR1->GetBinContent(1) * 100.0 << endl;
  else cout << "Percentage of signal at maximum for S/sqrt(S+B): " << histoR1->GetBinContent(histoR4->GetMaximumBin()) / histoR1->GetBinContent(histoR1->GetNbinsX()) * 100.0 << endl;


  c0->Divide(2,2);
  c0->cd(1);
  histoR1->Draw();
  c0->cd(2);
  histoR2->Draw();
  c0->cd(3);
  histoR3->Draw();
  c0->cd(4);
  histoR4->Draw();
  c0->Update();

  myString.str("");
  myString << "Scan_" << fileName << "_scanType" << scanType << "_q2Region" << q2Region << ".root";
  c0->Print(myString.str().c_str());

  PrintCurrentTime();
}


int main (int argc, char** argv)
{
  if (argc == 5)
    {
      unsigned int scanType = atoi(argv[1]);
      unsigned int q2Region = atoi(argv[2]);
      string MCFile         = argv[3];
      string DataFile       = argv[4];

      if (SETBATCH == true)
	{
	  cout << "\n@@@ Setting batch mode @@@" << endl;
	  gROOT->SetBatch(true);
	}
      TApplication theApp ("Applications", &argc, argv);


      // ##########################
      // # Set histo layout style #
      // ##########################
      gROOT->SetStyle("Plain");
      gROOT->ForceStyle();
      gStyle->SetPalette(1);
      gStyle->SetOptFit(1112);
      gStyle->SetOptStat(1110);
      gStyle->SetOptTitle(1);
      gStyle->SetPadRightMargin(0.02);
      gStyle->SetTitleOffset(1.25,"y"); 
      TGaxis::SetMaxDigits(3);


      cout << "\n@@@ Settings @@@" << endl;
      cout << "Parameter file: "         << ParameterFILE << endl;
      cout << "Do trig check: "          << DoTrigCheck << endl;
      cout << "Special high q2 bin #2: " << SpecialHighq2Bin2 << endl;
      cout << "nEvPrint: "               << nEvPrint << endl;
      cout << "SETBATCH: "               << SETBATCH << endl;


      Utility = new Utils();
      Utility->ReadAllBins(ParameterFILE,&q2Bins,&cosThetaKBins,&cosThetaLBins,&phiBins);
      Utility->ReadTriggerPathsANDCutsANDEntries(ParameterFILE);
      Utility->ReadSelectionCuts(ParameterFILE);
      Utility->ReadGenericParam(ParameterFILE);

 
      CutOptimization(scanType,q2Region,MCFile,DataFile);


      if (SETBATCH == false) theApp.Run (); // Eventloop on air
      return EXIT_SUCCESS;
    }
  else
    {
      cout << "Parameter missing: " << endl;
      cout << "./CutOptimization [scanType] [q2Region] [MC root file name] [Data root file name]" << endl;
      cout << "@@@@@@ Possible scanType values @@@@@@" << endl;
      cout << "0 = CL" << endl;
      cout << "1 = L/sigma" << endl;
      cout << "2 = cos(alpha)" << endl;
      cout << "3 = had pT" << endl;
      cout << "4 = mass(K*)" << endl;
      cout << "5 = hadDCA/sigma" << endl;
      cout << "@@@@@@ Possible q2Region values @@@@@@" << endl;
      cout << "0 = q2 bin 0,1" << endl;
      cout << "1 = q2 bin 7" << endl;
      cout << "2 = q2 bin 0,1,7" << endl;
      cout << "3 = q2 bin 0,1,special 2 (high edge q2 bin 2 = " << SpecialHighq2Bin2 << "),7" << endl;
      cout << "4 = q2 bin 2,4,6" << endl;
      cout << "5 = q2 bin all but 3,5" << endl;

      return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
