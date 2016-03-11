
/*
 *  TRich_Analysis.cpp
 *
 *
 *  Created by Matteo Turisini on 29/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */


#include <iostream>   // cout
#include <fstream>   // ifstream
#include <stdint.h>  // uint64_t
#include <inttypes.h>  // PRIu64
#include <stdio.h>  // printf
#include <string.h>  // std::string

#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TROOT.h>
#include <TGraph.h>
#include <TCut.h>
#include <TStyle.h>
#include <TColor.h>
#include <TGraphErrors.h>
#include <TPaveStats.h>
#include <TError.h>


#include "TRich_Analysis.h"
#include "TRich_Config.h"
#include "TRich_Scaler.h"
#include "TRich_TDC.h"
#include "TRich_Adapter.h"
#include "TRich_Calib.h"

#define SIZEOFCANV 200

#define HEVTTIME "triggerTIME"
#define HEVTFREQ "triggerFREQ"
#define HEVTEDGE "nedges_SCAT"
#define H_NEDGES "nedges_DISTRIB"
#define HBRDRISE "riseTIME"
#define HBRDFALL "fallTIME"

#define TEXT	 0
#define UNIF	 1
#define PIXEL	 2

 


TRich_Analysis::TRich_Analysis(const char * fileraw){
  
  for (int i=0; i<192; i++) {
    feff[i] =0.0;
    fposedge[i]=0;
  }

  
  fdaq_mode = -1;
  frepetition = -1;
  frunID_first = -1;
  frunID_last = -1;
  
  
  fTDC_repetition = -1;
  fTDC_runID_first = -1;
  fTDC_runID_last = -1;
  
  fsource="unknow_source" ;
  fplot="plottype_not_selected";
  fnorm=1.0;
  
  fthrscan_eps_all = "unknown_name_for_threshold_scan_plot";
  
  finraw = "";
  finlog = "";
  fintxt = "";
  fioTDCroot = "";

  fioSKAroot_Scan = "unknown_name_for_threshold_scan_root_file";
  fprefix = "";
 // fPlotTimeStamp = "";
  
  
  frun_path = " unknown path";
  frun_file = " unknownfile";
  frun_name = " unknownname";
  frun_prefix = " unknown prefix"; 
  frun_id = -1; 
  frun_suffix= " unknown suffix";
  
  gErrorIgnoreLevel = 3000;
  
  fTDCroot = "";
  fTDCscanroot = "";
  
  fTDCpdf= "";
  fTDCscanpdf= "";
  
  
  if (fileraw==NULL) {
    //printf("**************** Analysis object created\n");
  }else {
    this->NameRun(fileraw);
  }
  
  // da aggiungerer al metodo THR_SCAN
  //printf("Import Configuration data and logbook info \n");
  //TRich_Config * fcfg = new TRich_Config(finlog.c_str(),1);
  
  //  get parameters
  //int	nevts	= fcfg->GetLogNEvents();
  
  //printf("Run %s\n",finraw.c_str());
  //printf("Nevents   = %d\n",nevts);
  //printf("Threshold = %d\n",thr);
  
  
}

TRich_Analysis::~TRich_Analysis(){
  
  //printf("delete Analysis\n");
}


bool TRich_Analysis::SingleRun(){
  bool ret = (fmode == SINGLE_RUN) ? true :false;
  return ret;
}
bool TRich_Analysis::Scan(){

  bool ret = (fmode == SCAN) ? true :false;
  return ret;
}


void TRich_Analysis::SetDaqMode(int daqmode){
  
  fdaq_mode = daqmode; 
  
} 
void TRich_Analysis::SetSource(std::string sourceName,std::string plotName,int normalization)
{
  
  fsource=sourceName ;
  fplot=plotName;
  fnorm=normalization;
  
}


void TRich_Analysis::Print(){
  
  
  printf("PATH  : %s\n",frun_path.c_str());
  printf("FILE  : %s\n",frun_file.c_str());
  
  printf("NAME  : %s\n",frun_name.c_str());
  printf("SUFFIX: %s\n",frun_suffix.c_str());
  
  printf("PREFIX: %s\n",frun_prefix.c_str());
  printf("ID    : %d\n",frun_id);
  
  printf("Raw   : %s\n",finraw.c_str()); 
  printf("Txt   : %s\n",fintxt.c_str()); 
  printf("Log   : %s\n",finlog.c_str()); 
  
  printf("Data analysis: ");
  
  switch (fdaq_mode) {
  case 0:
    printf("SCALER \n");
    printf("\tCycles: %d from %d to %d\n",frepetition,frunID_first,frunID_last); 
    printf("\tRoot out:   %s\n",fioSKAroot_Scan.c_str()); 
    printf("\tPdf  out:  %s\n",fthrscan_eps_all.c_str());
    
    printf("\tSource: \"%s\"\n",fsource.c_str());
    printf("\tPlot: \"%s\" \n",fplot.c_str());
    printf("\tNormalization: %6d\n",fnorm);
    
    break;
  case 1:
    printf("TDC \n");
    switch(fmode){
      
    case SINGLE_RUN:
      printf("\tROOT %s\n",fTDCroot.c_str()); 
      printf("\tPDF  %s\n",fTDCpdf.c_str()); 
      break;
      
    case SCAN: 					
      printf("\tROOT %s\n",fTDCscanroot.c_str()); 
      printf("\tPDF  %s\n",fTDCscanpdf.c_str()); 
      break;
      
    default: 
      printf("Warning: scan mode not setted \n");
      break;
    }
    break;	
  default:
    printf("Warning: daq mode not setted \n");
    break;
  }
}





/************/
/*   UTILS  */
/************/

int TRich_Analysis::SetNames(std::string runPrefix,int runID_first,int runID_last,std::string runPath)
{	
  if (runPrefix==NULL) {printf("Error in %s: runprefix not specified\n",__FUNCTION__);return -1;}	
  if ((runID_first==0)&&(runID_last==0)) {printf("Error in %s: runID not specified\n",__FUNCTION__);return -1;}	
  if (runID_first>runID_last) {printf("Error in %s: something wrong with runID\n",__FUNCTION__);return -1;}
  
  
  frun_prefix  = runPrefix;	
  frunID_first = runID_first;
  frun_path = runPath;
  
  
  if (runID_last==0||runID_last==runID_first) { // single run analysis	  
    frepetition = 1;	  
    frunID_last = runID_first; // maybe redundant	
  }else { // run cycle analysis
    frepetition  =  runID_last - runID_first+1;
    frunID_last = runID_last;
    // generate the names of all the runs in a scan
    //string runName;
    //for (int i=0; i<frepetition; i++) {
    //runName = runPath + runPrefix;
    //printf("\t%d) %s\n",i+1,runName.c_str());
    //}
    
    // set the filename and path for the ROOT file that will house the data of the run cycle
    std::string file;
    char numstrfirst[7]; 
    char numstrlast[7];
    sprintf(numstrfirst, "_%06d",  runID_first);
    sprintf(numstrlast, "_%06d", runID_last);
    file.clear();
    file += "../data/scan/";
    file += frun_prefix;
    file += numstrfirst;
    file += numstrlast;
    file += ".root";
    fioSKAroot_Scan = file;
    
    // set name and path for the 2D plot (all 192 channel superimposed)
    
    file.clear();
    file += "../eps/2D_";
    file += frun_prefix;
    file += ".eps";
    fthrscan_eps_all = file;
  }
  return frepetition;
}


void TRich_Analysis::Ingest(){

	int nevt;
	
	printf("%s...\n",__FUNCTION__);
	fflush(stdout);

	std::string fileroot;
  std::string filelog;
  std::string fileraw;
 

  TRich_Config cfg;

	switch (fdaq_mode) {
  		
		case 0: // SCALER 
   	break;
  
		case 1: // TDC
    	switch(fmode){
				case SINGLE_RUN:
				nevt = this->TDC_Read(); // parse single run, writing an output Ttree with Time stamp and edge list
				printf("Read Single Run Returns %d\n",nevt);
      	break;
    
				case SCAN:
				this->TDC_Scan_Read(); // extract single run statistics and create a tree dedicated to scans
				break;				

				default: 
      	printf("Warning: scan mode not setted \n");
      	break;
    	}
    break;
		case 2: // ADC
		break;
  	default:
    printf("Warning: daq mode not setted \n");
   	break;
  }
}

void TRich_Analysis::TDC_Scan_Read(){
	
	bool printfilename = false;
	bool print = false;

	int r; // run 

	int rA = fTDC_runID_first;// first
	int rB = fTDC_runID_last; // last

	int ch; // channel 
	int chA=0; // first channel 
	int chB=191; // last channel
  
	// init output file... fTDC_runID_last must be correctly setted 
	char lnumstr[7]; // run ID
 	sprintf(lnumstr, "_%06d",  fTDC_runID_last);	
	std::string outfile = SCANPATH   + frun_name  + lnumstr + "_scan" + ".root";

	TRich_Calib * calib = new TRich_Calib(outfile.c_str());

	for(r=rA;r<=rB;r++){ // loop on runs

		calib->Reset();

		calib->RunID(r);

		// set  file names for this run (suffix _histosingle.root and .log)
		sprintf(lnumstr, "%06d",  r);
		std::string infile   = HISTOPATH + frun_prefix + lnumstr + "_histosingle" + ".root";
		std::string logbook  = "../../data/out/" + frun_prefix + lnumstr + ".log";
		if(printfilename)printf("%5d Histograms: %s \n",r,infile.c_str());
		//printf("%5d Logbook   : %s ",r,logbook.c_str());

		// retrieve data from logbook
		TRich_Config cfg;
		cfg.ParseFile(logbook.c_str());
  	cfg.Read();

		calib->Threshold(cfg.Threshold());
		calib->Gain(this->GetGain());	
  	calib->NTrigger(cfg.GetLogNEvents());

		// retrieve single channel TDC spectra (Rise,Fall, Duration)

		TFile * f = new TFile(infile.c_str(),"READ"); 

		for(ch=chA;ch<=chB;ch++){

			bool tris = true;

			TH1F * hrise = NULL;
			TH1F * hfall = NULL;
			TH1F * hdura = NULL;
		
			hrise = (TH1F*) f->Get(Form("ch%03d_rise",ch));  // 0 Rising Edges
			hfall = (TH1F*) f->Get(Form("ch%03d_fall",ch));  // 1 Falling Edges
			hdura = (TH1F*) f->Get(Form("ch%03d_dura",ch));  // 2 Time Over Threshold
	
			if(hrise==NULL){if(print){printf("Histo Rising Channel %d missing...\n",ch);} tris = false ;}
			if(hfall==NULL){if(print){printf("Histo Falling Channel %d missing...\n",ch);} tris = false;}
			if(hdura==NULL){if(print){printf("Histo Duration Channel %d missing...\n",ch);} tris = false;}

			if(!tris) continue;

			calib->Entries(0, hrise->GetEntries(),ch);
			calib->Entries(1, hfall->GetEntries(),ch);		
			calib->Entries(2, hdura->GetEntries(),ch);
			
			calib->Mean(0,hrise->GetMean(),ch);
			calib->Mean(1,hfall->GetMean(),ch);
			calib->Mean(2,hdura->GetMean(),ch);

			calib->RMS(0,hrise->GetRMS(),ch);
			calib->RMS(1,hfall->GetRMS(),ch);
			calib->RMS(2,hdura->GetRMS(),ch);

		}
		if(print){calib->Print();}
		f->Close();
		delete f;
		calib->Fill();
	}
	calib->Write();
	delete calib;
	printf("Scan data in %s\n",outfile.c_str());
	return;

}



