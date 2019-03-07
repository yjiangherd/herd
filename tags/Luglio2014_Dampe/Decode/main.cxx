#include <iostream>

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TStyle.h"
#include "TPaveStats.h"

#include "DecodeData.hh"
#include "Event.hh"
#include <stdlib.h>
#include <math.h>

using namespace std;


char progname[50];

int main(int argc,char** argv){
  char filename[255], pdf_filename[1024];
  char name[255];

  char DirRaw[255];
  char DirCal[255];
  char DirRoot[255];

  int run=110;
  int ancillary=0;

  int processed=0;
  int jinffailed=0;
  int readfailed=0;

  sprintf(DirRoot,"./RootData/");
  sprintf(DirRaw,"./RawData/");
  sprintf(DirCal,"./CalData/");

  sprintf(progname,"%s",argv[0]);
  if (argc >= 3) {
    run = atoi(argv[1]);
    ancillary = atoi(argv[2]);
    if (argc >= 4) {
      sprintf(DirRoot,"%s/",argv[3]);
      if (argc >= 5) {
	sprintf(DirRaw,"%s/",argv[4]);
	if (argc >= 6)
	  sprintf(DirCal,"%s/",argv[5]);
      }
    }
  } else{
    printf("Usage %s <runnum> <ancillary code (-1 is null)> [DirRoot] [DirRaw] [DirCal]  \n",argv[0]);
    printf("If you do not specify directories the default is:\n");
    printf(" DirRaw =  %s  \n", DirRaw);
    printf(" DirCal = %s \n", DirCal);
    printf(" DirRoot = %s \n", DirRoot);
    exit(1);  
  }

  printf("Reading Raw Data from %s\n", DirRaw);
  printf("Reading Cal Data from %s\n", DirCal);
  printf("Writing output in %s\n", DirRoot);

  DecodeData *dd1= new DecodeData(DirRaw,DirCal,run,ancillary);

  dd1->SetPrintOff();
  dd1->SetEvPrintOff();
  if (ancillary < 0)
    sprintf(filename,"%s/run_%06d.root",DirRoot,run);
  else
    sprintf(filename,"%s/run_%06d_ANC_%d.root",DirRoot, run, ancillary);
  sprintf(pdf_filename, "%s.pdf", filename);
  TFile *g=new TFile(filename,"RECREATE");
  TCanvas *canvas;
  char title[255], local_pdf_filename[255];
  for (int hh=0;hh<NTDRS;hh++){
    sprintf(name,"lad%d",hh);
    dd1->hmio[hh]= new TH1F(name,name,1024,0,1024);

  }

  TTree* t4= new TTree("t4","My cluster tree");
  t4->Branch("cluster_branch","Event",&(dd1->ev),32000,2);
  double chaK[24];
  for (int ii=0; ii<24; ii++) {
    t4->Branch(Form("ChargeK_Ladder%02d", ii), &chaK[ii], Form("ChargeK_Ladder%02d/D", ii));
  }
  t4->GetUserInfo()->Add(dd1->rh);


  int ret1=0;
  for (int ii=0;ii<10;){
    ret1=dd1->EndOfFile();    
    if (ret1) break;

    ret1=dd1->ReadOneEvent();
    
    ret1=dd1->EndOfFile();    
    if (ret1) break;

    if (ret1==0) {
      processed++;
      //      printf("This event has %d clusters\n", (dd1->ev)->NClusTot);
      memset(chaK, 0, 24*sizeof(chaK[0]));
      for (int cc=0; cc<(dd1->ev)->NClusTot; cc++) {
	Cluster* cl = (dd1->ev)->GetCluster(cc);
	if (cl->side==1) { //interesting only for K side (better resolution)
	  double charge = sqrt(cl->GetTotSig())/sqrt(35.0);
	  if (charge>chaK[cl->ladder]) {
	    chaK[cl->ladder]=charge;
	  }
	}
      }
      t4->Fill();
      if (processed%1000==0) printf("Processed %d events...\n", processed);
    }
    else if(ret1==-1){
      printf("=======================> END of FILE\n");
      break;
    }
    else if(ret1<-1){
      printf("=======================> READ Error Event Skipped\n");
      readfailed++;
      break;
    }
    else{
      jinffailed++;
    }


    dd1->ev->Clear();

  }

  bool first=true;
  gStyle->SetOptStat(1);
  gStyle->SetOptFit(1);
  for (int hh = 0; hh < NTDRS; hh++) {
    canvas = new TCanvas(name, name, 1024, 1024);
    sprintf(name, "ladder %d", hh);
    TF1 *fit_s = new TF1("fit_s", "gaus", 0, 639);
    TF1 *fit_k = new TF1("fit_k", "gaus", 640, 1023);
    fit_s->SetLineColor(kBlue);
    fit_k->SetLineColor(kRed);
    TH1F *clone_chartA = (TH1F *)dd1->hmio[hh]->Clone("cloneA");
    TH1F *clone_chartB = (TH1F *)dd1->hmio[hh]->Clone("cloneB");
    clone_chartA->Fit(fit_s, "R");
    clone_chartB->Fit(fit_k, "R");
    clone_chartA->Draw();
    gPad->Modified();
    gPad->Update();
    TPaveStats *statA = (TPaveStats*)(clone_chartA->GetListOfFunctions()->FindObject("stats"));
    clone_chartB->Draw("SAMES");
    gPad->Modified();
    gPad->Update();
    TPaveStats *statB = (TPaveStats*)(clone_chartB->GetListOfFunctions()->FindObject("stats"));
    if(statA && statB) {
      statA->SetTextColor(kBlue);
      statB->SetTextColor(kRed);
      statA->SetX1NDC(0.12); statA->SetX2NDC(0.32); statA->SetY1NDC(0.75);
      statB->SetX1NDC(0.72); statB->SetX2NDC(0.92); statB->SetY1NDC(0.78);
      statA->Draw();
      canvas->Update();
    }
    canvas->Update();
		
    int entries=dd1->hmio[hh]->GetEntries();
    canvas->Modified();
    canvas->Update();
    sprintf(title, "ladder %d", hh);
    canvas->SetTitle(title);
    if (entries>1) {
      if (!first) strcpy(local_pdf_filename, pdf_filename);
      else {
	sprintf(local_pdf_filename, "%s(", pdf_filename);
	first=false;
      }
      canvas->Print(local_pdf_filename, "pdf");
    }
    delete canvas;
  }
  TCanvas* c_exit = new TCanvas("dummy", "dummy", 1024, 1024);
  snprintf(local_pdf_filename, 255, "%s]", pdf_filename);
  c_exit->Print(local_pdf_filename, "pdf");
  delete c_exit; 

  t4->Write("",TObject::kOverwrite);

  printf("\nProcessed %5d  Events\n",processed+readfailed+jinffailed);
  printf("Accepted  %5d  Events\n",processed);
  printf("Rejected  %5d  Events --> Read Error\n",readfailed);
  printf("Rejected  %5d  Events --> Jinf/Jinj Error\n",jinffailed);

  delete dd1;

  g->Write("",TObject::kOverwrite);
  g->Close("R");

  return 0;
}