void TRich_Analysis::TDC_Scan_Histo()
{

	char lnumstr[7]; // run ID
 	sprintf(lnumstr, "_%06d",  fTDC_runID_last);	
	std::string infile = SCANPATH   + frun_name  + lnumstr + "_scan" + ".root";
	std::string outfile = HISTOPATH   + frun_name  + lnumstr + "_scanhisto" + ".root";

//	printf("%s input  = %s\n",__FUNCTION__,infile.c_str());
//	printf("%s output = %s\n",__FUNCTION__,outfile.c_str());

//	printf("FIRST  RUN %d \n",fTDC_runID_first);	
//	printf("LAST   RUN %d \n",fTDC_runID_last);

	TFile * f = new TFile(outfile.c_str(),"RECREATE");

	// Histograms declaration
	// 192 TH1F for:
	// 0 : Efficiency Rise 
	// 1 : Efficiency Fall
	// 2 : Efficiency Duration
	// 3 : Delay Leading  (falling)
	// 4 : Delay Trailing (rising)
	// 5 : Jittter Leading  (falling)
	// 6 : Jittter Trailing (rising)
	// 7 : Duration (time over threshold)
	// 8 : Duration RMS (time over threshold)
	int nvar = 9;	

	int ch;
	int chA =0;
	int chB = 191;

	TH1F * hthr[192][nvar];

	int thr;
	int gain;
	int ntrig;

	TH1F * hNtrig;

	double effFall;
	double effRise;
	double effDura;

	double delayFall;
	double jitterFall;
	double delayRise;
	double jitterRise;

	double duration;
	double durationRMS;

	int nbinTHR = 300;
	double lowTHR = 0-0.5;
	double highTHR = 600-0.5;


	for(ch=chA;ch<=chB;ch++){
			hthr[ch][0] = new TH1F(Form("ch%d_EffFall",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][1] = new TH1F(Form("ch%d_EffRise",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][2] = new TH1F(Form("ch%d_EffDura",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][3] = new TH1F(Form("ch%d_DlyFall",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][4] = new TH1F(Form("ch%d_DlyRise",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][5] = new TH1F(Form("ch%d_ttsFall",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][6] = new TH1F(Form("ch%d_ttsRise",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][7] = new TH1F(Form("ch%d_MeanDur",ch),"",nbinTHR,lowTHR,highTHR);
			hthr[ch][8] = new TH1F(Form("ch%d_RMSDura",ch),"",nbinTHR,lowTHR,highTHR);
	}
	
	hNtrig= new TH1F("ntrig","",nbinTHR,lowTHR,highTHR);
	
	TH2F * h2 = new TH2F("h2_EffDura","",191,-0.5,190.5,nbinTHR,lowTHR,highTHR);

	TRich_Calib c;

	if(!c.OpenFile(infile.c_str())){printf("File %s not found\n",infile.c_str());return;} 
	
	int entries = c.GetTree();

	printf("Found %d runs\n",entries);

	for(int e=0;e<entries;e++)
	{
		c.GetEntry(e);
		c.Print();
		ntrig = c.NTrigger();

		hNtrig->Fill(thr,ntrig);

		thr = c.Threshold(-1,0);

		if(ntrig==0) continue;
		
		for(ch=chA;ch<=chB;ch++){

			gain = c.Gain(-1,ch); 
			
			effFall =  c.Entries(1,-1,ch)/ntrig;
			effRise =  c.Entries(0,-1,ch)/ntrig;
			effDura =  c.Entries(2,-1,ch)/ntrig;

			//printf("eff[%3d]= %.3lf %.3lf %.3lf\n ",ch,effFall,effRise,effDura);

			delayFall = c.Mean(1,-1,ch);
			jitterFall = c.RMS(1,-1,ch);

			delayRise = c.Mean(0,-1,ch);
			jitterRise = c.RMS(0,-1,ch);

			duration = c.Mean(2,-1,ch);
			durationRMS = c.RMS(2,-1,ch);		


			hthr[ch][0]->Fill(thr,effFall);
			hthr[ch][1]->Fill(thr,effRise);
			hthr[ch][2]->Fill(thr,effDura);
			hthr[ch][3]->Fill(thr,delayFall);
			hthr[ch][4]->Fill(thr,delayRise);
			hthr[ch][5]->Fill(thr,jitterFall);
			hthr[ch][6]->Fill(thr,jitterRise);
			hthr[ch][7]->Fill(thr,duration);
			hthr[ch][8]->Fill(thr,durationRMS);

			h2->Fill(ch,thr,effDura);

		}
	}

	f->cd();
	hNtrig->Write();
	delete hNtrig;
	for(int k = 0; k<nvar; k++){
		for(ch=chA;ch<=chB;ch++){
			hthr[ch][k]->Write();
			delete hthr[ch][k];
		}
	}

	h2->Write();
	delete h2;

	f->Close();
	delete f;
	
	printf("Scan Histograms in %s\n",outfile.c_str());


}	
void TRich_Analysis::TDC_Scan_Plot1()
{
// Create a pdf with plots vs THR for different gain
	char lnumstr[7]; // run ID
 	sprintf(lnumstr, "_%06d",  fTDC_runID_last);	
	std::string infile = HISTOPATH   + frun_name  + lnumstr + "_scanhisto" + ".root";
	std::string outfile = PDFPATH   + frun_name  + lnumstr + "_vsTHR" + ".pdf";

	//printf("%s input  = %s\n",__FUNCTION__,infile.c_str());
	//printf("%s output = %s\n",__FUNCTION__,outfile.c_str());
	
	const char * p = 	outfile.c_str();
	const char * canvname = "c";
	int rows = 3;
	int columns = 3;
	int padsize = 200;
	int padmargin = 8.; //%	
	int sizex = columns *padsize;
	int sizey = rows * padsize;
	TCanvas * c = new TCanvas(canvname,"",sizex,sizey);
	c->Print(Form("%s[",p)); 
	c->Divide(columns,rows);
	const int NPAD = columns*rows;
	TPad * pad[NPAD];
	float padm = padmargin/100.; 	
	for(int p=0;p<NPAD;p++){
		pad[p]=(TPad*)(c->FindObject(Form("%s_%d",canvname,p+1)));	
		pad[p]->cd();
  	pad[p]->SetGrid(kTRUE);
		pad[p]->SetLogx(kFALSE);
		pad[p]->SetLogy(kFALSE);
		pad[p]->SetMargin(padm,padm,padm,padm); // left, right, bottom, top
	}
	
	TFile * f = new TFile(infile.c_str(),"READ");

	TH1F * h[9]; 

	int ch;
	int chA = 0;
	int chB = 191;

	int max_eff = 4;
	int tmax = 800;
	int jmax = 10;

	int color;
	for(ch = chA; ch<=chB; ch++){

		h[0] = (TH1F*) f->Get(Form("ch%d_EffFall",ch));
		h[1] = (TH1F*) f->Get(Form("ch%d_EffRise",ch));
		h[2] = (TH1F*) f->Get(Form("ch%d_EffDura",ch));
	
		h[3] = (TH1F*) f->Get(Form("ch%d_DlyFall",ch));
		h[4] = (TH1F*) f->Get(Form("ch%d_DlyRise",ch));
		h[5] = (TH1F*) f->Get(Form("ch%d_MeanDur",ch));
	
		h[6] = (TH1F*) f->Get(Form("ch%d_ttsFall",ch));
		h[7] = (TH1F*) f->Get(Form("ch%d_ttsRise",ch));
		h[8] = (TH1F*) f->Get(Form("ch%d_RMSDura",ch));
	
		h[0]->SetMaximum(max_eff);
		h[1]->SetMaximum(max_eff);
		h[2]->SetMaximum(max_eff);

		h[3]->SetMaximum(tmax);
		h[4]->SetMaximum(tmax);
		h[5]->SetMaximum(tmax);

		h[6]->SetMaximum(jmax);
		h[7]->SetMaximum(jmax);
		h[8]->SetMaximum(jmax);


		for(int k = 0; k<9;k++){
			pad[k]->cd();
			if(h[k]){
				switch(k%3){
					case 0: color = kRed; break;//fall 
					case 1: color = kBlue; break;// rise
					case 2: color = kMagenta; break;// time over threshold
				}
				h[k]->SetLineColor(color);
				h[k]->GetXaxis()->SetTitle("Threshold [DAC unit]");
				h[k]->GetXaxis()->CenterTitle();
				h[k]->Draw();
			}
		}
		c->Print(Form("%s",p)); 
	}
	c->Print(Form("%s]",p)); 
	delete c;
	f->Close();
	delete f;
	printf("Single Channel TDC Scan Plots in %s\n",p);

}
void TRich_Analysis::TDC_Scan_Plot2()
{
// creates gauguin plot X = channel Y = threshold Z = efficincy Duration

	char lnumstr[7]; // run ID
 	sprintf(lnumstr, "_%06d",  fTDC_runID_last);	
	std::string infile = HISTOPATH   + frun_name  + lnumstr + "_scanhisto" + ".root";
	std::string outfile = PDFPATH   + frun_name  + lnumstr + "_vsTHR_vsChannel" + ".pdf";
	const char * canvname = "c";

	int rows = 1;
	int columns = 1;
	int padsize = 200;
	int padmargin = 15.; //%	
	int sizex = columns *padsize;
	int sizey = rows * padsize;
	TCanvas * c = new TCanvas(canvname,"",sizex,sizey);
	c->Divide(columns,rows);
	const int NPAD = columns*rows;
	TPad * pad[NPAD];
	float padm = padmargin/100.; 	
	for(int p=0;p<NPAD;p++){
		pad[p]=(TPad*)(c->FindObject(Form("%s_%d",canvname,p+1)));	
		pad[p]->cd();
  	pad[p]->SetGrid(kFALSE);
		pad[p]->SetLogx(kFALSE);
		pad[p]->SetLogy(kFALSE);
		pad[p]->SetMargin(padm,padm,padm,padm); // left, right, bottom, top
	}
	
	TFile * f = new TFile(infile.c_str(),"READ");

	TH2F * h2 = (TH2F*) f->Get("h2_EffDura");
  
  const Int_t NRGBs = 5;
  const Int_t NCont = 512;
  
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);


	double minEff = 0.0;
	double maxEff = 1.5;

	if(h2){
 		gStyle->SetOptStat(0);
		pad[0]->cd();
 		h2->SetMinimum(minEff);
 		h2->SetMaximum(maxEff);
  	h2->GetXaxis()->SetTitle("Channel ID");	h2->GetXaxis()->CenterTitle();
  	//h2->GetXaxis()->SetTitle("ANODE ID");	h2->GetXaxis()->CenterTitle();
 		h2->GetYaxis()->SetTitle("Threshold [DAC unit]");	h2->GetYaxis()->CenterTitle();	
		h2->GetZaxis()->SetTitle("Hit Efficiency [# hit / # trigger ]");	h2->GetZaxis()->CenterTitle();	
 		h2->GetYaxis()->SetTitleOffset(1.5);
		h2->GetXaxis()->SetTitleOffset(1.5);
		h2->GetZaxis()->SetTitleOffset(1.5);
		h2->Draw("COLZ");
		c->Print(outfile.c_str()); 
	}	
	
	delete c;
	f->Close();
	delete f;
	printf("Gauguin TDC Scan Plots in %s\n",outfile.c_str());
}



void TRich_Analysis::TDC_Scan_Plot3()
{
// creates calibration plots X = threshols, y = gain, z = efficiency/jitter		


	char lnumstr[7]; // run ID
 	sprintf(lnumstr, "_%06d",  fTDC_runID_last);	
	std::string infile = HISTOPATH   + frun_name  + lnumstr + "_scanhisto" + ".root";
	std::string outfile = PDFPATH   + frun_name  + lnumstr + "_vsTHR_vsGain" + ".pdf";

	printf("%s input  = %s\n",__FUNCTION__,infile.c_str());
	printf("%s output = %s\n",__FUNCTION__,outfile.c_str());

}	



int	TRich_Analysis::NameRun(const char * fileraw,int runID_last,bool enablePrint){
  
  // print args for debug	
  //printf("inFile = %s , last run ID %d\n",fileraw,runID_last);
  
  // check input file exists
  std::ifstream f(fileraw);
  if(!f.good()){f.close();return -1;}
  
  finraw = fileraw;
  
  // parse input filename
  unsigned found = finraw.find_last_of("/\\");	 // look for last "/" (from left to right)
  
  
  frun_path = finraw.substr(0,found+1);
  frun_file = finraw.substr(found+1);	
  
  found = frun_file.find_last_of(".\\");		// look for last "." (from left to right)	
  
  frun_name= frun_file.substr(0,found);
  frun_suffix = frun_file.substr(found);	
  
  found =  frun_name.find_last_of("_\\");	
  
  std::string lrunID = frun_name.substr(found+1);
  frun_prefix= frun_name.substr(0,found+1);
  frun_id = atoi(lrunID.c_str());
  
  
  
  // configuration name
  finlog = frun_path + frun_name + ".log";
  
  // data txt name
  fintxt = frun_path + frun_name + ".txt";
  
  
  // tutti i nomi relativi al file di ingresso sono stati settati
  
  // ora dovrei gestire il caso SCALER o TDC e quello SINGLE RUN o SCAN
  
  
  int repetition = runID_last-frun_id;
  
  if(repetition<0){return -2;}
  if(repetition==0){fmode=SINGLE_RUN;}
  if(repetition>0){fmode=SCAN;}
  
  char lnumstr[7];
  
  
  switch(fdaq_mode){
    
  case 0: 
    printf("SCALER DATA");
    break;
    
  case 1: 
    //			printf("TDC DATA");
    switch(fmode){
      
    case SINGLE_RUN:
      //printf(" SINGLE RUN");
      fTDCroot = "../../data/parsed/" + frun_name + "_TDC" + ".root";
      fTDCpdf			= "../../pdf/" + frun_name +".pdf";
      
      
      // nome tdc.root
      break;
      
    case SCAN: 
      //	printf(" SCAN");
      
      fTDC_repetition = repetition;
      fTDC_runID_first = frun_id;
      fTDC_runID_last = runID_last;
      
      sprintf(lnumstr, "_%06d",  runID_last);	
      
      fTDCscanroot = "../../data/parsed/" + frun_name + lnumstr + "_TDC" +"_SCAN"+ ".root";
      fTDCscanpdf	= "../../pdf/" + frun_name + lnumstr + "_TDC" +"_SCAN" +".pdf";

      break;
    default: 
      break;
      
    }
    break;
  default:
    printf("Warning: Unknown daq mode\n"); 
    break;
  }
  fioTDCroot = "../../data/parsed/" + frun_name + "_TDC" + ".root";

  return 	frun_id;
}


unsigned int TRich_Analysis::GetThreshold(){
  /*
    unsigned int threshold =0;
    TRich_Config * cfg = new TRich_Config();
    cfg->ParseFile(finlog.c_str());
    cfg->Read(); // import configuration data
    threshold = cfg->Threshold();
    delete cfg;
    return threshold;
  */
  TRich_Config cfg; 
  
  cfg.ParseFile(finlog.c_str());
  
  cfg.Read(); // import configuration data
  
  return (unsigned int) cfg.Threshold();
  
  
}

unsigned int TRich_Analysis::GetGain(){
  
  TRich_Config cfg; 
  
  cfg.ParseFile(finlog.c_str());
  
  cfg.Read(); // import configuration data
  
  return (unsigned int) cfg.Gain();
}



unsigned int TRich_Analysis::GetPolarDiscri(){
  
  TRich_Config cfg; 
  
  cfg.ParseFile(finlog.c_str());
  
  cfg.Read(); // import configuration data
  
  return cfg.GetPolarDiscri();
	
}


unsigned int TRich_Analysis::GetFPGAParameters(int opt){
  
  unsigned int ret=-1;
  TRich_Config * cfg = new TRich_Config();
  cfg->ParseFile(finlog.c_str());
  cfg->Read(); // import configuration data, in fact FPGA settings are copied to cfg private data members
  
  switch (opt) {
  case 0: ret = cfg->GetTriggerDelay();
    break; 
  case 1: ret = cfg->GetEventBuilderLookBack();
    break; 
  case 2: ret = cfg->GetEventBuilderWindow();	
    break; 
  default:
    printf("Warning in %s: unknown option\n",__FUNCTION__);
    break;
  }
  delete cfg;
  return ret;
}


unsigned int TRich_Analysis::GetTriggerDelay(){return GetFPGAParameters(0);}
unsigned int TRich_Analysis::GetEventBuilderLookBack(){return GetFPGAParameters(1);}	
unsigned int TRich_Analysis::GetEventBuilderWindow(){return GetFPGAParameters(2);}




double	TRich_Analysis::GetEfficiency(int channel){

	//  argument check TO BE ADDED
	
	return feff[channel];
}


/**********/
/*   TDC  */
/**********/
int	TRich_Analysis::TDC_Read(){ 
	
	// open binary file and determine its size
	FILE *fbin = fopen(finraw.c_str(), "rb");	
	if(fbin==NULL)fputs("File Error",stderr);
	fseek(fbin,0,SEEK_END);
	long lSize = ftell (fbin);
	rewind(fbin);
	printf("File %s         (%10ld bytes)\n",finraw.c_str(),lSize);


	// copy binary data into plain text file 
	FILE *ftxt = fopen(fintxt.c_str(), "wt");
	int val;
	int n;
	int nwords = 0;
	int i;
	for(i=0;i<(lSize/4);i++){
		n = fread(&val,1,sizeof(val),fbin);
	  //printf("%10d 0x%08X %d\n",i,val,n);
	  if(n!=sizeof(val)){printf("Error while reading binary file\n");break;}
	  fprintf(ftxt,"%08X\n",val); 
	}
	fclose(fbin);
	fclose(ftxt);
	nwords = i-1;
	printf("File %s         (%10d words) ",fintxt.c_str(),nwords); fflush(stdout);


	// get logbook parameters
	TRich_Config cfg;
	cfg.ParseFile(finlog.c_str());
  cfg.Read();
	//printf("TRIG_DELAY %3d ",cfg.GetTriggerDelay());
	//printf("LOOKBACK %4d ",cfg.GetEventBuilderLookBack());
	//printf("WINDOW %4d ",cfg.GetEventBuilderWindow());
  //printf("THR %4d ",cfg.Threshold());
  //printf("GAIN %4d ",this->GetGain());
	//printf("\n");
	// retrieve the total number of events	
	int Nevents = cfg.GetLogNEvents();
	printf("Parsing %d events ",Nevents);fflush(stdout);

	// Parse the data
  char lv = 0; // 0 no print, 1 hex, 2 decoded, 3 events // local verbosity

  int edgec=0;

  
  ifstream fin(fintxt.c_str(),fstream::in);
  unsigned int word;
  int maxword = 1E9; 
  int tag = 15;
  int tag_idx = 0;
  int zword = 0;
  
  TRich_TDC * evt = new TRich_TDC(fioTDCroot.c_str());
  

  int evtid; // event counter
  unsigned int e;
  
  for (i=0; i<maxword; i++) {
    
    // check for end of file
    if (fin.eof()) {if(lv==1){printf("eof reached\n");}break;}
    
    // read one word
    fin >> std::hex >> word;
    
    // check that word is not zero (could it be in case of non-blocking socket)
    if(word==0){zword++; continue;}
    
    if(lv==1){printf("word[%6d]: hex = 0x%08X  tag = %2d  tag_idx = %d\n",i,word,tag,tag_idx);}
    
    // prepare for parsing the event		
    if(word & 0x80000000){ // 
      
      tag = (word>>27) & 0xF;
      tag_idx = 0;
      
    }else{
      tag_idx++;
    }
    
    switch(tag){
    	case 0:	if (lv==2) {printf("[BLOCKHEADER] SLOT=%d, BLOCKNUM=%d, BLOCKSIZE=%d", (word>>22)&0x1F, (word>>8)&0x3FF, (word>>0)&0xFF);}
    	  break;
    	case 1:	if (lv==2) {printf("[BLOCKTRAILER] SLOT=%d, WORDCNT=%d", (word>>22)&0x1F, (word>>0)&0x3FFFFF);}
      	break;
    	case 2: //Event Header

      	// new header found, the previous event is concluded lets finalize the new entry and fill the tree
				evt->EdgeList();				
				evt->Print(2,33);//evt->Print(2,34);// 0 nothing, 1 general info, 2, channel info use second argument to select channel, use 192 for all the channels			
				evt->Fill();
      	//evt->Reset(); // mandatory, it is done at the end of evt->Fill() 

				if(lv==0 && !(evtid % (Nevents/10))){  printf(".");fflush(stdout);}
      	
				if(lv==2||lv==3){printf("\n");}

				// process the header (Trigger ID, Device ID)
      	evtid = (word>>0)&0x3FFFFF;
				evt->TrigNum((word>>0)&0x3FFFFF); 
      	evt->DevId((word>>22)&0x1F);
      	if (lv==2) {printf("[EVENTHEADER] TRIGGERNUM=%d, DEVID=%d", (word>>0)&0x3FFFFF, (word>>22)&0x1F);}
				if (lv==3) {printf("Evt %5d ", word&0x3FFFFF);}
      	break;


    	case 3: // Time stamp	
      	evt->Tstamp(tag_idx,(word>>0)&0xFFFFFF);
      	if(tag_idx == 0){
					if(lv==2){printf("[TIMESTAMP 0] TIME=%8d ", (word>>0)&0xFFFFFF);}
					if(lv==3){printf("T0 %8d ", (word>>0)&0xFFFFFF);}
      	}else if(tag_idx == 1){
					if(lv==2){printf("[TIMESTAMP 1] TIME=%8d ",(word>>0)&0xFFFFFF);}
					if(lv==3){printf("T1 %8d ", (word>>0)&0xFFFFFF);}
      	}
      	break;

    	case 8:	// TDC data
      	evt->Edge((word>>16)&0xFF, (word>>0)&0xFFFF,(word>>26)&0x1); 
				edgec++;	
      	// edge = 0 //rising,
      	// edge = 1 // falling
      	e = (word>>26)&0x1;
      	if (e==0) {
					fposedge[(word>>16)&0xFF]++;
      	}
      	if (lv==2) {printf("0x%08X [TDC HIT] EDGE=%d, CH=%d, TIME=%d\n",word, (word>>26)&0x1,(word>>16)&0xFF, (word>>0)&0xFFFF);}
      	if (lv==3) {printf("\tEDGE=%d, CH=%d, TIME=%d\n", (word>>26)&0x1,(word>>16)&0xFF, (word>>0)&0xFFFF);}
      	break;


    	case 14:if (lv==2) {printf("0x%08X [DNV]",word);}break;
    	case 15:if (lv==2) {printf("0x%08X [FILLER]",word);}break;
    	default:if (lv==2) {printf("0x%08X [UNKNOWN]",word);}break;
    }
  }
   
  if (evt) {
		// last event is not Processed 
		//evtid--; // data about last event have been read but they are negliged in the following analysys

		evt->Write(); 
    delete evt;
  }


  fin.close();
  printf("\n");
	printf("File %s (%10d events, %10d edges, %3.0lf average occupancy)",fioTDCroot.c_str(),evtid,edgec,0.5*edgec/evtid);

	// Read SUMMARY:
	// to be matched with what is done in evt.Process->() ...
	// valid events
	// skipped
	// total events

 	if(i>=maxword){ printf("*");}

//	Note : last eventid is the total number of events only if evt->Process start from 1! (IMPORTANT FOR NORMALIZATION)

  if(zword){
		printf("ZWORDS %8d ",zword);
	} // in case of non blocking socket not managed in DAQ at the moment of outfile print.
  //printf("\n");
  
  //}

	// remove .txt (temporary) file 
	if( remove( fintxt.c_str() ) != 0 ){
		perror( "Error deleting file" );
	}else{
		//puts( "File successfully deleted" );
	}
	printf("\n");
  return evtid; 
}


void TRich_Analysis::TDC_Draw(TH1F * hRise,TH1F * hFall){
  
  if(hRise->GetEntries()!=0&&hFall->GetEntries()!=0){
    
    double ymax = 1000000.;
    double ymin = 0.1;	
    
    gStyle->SetOptStat(111111);	// includes undeflows and overflows
    
    hRise->GetXaxis()->SetTitle("Time Window [ns]");
    hRise->GetYaxis()->SetTitle("Occourrency [#]");
    hRise->SetMaximum(ymax);
    hRise->SetMinimum(ymin);	
    hRise->Draw();
    
    gPad->Update(); 
    
    TPaveStats *tpsRise = (TPaveStats*) hRise->FindObject("stats");
    tpsRise->SetTextColor(kBlue);
    tpsRise->SetLineColor(kBlue);
    double X1 = tpsRise->GetX1NDC();
    double Y1 = tpsRise->GetY1NDC();
    double X2 = tpsRise->GetX2NDC();
    double Y2 = tpsRise->GetY2NDC();
    
    hFall->SetLineColor(kRed);
    hFall->Draw();
    
    gPad->Update(); 
    
    TPaveStats * tpsFall = (TPaveStats*) hFall->FindObject("stats");
    tpsFall->SetTextColor(kRed);
    tpsFall->SetLineColor(kRed);
    tpsFall->SetX1NDC(X1);
    tpsFall->SetX2NDC(X2);
    tpsFall->SetY1NDC(Y1-(Y2-Y1));
    tpsFall->SetY2NDC(Y1);
    
    // Drawing 
		/*
    TCanvas * lmycanv = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");
    if(!lmycanv){printf("Canvas not found...\n");}
    
    lmycanv->cd(1)->SetLogy(1);
    lmycanv->cd(1)->SetGrid(1);
    */
    hRise->Draw();
    hFall->Draw("same");
    tpsRise->Draw("same");
    tpsFall->Draw("same");
    //lmycanv->Print(fTDCpdf.c_str());
  }
}

void TRich_Analysis::Plot(){

	std::string infile  = HISTOPATH + frun_name + "_histo" + ".root";
	std::string outfile = PDFPATH   + frun_name + "_global" + ".pdf";
	
	const char * p =  outfile.c_str();
	
	TCanvas * c = new TCanvas("c");
	
	c->Print(Form("%s[",p)); 

	TFile f(infile.c_str(),"READ"); 
	
	TH1F * h = NULL;
	h = (TH1F*) f.Get(HEVTTIME); 
	if(h!=NULL){h->Draw(); c->Print(Form("%s",p)); h=NULL;}
	
	h = (TH1F*) f.Get(HEVTFREQ); 
	if(h!=NULL){h->Draw(); c->SetLogx(1);c->Print(Form("%s",p)); h=NULL;}
	
	h = (TH1F*) f.Get(HEVTEDGE); 
	if(h!=NULL){h->Draw(); c->SetLogx(0);c->Print(Form("%s",p)); h=NULL;}
	
	h = (TH1F*) f.Get(H_NEDGES); 
	if(h!=NULL){h->Draw(); c->SetLogx(0);c->Print(Form("%s",p)); h=NULL;}
	
	h = (TH1F*) f.Get(HBRDRISE); 
	if(h!=NULL){h->Draw(); c->SetLogy(1);c->Print(Form("%s",p)); h=NULL;}
	
	h = (TH1F*) f.Get(HBRDFALL); 
	if(h!=NULL){h->Draw(); c->SetLogy(1);c->Print(Form("%s",p)); h=NULL;}

	c->Print(Form("%s]",p)); 

	f.Close();	
	delete c;

	printf("Plots File %s\n ",p);
}



// Process parsed data (.root), creates histograms and save them in (_histo.root)

// First loop is for fixed x-range plots while the  second loop variable range ones (e.g. edges distribution) 

/*
* trigger time vs event ID
* trigger frequency (distribution)
* number of edges vs event ID
* number of edges (distribtion)
* rise time distributino
* fall time distribution
*/

void 	TRich_Analysis::TDC_Spectra(){


	
	std::string outfile  = HISTOPATH + frun_name + "_histo" + ".root";

	unsigned int event, nedge, channel, polarity, time,nedge_max=0;
	double etime, dt, etime_prev=0.0;
	bool dbg= false;
	bool ok;
	int entries;
  unsigned int lwindow	= this->GetEventBuilderWindow(); // clock ticks [8ns]
  int timmax = 8*lwindow; // ns
  int timmin = 0;
	
	TRich_TDC * tdc = new TRich_TDC();
	ok = tdc->OpenFile(fioTDCroot.c_str()); 
	if(!ok) return;
	entries = tdc->RetrieveTree();

	TFile * out_file 	= new TFile(outfile.c_str(),"RECREATE"); 
	
  TH1F * trigger 	= new TH1F(HEVTTIME,"",entries			,-0.5				,entries-0.5);
	TH1F * frequency= new TH1F(HEVTFREQ,"",1E6		 			,-0.5				,	1E6-0.5);	
  TH1F * edge 	 	= new TH1F(HEVTEDGE,"",entries			,-0.5				,entries-0.5);
  TH1F * rise 		= new TH1F(HBRDRISE,"",timmax-timmin,timmin-0.5	, timmax-0.5);
  TH1F * fall 		= new TH1F(HBRDFALL,"",timmax-timmin,timmin-0.5	, timmax-0.5);

	for(int e=0; e<entries; e++){
		//dbg = (e%(entries/10)==0) ? true : false;
		tdc->Reset();
		tdc->GetEntry(e);// event ID (header data) + edge list
		event = tdc->TrigNum();
		etime = tdc->DecodeTimeStamp();	
		nedge = tdc->NEdges();	
		nedge_max = ( nedge > nedge_max )? nedge : nedge_max;
		if(dbg) printf("Event ID = %u Time = %13.9lf Edge = %4d \n",event,etime,nedge);
		trigger->Fill(event,etime);
		edge->Fill(event,nedge);
   	dt = etime-etime_prev;
    etime_prev = etime;
		if(dt>=0.)frequency->Fill(1/dt);
		for(unsigned int edge=0;edge<nedge;edge++){ // edge list
			channel 	= tdc->Channel(edge);
			polarity 	= tdc->Polarity(edge);
			time 			= tdc->Time( edge);
		 	switch (polarity) {
      	case 0:rise->Fill(time);break;
      	case 1:fall->Fill(time);break;
      	default: printf("Error in %s: unknown edge polarity %d\n",__FUNCTION__,polarity);break;
      }
			//if(e<5) printf("CH=%4u POL=%u TIME%5u\n",channel, polarity,time);
		}
	}
	trigger->Write();
	frequency->Write();
	edge->Write();
	rise->Write();
	fall->Write();
	delete edge;
	delete trigger;
	delete frequency;
	delete rise;
	delete fall;

	// second loop is for adjustable x-axis range
	//	printf("Max edges = %u, take %d\n",nedge_max,(int)(nedge_max*1.1));
  int nbin = nedge_max*2.0;
  TH1F * edgeD = new TH1F(H_NEDGES,"",nbin,-0.5,nbin-0.5);
	for(int e=0; e<entries; e++){
	//	dbg = (e%(entries/10)==0) ? true : false;
		tdc->Reset();
		tdc->GetEntry(e);		
		nedge = tdc->NEdges();	
		edgeD->Fill(nedge);
		if(dbg)printf("Edge = %4d \n",nedge);
	}
	edgeD->Write();
	delete edgeD;

  out_file->Close();
	delete out_file;
	delete tdc;
	printf("Histograms saved in %s\n",outfile.c_str());
}

void 	TRich_Analysis::TDC_SpectraSingleChannel(){


	std::string outfile  = HISTOPATH + frun_name + "_histosingle" + ".root";

	unsigned int nedge, channel, polarity, time,channel2, polarity2, time2;
	//bool dbg;
	bool ok;
	int entries,duration;
  unsigned int lwindow	= this->GetEventBuilderWindow(); // clock ticks [8ns]
  int t2 = 8*lwindow; // ns
  int t1 = 0;
  
  int d1 = -200;
  int d2 = 200;
  //int polar_discri = this->GetPolarDiscri();
  //printf("Polar Discri = %u\n",polar_discri);// =1 LEADING EDGE is the falling

	TRich_TDC * tdc = new TRich_TDC();
	ok = tdc->OpenFile(fioTDCroot.c_str());  if(!ok) return;
	entries = tdc->RetrieveTree();

	TFile * out_file 	= new TFile(outfile.c_str(),"RECREATE"); 
 	
 	int nchannels = 192;
 	
 	TH1F * rise[nchannels];	
  TH1F * fall[nchannels];
  TH1F * dura[nchannels];
	TH2F * walk[nchannels];

	for (int i=0; i<nchannels; i++) {
		rise[i] = new TH1F(Form("ch%03d_rise",i),"",t2-t1,t1-0.5,t2-0.5);
    fall[i] = new TH1F(Form("ch%03d_fall",i),"",t2-t1,t1-0.5,t2-0.5);
    dura[i] = new TH1F(Form("ch%03d_dura",i),"",d2-d1,d1-0.5,d2-0.5);
		walk[i] = new TH2F(Form("ch%03d_walk",i),"",d2-d1,d1-0.5,d2-0.5,t2-t1,t1-0.5,t2-0.5);
  }
  
	for(int e=0; e<entries; e++){
		//dbg = (e%(entries/10)==0) ? true : false;
		tdc->Reset();
		tdc->GetEntry(e);// event ID (header data) + edge list
		nedge = tdc->NEdges();	
		for(unsigned int edge=0;edge<nedge;edge++){ // edge list
			channel 	= tdc->Channel(edge);
			polarity 	= tdc->Polarity(edge);
			time 			= tdc->Time( edge);
		 	switch (polarity) {
      	case 0:rise[channel]->Fill(time);break;
      	case 1:fall[channel]->Fill(time);break;
      	default: printf("Error in %s: unknown edge polarity %d\n",__FUNCTION__,polarity);break;
      }
      // hit reconstruction
      duration=0;
      // scan the edge list looking for an edge of opposite polarity for the same channel and calculate the time over threshold 
      for (unsigned int j=edge+1; j<nedge; j++){
      	channel2 = tdc->Channel(j);
      	if(channel2==channel){
      		polarity2= tdc->Polarity(j);
      		if(polarity2!=polarity){
      			time2 = tdc->Time(j);
      			duration = time2-time;//to b improved,add checks t1>t2, and use polar discri
						dura[channel]->Fill(duration);
						walk[channel]->Fill(duration,time);
			//		printf("duraiton x %d, y time %u \n",duration , time);
      		}
      	}
      }
		}
	}
	for (int i=0; i<nchannels; i++) {
	
		rise[i]->Write();
		fall[i]->Write();
		dura[i]->Write();
		walk[i]->Write();
		delete rise[i];
		delete fall[i];
		delete dura[i];	
		delete walk[i];	
  }
  
  out_file->Close();
	delete out_file;
	delete tdc;
	printf("Histograms saved in %s\n",outfile.c_str());
}


void 	 TRich_Analysis::TDC_Export(){
	
	this->TDC_GetSingleChannel(0);
}

void 	 TRich_Analysis::TDC_Uniformity(){

	this->TDC_GetSingleChannel(1);

}
void 	 TRich_Analysis::TDC_Image(){

	this->TDC_GetSingleChannel(2);

}


void TRich_Analysis::TDC_GetSingleChannel(int opt){


					gStyle->SetOptStat(0);	
	std::string infile  = HISTOPATH + frun_name + "_histosingle" + ".root";
	
	std::string outfile;
	FILE * txt;
	int nhisto = 9;
	TH1F * hch[nhisto]; // Uniformity histos 
	TH2F * hpx[nhisto]; // 2D representation Pixels

	// 0 - Rise entries vs channel
	// 1 - Fall entries vs channel
	// 2 - Rise delay vs channel
	// 3 - Fall delay vs channel
	// 4 - Rise tts vs channel
	// 5 - Fall tts vs channel	
	// 6 - Duration Entries vs channel 
	// 7 - Duration Mean vs channel
	// 8 - Duration RMS vs channel
	
TCanvas * mycanv;


	TRich_Adapter adapter;
	

	int pixel;
	int pixel_offset;

//	bool pixel_enable = true; // anodic view instead of electromnic channel
//	bool geo_enable = true; // mapmt view 2 D

	int Xpix_offset;
	int Ypix;
	int Xpix;


	switch(opt){
		case TEXT:  
			outfile = TXTPATH + frun_name + "_statistics" + ".txt";
			txt = fopen(outfile.c_str(),"w");
			break;

		case UNIF: 
			outfile = PDFPATH + frun_name + "_uniformity" + ".pdf";
			mycanv = new TCanvas("mycanv");
			mycanv->Print(Form("%s[",outfile.c_str()));
			mycanv->cd(1)->SetLogy(0);
  		mycanv->cd(1)->SetLogx(0);
 			mycanv->cd(1)->SetGrid(1);
			for (int i=0; i<nhisto; i++) hch[i] = new TH1F("","",192,0-.5,192-.5);
			break;
		
		case PIXEL: 
			outfile = PDFPATH + frun_name + "_image" + ".pdf";
			mycanv = new TCanvas("mycanv");
			mycanv->Print(Form("%s[",outfile.c_str()));
			mycanv->cd(1)->SetLogy(0);
  		mycanv->cd(1)->SetLogx(0);
 			mycanv->cd(1)->SetGrid(1);
			for (int i=0; i<nhisto; i++) hpx[i] = new TH2F("","",30,0,30,8,0,8);

			break;
		default: printf("Error in %s: Unknown option",__FUNCTION__);break;
	} 

	// READ settings from logbook
	TRich_Config cfg;
	cfg.ParseFile(finlog.c_str());
  cfg.Read();
	int thr = cfg.Threshold();
	int gain = this->GetGain();
	int totEv = cfg.GetLogNEvents();
	//printf("THR %d GAIN %d NEvents %d Option %d\n",thr,gain ,totEv,opt);


	// Get Single channel histograms from .root file
	TFile f(infile.c_str(),"READ"); 
	
	TH1F * hrise = NULL;
	TH1F * hfall = NULL;
	TH1F * hdura = NULL;
	
	int nchannels = 192;
		
	const int k = 3; 
	double counts[k]; 
	double mean[k];
	double rms[k];		
		
	// Loop on channels and extract number of hit (entries), average delay (TDC mean), jitter (TDC rms)	
	for(int i =0; i<nchannels; i++){
		
		hrise = (TH1F*) f.Get(Form("ch%03d_rise",i)); 
		hfall = (TH1F*) f.Get(Form("ch%03d_fall",i)); 
		hdura = (TH1F*) f.Get(Form("ch%03d_dura",i)); 

		if(hfall!=NULL&&hrise!=NULL){

			counts[0] = hfall->GetEntries();
			mean[0] = hfall->GetMean();
			rms[0] = hfall->GetRMS();
	
			counts[1] = hrise->GetEntries();
			mean[1] = hrise->GetMean();
			rms[1] = hrise->GetRMS();

			counts[2] = hdura->GetEntries();
			mean[2] = hdura->GetMean();
			rms[2] = hdura->GetRMS();

			switch(opt){
				case TEXT: 

						fprintf(txt,"CH %3d ",i);
						fprintf(txt,"Entries: %.0lf %.0lf %.0lf ",counts[0],counts[1],counts[2]);
						fprintf(txt,"Mean : %.3lf %.3lf %.3lf ",mean[0],mean[1],mean[2]);				
						fprintf(txt,"RMS : %.3lf %.3lf %.3lf ",rms[0],rms[1],rms[2]);
						fprintf(txt,"\n");

						fprintf(stderr,"CH %3d ",i);
						fprintf(stderr,"Entries: %.0lf %.0lf %.0lf ",counts[0],counts[1],counts[2]);
						fprintf(stderr,"Mean : %.3lf %.3lf %.3lf ",mean[0],mean[1],mean[2]);				
						fprintf(stderr,"RMS : %.3lf %.3lf %.3lf ",rms[0],rms[1],rms[2]);
						fprintf(stderr,"\n");
						/*
						printf("CH %3d ",i);
						printf("N %.0lf MEAN %.3lf RMS %.3lf ",counts[0],mean[0],rms[0]);
						printf("N %.0lf MEAN %.3lf RMS %.3lf ",counts[1],mean[1],rms[1]);				
						printf("N %.0lf MEAN %.3lf RMS %.3lf ",counts[2],mean[2],rms[2]);
						printf("\n");	
					
						*/

				
					break;
				case UNIF: 
						hch[0]->Fill(i,counts[1]);		
						hch[1]->Fill(i,counts[0]);
						hch[2]->Fill(i,mean[1]);
						hch[3]->Fill(i,mean[0]);			
						hch[4]->Fill(i,rms[1]);
						hch[5]->Fill(i,rms[0]);
						hch[6]->Fill(i,counts[2]);
						hch[7]->Fill(i,mean[2]);
						hch[8]->Fill(i,rms[2]);
				
					break;
				case PIXEL: 
					pixel = adapter.GetAnode(i%64);
					pixel_offset = (i/64)*64;
				//	printf("channel %d -> anode %d ",i,pixel);
				//	printf("asic %d ->offset %d \n",i/64,pixel_offset)	;

					Xpix_offset = i/64*8;
					Ypix =(pixel-1)/8;
					Xpix = (pixel-1)%8;

				//	printf("channel %3d pixel %2d Y %d X %d (%2d mapmt %d)\n",i,pixel,Ypix,Xpix,Xpix+Xpix_offset,i/64);
		
					hpx[0]->Fill(Xpix+Xpix_offset,Ypix,counts[1]/totEv);		
					hpx[1]->Fill(Xpix+Xpix_offset,Ypix,counts[0]/totEv);
					hpx[2]->Fill(Xpix+Xpix_offset,Ypix,mean[1]);
					hpx[3]->Fill(Xpix+Xpix_offset,Ypix,mean[0]);			
					hpx[4]->Fill(Xpix+Xpix_offset,Ypix,rms[1]);
					hpx[5]->Fill(Xpix+Xpix_offset,Ypix,rms[0]);
					hpx[6]->Fill(Xpix+Xpix_offset,Ypix,counts[1]/totEv);
					hpx[7]->Fill(Xpix+Xpix_offset,Ypix,mean[2]);
					hpx[8]->Fill(Xpix+Xpix_offset,Ypix,rms[2]);



					break;
				default: printf("Error in %s: Unknown option",__FUNCTION__);break;
			}			
		}
	}
	f.Close();	

// Finalize
	switch(opt){
				case TEXT: 
					fclose(txt);
					printf("Single Channel Statistics in %s\n",outfile.c_str());
					break;

				case UNIF: 
					for (int i=0; i<nhisto; i++) {
					//	hch[i]->SetStats(0);
						if(i%2==1&&i<6)hch[i]->SetLineColor(kRed);
						if(i>=6)hch[i]->SetLineColor(kMagenta);
						hch[i]->GetXaxis()->SetTitle("Channel [0..191]"); // electronics view
				//		hch[i]->GetXaxis()->SetTitle("Pixel [0..63][0..63][0..63]"); // anodic view
					}			

					// Efficiency
 					//hch[0]->SetMaximum(2.);
 					//hch[0]->SetMinimum();
					hch[0]->Scale(1./totEv);
					hch[1]->Scale(1./totEv);
					hch[0]->GetYaxis()->SetTitle("N Edges /Tot Events");
  				hch[0]->SetTitle("Efficiency");
					hch[0]->Draw();
					hch[1]->Draw("SAME");
					mycanv->Print(Form("%s",outfile.c_str()));
					
					// Mean Delay
					//	hch[2]->SetMaximum(timmax-timmin);
					hch[2]->GetYaxis()->SetTitle("Edge Time Average  [ns]");
  				hch[2]->SetTitle("Time Delay");
					hch[2]->Draw();
					hch[3]->Draw("SAME");
					mycanv->Print(Form("%s",outfile.c_str()));
	
					// Jitter
					//	hch[4]->SetMaximum(5);
					hch[4]->GetYaxis()->SetTitle("Edge Time RMS [ns]");
  				hch[4]->SetTitle("Time Resolution");
					hch[4]->Draw();
					hch[5]->Draw("SAME");
					mycanv->Print(Form("%s",outfile.c_str()));
  	

					// Efficiency Duration
				// 	hch[6]->SetMaximum(2.0);
					hch[6]->Scale(1./totEv);
					hch[6]->GetYaxis()->SetTitle("N Durations /Tot Events");
				  hch[6]->SetTitle("Hit Efficiency");
					hch[6]->Draw();
					mycanv->Print(Form("%s",outfile.c_str()));
	
					// Mean Duration
					//	hch[7]->SetMaximum(timmax-timmin);
					hch[7]->GetYaxis()->SetTitle("Mean Duration [ns]");
  				hch[7]->SetTitle("Time over Threshold ");
					hch[7]->Draw();
					mycanv->Print(Form("%s",outfile.c_str()));
	
					// Duration rms
					//hch[8]->SetMaximum(5);
					hch[8]->GetYaxis()->SetTitle("RMS Duration[ns]");
  				hch[8]->SetTitle("Time over Threshold RMS");
					hch[8]->Draw();
					mycanv->Print(Form("%s",outfile.c_str()));

					mycanv->Print(Form("%s]",outfile.c_str()));
					for (int i=0; i<nhisto; i++) if(hch[i])delete hch[i];
					delete mycanv;
					printf("Uniformity Plots in %s\n",outfile.c_str());
					break;

				case PIXEL: 
					hpx[0]->SetTitle("Efficiency Rise");
					hpx[1]->SetTitle("Efficiency Fall");
					hpx[2]->SetTitle("Rise Time");
					hpx[3]->SetTitle("Fall Time");
					hpx[4]->SetTitle("Jitter Rise");
					hpx[5]->SetTitle("Jitter Fall");
					hpx[6]->SetTitle("Efficiency Time Over Threshold");
					hpx[7]->SetTitle("Time Over THreshold (Charge)");
					hpx[8]->SetTitle("Tot REsolution (Charge RMS)");

					hpx[0]->SetMaximum(2.);
					hpx[1]->SetMaximum(2.);
					hpx[6]->SetMaximum(2.);

/* TO BE IMPROVED, time windowing is is correlated with calibration and daq rate...	
				hpx[2]->SetMaximum(350);
					hpx[2]->SetMinimum(150);
					hpx[3]->SetMaximum(350);
					hpx[3]->SetMinimum(150);
	*/					
					gStyle->SetOptStat(0);	 
					for (int i=0; i<nhisto; i++) {
						hpx[i]->Draw("COLZ");
						mycanv->Print(Form("%s",outfile.c_str()));
					}
					mycanv->Print(Form("%s]",outfile.c_str()));
					for (int i=0; i<nhisto; i++) if(hch[i])delete hpx[i];
					delete mycanv;
					printf("MAPMT images in %s\n",outfile.c_str());
					break;
				default: printf("Error in %s: Unknown option",__FUNCTION__);break;
	}

}

void TRich_Analysis::TDC_PlotSingleChannel(){

	std::string infile  = HISTOPATH + frun_name + "_histosingle" + ".root";
	std::string outfile = PDFPATH   + frun_name + "_single" + ".pdf";
	const char * p =  outfile.c_str();
	const char * canvname = "c";
	int rows = 1;
	int columns = 3;
	int padsize = 200;
	int padmargin = 10.; //%	
	int sizex = columns *padsize;
	int sizey = rows * padsize;
	TCanvas * c = new TCanvas(canvname,"",sizex,sizey);
	c->Print(Form("%s[",p)); 
	c->Divide(columns,rows);
	const int NPAD = columns*rows;
	TPad * pad[NPAD];
	float padm = padmargin/100.; 	
	for(int p=0;p<NPAD;p++){
		pad[p]=(TPad*)(c->FindObject(Form("%s_%d",canvname,p+1)));	
		pad[p]->cd();
  	pad[p]->SetGrid(kTRUE);
		pad[p]->SetLogx(kFALSE);
		pad[p]->SetLogy(kFALSE);
		pad[p]->SetMargin(padm,padm,padm,padm); // left, right, bottom, top
	}

	TFile f(infile.c_str(),"READ"); 
	
	TH1F * hrise = NULL;
	TH1F * hfall = NULL;
	TH1F * hdura = NULL;
	TH2F * hwalk = NULL;
	
	int nchannels = 192;
	double max = 10.0E6;
	double min = 0.1;
	
	for(int i =0; i<nchannels; i++){
		
		hrise = (TH1F*) f.Get(Form("ch%03d_rise",i)); 
		hfall = (TH1F*) f.Get(Form("ch%03d_fall",i)); 
		pad[0]->cd()->SetLogy(1);		


		this->TDC_Draw(hrise,hfall);
/*
		if(hfall!=NULL&&hrise!=NULL){
			hfall->SetLineColor(kRed);
			hrise->SetLineColor(kBlue);		
			hfall->SetMaximum(max);
			hfall->SetMinimum(min);
			hfall->Draw();
			hrise->Draw("SAME");
		}
	*/	
		hdura = (TH1F*) f.Get(Form("ch%03d_dura",i)); 
		pad[1]->cd()->SetLogy(1);	
		if(hdura!=NULL){
			hdura->SetMaximum(max);
			hdura->SetMinimum(min);
			hdura->SetLineColor(kMagenta);		
			hdura->Draw();
		}

		hwalk = (TH2F*) f.Get(Form("ch%03d_walk",i)); 
		pad[2]->cd()->SetLogy(0);	
		if(hwalk!=NULL){
//			hwalk->SetMaximum(max);
	//		hwalk->SetMinimum(min);
		//	hwalk->SetLineColor(kMagenta);		
			hwalk->Draw();
		}
		

		c->Print(Form("%s",p)); 
	}
	c->Print(Form("%s]",p)); 
	f.Close();	
	delete c;
	printf("Single Channel Time Spectra in %s\n ",p);
}






void	TRich_Analysis::TDC_Plot(int asic,bool draw_single_channel){	

	TFile * lfile = new TFile(fioTDCroot.c_str());//if(lfile->IsZombie()){printf("pFile is Zombie!\n");}
	TTree * ltree = (TTree*)lfile->Get(TREE_TDC);
	TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
	TCanvas* mycanv = new TCanvas("mycanv","",1280-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
	UInt_t		lEventID	= 0; // trig num i.e. event id
	UShort_t	ltrise[192];  // delay from trigger, in ns, rising edge   
	UShort_t	ltfall[192]; // // delay from trigger, in ns, rising edge
	for (int i =0; i<192; i++) {ltrise[i] = 0;ltfall[i] = 0;}
	ltree->SetBranchAddress("triggerID",&lEventID);
	ltree->SetBranchAddress("trise",ltrise);
	ltree->SetBranchAddress("tfall",ltfall);
	int nrise_miss = 0;
	int nfall_miss = 0;
	TH1F * hRise[192];for (int i=0; i<192; i++) {hRise[i] = new TH1F(Form("hRise%d",i),Form(" Channel %d Time Distribution",i),1024,-0.5,1023.5);}
	TH1F * hFall[192];for (int i=0; i<192; i++) {hFall[i] = new TH1F(Form("hFall%d",i),Form(" Channel %d Time Distribution",i),1024,-0.5,1023.5);}
	int totEvts = ltree->GetEntries();//printf("entries = %d\n",totEvts);
	for(int e = 0;e<totEvts;e++){
		lEventID	= 0; for (int i =0; i<192; i++) {ltrise[i] = 0;ltfall[i] = 0;}
		ltree->GetEntry(e);
		int nrise = 0;
		int nfall = 0;
		// look for recorded edges 
		for (int i =0; i<192; i++) { 
			if (ltrise[i]!=0) {nrise++;hRise[i]->Fill(ltrise[i]);}
			if (ltfall[i]!=0) {nfall++;hFall[i]->Fill(ltfall[i]);}
		}		
		// check: digital pulse (hit) must have both edges
		if (nrise!=nfall) {
			if (nrise>nfall) {nfall_miss++;}else {	nrise_miss++;}
		}
	}	
	
	float rise_missing = ((float)nrise_miss)/totEvts;if (rise_missing>0) {printf("missing %d rising  edges %.4f %%\n",nrise_miss,rise_missing);}
	float fall_missing = ((float)nfall_miss)/totEvts;if (fall_missing>0) {printf("missing %d falling edges %.4f %%\n",nfall_miss,fall_missing);}

	
	char nomerun[256];
	if (draw_single_channel) {
		printf("Drawing...\n");
	}
	for (int i =0; i<192; i++) {
		if ((i/64)==asic) { // 
			if(hRise[i]->GetEntries()!=0){
				hRise[i]->GetXaxis()->SetTitle("Time Window [ns]");
				hRise[i]->GetYaxis()->SetTitle("Occourrency [#]");
				hRise[i]->SetMaximum(totEvts);
				hRise[i]->SetMinimum(0.1);	
				hRise[i]->Draw();
				gPad->Update(); 
				TPaveStats *tps1 = (TPaveStats*) hRise[i]->FindObject("stats");
				tps1->SetName("hRise Stats");
				double X1 = tps1->GetX1NDC();
				double Y1 = tps1->GetY1NDC();
				double X2 = tps1->GetX2NDC();
				double Y2 = tps1->GetY2NDC();
				hFall[i]->SetLineColor(kRed);
				hFall[i]->Draw();
				gPad->Update(); 
				TPaveStats *tps2 = (TPaveStats*) hFall[i]->FindObject("stats");
				tps2->SetTextColor(kRed);
				tps2->SetLineColor(kRed);
				tps2->SetX1NDC(X1);
				tps2->SetX2NDC(X2);
				tps2->SetY1NDC(Y1-(Y2-Y1));
				tps2->SetY2NDC(Y1);
				sprintf(nomerun,"../eps/TDC_asic%d.gif+1",asic); 
				mycanv->cd(1)->SetLogy(1);
				mycanv->cd(1)->SetGrid(1);
				/* Draw all (two histograms and their stat boxes in one canvas */
				//TCanvas *c3 = new TCanvas();
				hRise[i]->Draw();
				hFall[i]->Draw("same");
				tps1->Draw("same");
				tps2->Draw("same");
				// the following line is time consuming, better to put a flag: enable_draw_individual_channel_time_spectra 
				if (draw_single_channel) {
					mycanv->Print(nomerun,".gif+1");
				}
			} 
		}
	}

	delete mycanv;
	
	
	
	TCanvas *old2 = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv2");if(old2){delete old2;}
	TCanvas* mycanv2 = new TCanvas("mycanv2","",1280-2*SIZEOFCANV,0,2*SIZEOFCANV,3*SIZEOFCANV);
	mycanv2->Divide(1,3);
	
	double nhit;
	double avg;
	double rms;
	
	int totchannels = 64;
	
	TH1F * heff = new TH1F("heff",Form("Channel Efficiency ASIC %d",asic),totchannels,-0.5,totchannels-0.5);
	TH1F * hm = new TH1F("hm",Form("Average Rising Time - ASIC %d",asic),totchannels,-0.5,totchannels-0.5);	
	TH1F * hn = new TH1F("hn",Form("Rising Time Fluctuation - ASIC %d",asic),totchannels,-0.5,totchannels-0.5);	
	
	for (int i =0 ; i<64; i++) {
		
		nhit = hRise[i+64*asic]->GetEntries();
		avg  = hRise[i+64*asic]->GetMean();
		rms  = hRise[i+64*asic]->GetRMS();
		
		heff->Fill(i,nhit/totEvts);
		hm->Fill(i,avg);
		hn->Fill(i,rms);
		
		/*
		if (i%16==0) {printf("channel  Hit   Efficiency  Mean [ns] RMS [ns]\n");}
		printf(" %2d %2d",i/64,i%64);
		printf("%6.0lf   %6.6lf",nhit,1*(nhit/totEvts));
		printf("%7.0lf %8.1lf",avg,rms);
		printf("\n");*/
		
	}
	heff->SetMarkerColor(2);
	heff->SetMarkerStyle(10);
	heff->GetXaxis()->SetTitle("ChannelID [#]");
	heff->GetYaxis()->SetTitle("Hit [#]/Trig [#] ");
	
	hm->SetMarkerColor(1);
	hm->SetMarkerStyle(10);
	hm->GetXaxis()->SetTitle("ChannelID [#]");	
	hm->GetYaxis()->SetTitle("Time Mean [ns]");
	
	hn->SetMarkerColor(4);
	hn->SetMarkerStyle(10);
	hn->GetXaxis()->SetTitle("ChannelID [#]");
	hn->GetYaxis()->SetTitle("Time RMS [ns]");
	
	mycanv2->cd(1);
	heff->Draw("P");
	mycanv2->cd(2);
	hm->Draw("P");
	mycanv2->cd(3);
	hn->Draw("P");
	
	heff->SetStats(0);
	hm->SetStats(0);
	hn->SetStats(0);
	//gStyle->SetOptStat(0);
	mycanv2->Print(Form("../eps/TDCreport_asic%d.eps",asic));	
	mycanv2->Update();
	

	delete mycanv2;
	for (int i =0; i<192; i++) { 
		if(hRise[i]!=NULL){delete hRise[i];} 
		if(hFall[i]!=NULL){delete hFall[i];} 	
	}

	
	lfile->Close();	
	delete lfile;
}




double 	TRich_Analysis::DecodeTimeStamp(UInt_t tstamp0,UInt_t tstamp1){

	//printf("\n");	
	//printf(" TIME[1] =%" PRIu64 " \n",tstamp1);
	//printf(" TIME[0] =%" PRIu64 " \n",tstamp0);
	ULong64_t timestamp = (tstamp1<<24) + tstamp0;	// in clock ticks					
	//printf(" TIME =%" PRIu64 " ",timestamp);
	ULong64_t timestamp_ns = timestamp<<3; // 8 ns clock
	//printf(" %" PRIu64 "[ns]",timestamp_ns);
	double timestamp_s = timestamp_ns/1000000000.;	// expressed in second
	//printf(" TIME = %14.10lf [s]",timestamp_s); 
	//printf(" PREV TIME= %14.10lf [s]\n",timestamp_s_old);	
	//printf(" DT = %14.10lf ",timestamp_s-timestamp_s_old);
	//printf(" freq[Hz] %14.10lf\n",1/(timestamp_s-timestamp_s_old));
	//timestamp_s_old =  timestamp_s;
	return timestamp_s;
}






/**********/
/* SCALER */
/**********/
TRich_Scaler *	TRich_Analysis::SKA_Read(){

	// open file 
	ifstream fin(finraw.c_str());
	std::string lstring;
	TRich_Scaler *  lska = new TRich_Scaler();
	
	// the following lines parse the (temporary) format for scaler acquisition
	// which is something like VARIABLE_NAME= number
	// the variable name is read as a string and discarded
	// the number is written on the TRich_Scaler data structure
	// the function returns the address of a TRich_Scaler
	int val;
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->GClk125(val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Sync(val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Trig(val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Input(0,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Input(1,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Input(2,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Output(0,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Output(1,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Output(2,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Or0(0,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Or0(1,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Or0(2,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Or1(0,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Or1(1,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Or1(2,val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->Busy(val);
	for (int i=0; i<2; i++) {fin>>lstring;}fin>>val;lska->BusyCycles(val);
	
	int skip;
	int channel;
	for (int ch =0; ch<64; ch++) {
		if (ch<10) {skip=3;}else{skip=2;} // to be changed in daq the number of string to skip
		for (int asic=0; asic<3; asic++) {
			for (int i=0; i<skip; i++) {
				fin>>lstring;//printf("STRING[%d] %s\n",i,lstring.c_str());
			}
			channel = ch+64*asic;
			fin>>val; 
			lska->Scaler(channel,val);
		//printf("MAROC[%d] Channel[%d] =  %4d (local channel = %d)\n",asic,ch,linteger,ch+64*asic);
		}
	}
	return lska;
}


/*
 * Create a Ttree with threhsold[3], gain[192] and scaler[192] counts by looping
 * on a set of consecutive files (txt and config)
 */ 
int TRich_Analysis::SKA_Read_Threshold_Scan(){

  bool p = true; // print enable for debug purposes
  // use string compare to check data private data members are initialized and that repetition is >=0
  // to be done  
  unsigned int thr;
  unsigned int scaler[192];		
  TFile	* file = new TFile(fioSKAroot_Scan.c_str(),"RECREATE");
  TTree	* tree = new TTree(TREE_SCALER_THR,"Tree of Data for Analysis");	
  tree->Branch("Threshold",&thr,"Threshold/i");
  tree->Branch("Scaler",scaler,"Scaler[192]/i");
	
  // loop on runs
  std::string name;
  char numstr[7]; // enough to hold all run numbers and the '_' underscore (6 decimal digits)
  
  int i;

  for (i=frunID_first; i<=frunID_last; i++) {

    // compose the filename e.g. ../data/run_00123.tXt
    name.clear(); 
    name+= frun_path.c_str();
    name+= frun_prefix.c_str();		
    sprintf(numstr, "%06d", i); //		sprintf(numstr, "_%06d", i);
    name += numstr;
    name += ".txt";


    this->NameRun(name.c_str()); 
    if(p)printf("name = %s\n",name.c_str()); 
    
    thr=0;for (int j=0; j<192; j++) {scaler[j] =0;}	// reset variables associated with the TTree

    TRich_Config * cfg = new TRich_Config();
    bool success = cfg->ParseFile(finlog.c_str());
    if (success) {// the db is ready for access
      if(p)printf("Log File Opened: %s\n",finlog.c_str());
      cfg->Read(); // import configuration data
      if(p)printf("Log File Read\n");

	printf("Run ID first %d, Run ID last %d\n",frunID_first,frunID_last);

      // import logbook information
      thr = cfg->Threshold(); // thr should be an array of values, one for each asic
      if (!p&&i==frunID_first) {printf("Threshold from %d ",thr);}
      if (!p&&i==frunID_last) {printf("to %d [DAC0 unit]\n",thr);}
      // read scaler data from raw file
      TRich_Scaler *  scal = this->SKA_Read();
      
      //	printf("****************\n");
      	//scal->Print();
      	//printf("****************\n");			
      for (int j=0; j<192; j++) {scaler[j] =scal->Scaler(j);}			
      //if(p){scal->Print();usleep(10000);}	
      // Store data in the TTree
      tree->Fill();
      delete scal; // initialized by ReadScaler()
    }else {
      break;
    }
    delete cfg;
    
  }// end loop on runID
  tree->Write();
  file->Close();
  //if(!p&&i==frunID_last){printf("Created %s file\n",fioSKAroot_Scan.c_str());}
  if(!p){printf(" ");}
  return 0;
}

/*
 * Single channel plot Counts vs Threshold
 * prende un Tree 
 * crea un histogramma per canale con il numero di conteggi per ciascuna soglia
 * Rappresenta tutti i canali su un file pdf
 */

int	TRich_Analysis::SKA_Plot2(float minY,float maxY){
  
  fflush(stdout);
  int ch_first = 0;
  int ch_last = 192;
  int nch = ch_last-ch_first;
  
  // get the TTree contained in lfile (use ReadScaler_THR_Scan to produce it)
  TFile * lfile = new TFile(fioSKAroot_Scan.c_str());//if(lfile->IsZombie()){printf("pFile is Zombie!\n");}
  TTree * ltree = (TTree*)lfile->Get(TREE_SCALER_THR);
  TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
  TCanvas* mycanv = new TCanvas("mycanv","",1280-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
  
  // single channel analysis
  int channel;
  int entries =0;
  //char nomerun[256]; // obsolete

  
  TH1F * h[nch];
  for (channel=0; channel<nch; channel++) {h[channel] = NULL;}	
  

bool laser = (strcmp("Laser", fsource.c_str()) == 0) ? true:false;

	minY=0.1;
	maxY=10000.;

  for (channel=0; channel<nch; channel++) {

	if(!laser)
	{ 	// Dark or Background 
		h[channel] = new TH1F(Form("h%d",channel),Form("SCALER, HV ON, LASER OFF, duration %d s, %s CHANNEL %d",fnorm, frun_prefix.c_str(),channel),1024,-0.5,1024.5);

	

      		h[channel]->GetYaxis()->SetTitle("Rate  [Hz]");
      		h[channel]->GetYaxis()->CenterTitle();	
	}else{	
		//Laser
 		h[channel] = new TH1F(Form("h%d",channel),Form("SCALER, HV ON, LASER ON, Frequency %d Hz, %s CHANNEL %d",fnorm, frun_prefix.c_str(),channel),1024,-0.5,1024.5);

      		h[channel]->GetYaxis()->SetTitle("Efficiency [%]");
      		h[channel]->GetYaxis()->CenterTitle();	
	}
  }     
  gStyle->SetTitleFontSize(0.1);

  int n = ltree->GetEntries();
  unsigned int lthr = 0;
  unsigned int lscaler[192];
  for (int i =0; i<192; i++) {lscaler[i] = 0;}	
  ltree->SetBranchAddress("Threshold",&lthr);
  ltree->SetBranchAddress("Scaler",lscaler);
  
  for(int i= 0;i<n;i++){// loop on Entries (i.e. on threshold)
    lthr =0;
    for (int k =0; k<192; k++) {lscaler[k] = 0;}
    ltree->GetEntry(i); 
    for(int k=0;k<192;k++){
      if(lscaler[k]!=0){
	h[k]->Fill(lthr,lscaler[k]);
      }
    }
  }
  mycanv->Print(Form("../../pdf/%s_SKA_SingleChannel.pdf[",frun_prefix.c_str()));

  gStyle->SetOptStat(0);
  for(channel=0;channel<192;channel++ ) {
    if (channel<=63||channel>=128){

      h[channel]->Sumw2(); //how it works? square root of counts?
      h[channel]->Scale(1./fnorm); //how the error is propagated? 

      h[channel]->SetMaximum(maxY); 
     h[channel]->SetMinimum(minY); 


      gPad->SetTopMargin(0.1);
      gPad->SetLeftMargin(0.1);
      gPad->SetRightMargin(0.1);
      gPad->SetBottomMargin(0.1);


      gPad->SetLogy(1); 
      h[channel]->GetYaxis()->SetTitleOffset(1.);
      h[channel]->GetXaxis()->SetTitle("Threshold [DAC units]");	
      h[channel]->GetXaxis()->CenterTitle();


      h[channel]->SetMarkerSize(0.1);
      h[channel]->SetMarkerStyle(1);
      h[channel]->Draw("E1");
      mycanv->cd(1)->SetGrid(1);
      mycanv->Print(Form("../../pdf/%s_SKA_SingleChannel.pdf",frun_prefix.c_str()));
    }
  }
 mycanv->Print(Form("../../pdf/%s_SKA_SingleChannel.pdf]",frun_prefix.c_str()));

  // clean up memory
  for (channel=0; channel<nch; channel++) {
    if(h[channel]!=NULL){
      delete h[channel];
    }
  }	
  delete mycanv;
  lfile->Close();
  delete lfile;
  return entries;
}

/* Crea un Gauguin Plot
 * rappresenta i conteggi al variare della soglia per un singolo chip
 * opzione A : channel ID or Pixel ID
 * opzione B :  singolo chip oppure intera scheda
 */
int TRich_Analysis::SKA_Plot3(float minZ, float maxZ,int DAC0step){
	
  TFile * lfile = new TFile(fioSKAroot_Scan.c_str());//if(lfile->IsZombie()){printf("pFile is Zombie!\n");}
  TTree * ltree = (TTree*)lfile->Get(TREE_SCALER_THR);
  
  TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
  TCanvas* mycanv = new TCanvas("mycanv","",1280-2*SIZEOFCANV,0,8*SIZEOFCANV,8*SIZEOFCANV);
  int n = ltree->GetEntries();
  
  unsigned int lthr = 0;
  unsigned int lscaler[192];
  for (int i =0; i<192; i++) {lscaler[i] = 0;}
  
  ltree->SetBranchAddress("Threshold",&lthr);
  ltree->SetBranchAddress("Scaler",lscaler);
  
  int lchfirst = 0;
  int lchlast = 191;
 
  int DAC0min = 0;
  int DAC0max = 1024;


  double Norm = fnorm*1.0;  
  
   mycanv->SetLogz(1); 
 
  //  X - Channel ID [#], Y  - Threshold [DAC]
  

bool laser = (strcmp("Laser", fsource.c_str()) == 0) ? true:false;

	TH2F * h2;
	if(!laser)
	{ 	// Dark or Background we are interested in Rate 0.1 -> 40 MHz
		h2 = new TH2F("h2",Form("SCALER, HV ON, LASER OFF, duration %d s, %s",fnorm, frun_prefix.c_str()),lchlast-lchfirst+1,lchfirst-0.5,lchlast+0.5,(DAC0max-DAC0min+1)/DAC0step,DAC0min-0.5,DAC0max+0.5);

		h2->GetZaxis()->SetTitle("Rate [Hz]");
		h2->GetZaxis()->CenterTitle();	
	}else{	
		// In case of Laser we do efficiency measuremetns
 		h2 = new TH2F("h2",Form("SCALER, HV ON, LASER ON, frequency %d Hz, %s",fnorm, frun_prefix.c_str()),lchlast-lchfirst+1,lchfirst-0.5,lchlast+0.5,(DAC0max-DAC0min+1)/DAC0step,DAC0min-0.5,DAC0max+0.5); 
	
 		h2->GetZaxis()->SetTitle("Efficiency");	
		h2->GetZaxis()->CenterTitle();	
	}

  // ANODES version
  //TH2F * h2 = new TH2F("h2","Title 3D plot",64,0.5,64.5,(DAC0max-DAC0min)/10.,DAC0min-0.5,DAC0max-0.5);
 
   printf("BinX Width %lf",	h2->GetXaxis()->GetBinWidth(4));
  printf("BinY Width %lf",	h2->GetYaxis()->GetBinWidth(4));
  h2->SetMinimum(minZ);
  h2->SetMaximum(maxZ);
  
  const Int_t NRGBs = 5;
  const Int_t NCont = 512;
  
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);
  
  
  TRich_Adapter adapter;
  
  //int mapmt_rate;
  int first_run_idx = 0; // run idx is the entry idx of the tree (not the RunID, not the Threshold), to be improved...
  int lastf_run_idx = n;
  for (Int_t i = first_run_idx; i<lastf_run_idx; i++) { // loop on entries (run)
    // reset variables first!
    lthr = 0;
    //mapmt_rate =0;
    for (int k =0; k<192; k++) {lscaler[k] = 0;}
    ltree->GetEntry(i); 
    //printf("threshold = %d \n",lthr);
    for (int j = lchfirst ; j<lchlast; j++) { 	// loop on channels
      // interface with mapmt
      if (lscaler[j]!=0) {
       
	h2->Fill(j,lthr,lscaler[j]/Norm); //(x,y,z) 
		h2->GetYaxis()->GetBinWidth(4);
	//h2->Fill(adapter.GetAnode(j-128),lthr,lscaler[j]/Norm); //(x,y,z) 	
	
	//	printf("\tscaler[%d] %lf\n",j,lscaler[j]);	//printf("\n");
      }
      //	mapmt_rate += lscaler[j]/Norm;
      // totale mapmt
    }
    //printf("threshold %d Rate = %d [Hz] Average = %lf \n",lthr,mapmt_rate,mapmt_rate/64.);
  }
  h2->Draw("COLZ");
  //h2->Draw("COLZ,TEXT");
  //h2->SetMarkerSize(0.05);
  h2->SetStats(false);
  h2->GetXaxis()->SetTitle("Channel ID");	h2->GetXaxis()->CenterTitle();
  //h2->GetXaxis()->SetTitle("ANODE ID");	h2->GetXaxis()->CenterTitle();
 
 h2->GetYaxis()->SetTitle("Threshold [DAC unit]");	h2->GetYaxis()->CenterTitle();

 h2->GetYaxis()->SetTitleOffset(1.6);
  h2->GetZaxis()->SetTitleOffset(1.6);	
  gPad->SetLeftMargin(0.15);
  gPad->SetRightMargin(0.22);	 
  mycanv->Print(Form("../../pdf/%s_SKA.pdf",frun_prefix.c_str()));
  delete mycanv;
  lfile->Close();	
  delete lfile;
  return n;
}

