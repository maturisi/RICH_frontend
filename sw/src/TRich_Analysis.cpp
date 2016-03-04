
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

#define SIZEOFCANV 200

#define HEVTTIME "triggerTIME"
#define HEVTFREQ "triggerFREQ"
#define HEVTEDGE "nedges_SCAT"
#define HBRDRISE "riseTIME"
#define HBRDFALL "fallTIME"
 


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
fioTDCtrigger = "";
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

	printf("%s...\n",__FUNCTION__);
	fflush(stdout);
  bool p = false;
  
	// single run variables
  int gain;
	int thr;
	int runID;
	int nevt;
	int totEvents;
	
	Float_t riseEdges[192];
	Float_t riseMean[192];
	Float_t riseRMS[192];

	Float_t fallEdges[192];
	Float_t fallMean[192];
	Float_t fallRMS[192];


  	std::string fileroot;
  	std::string filelog;
  	std::string fileraw;
  	char strID[7];
  
  	TFile * lfile;
  	TTree * ltree;
  
  	TFile * lscanfile;
  	TTree * lscantree;
  	TH1F * 	lhRise;
	TH1F * 	lhFall; 
  

  	Double_t mean, rms,eff;
  
  	char lhname[10];
  	char lhfallname[10];

  	TRich_Config cfg;

	switch (fdaq_mode) {
  		case 0: // scaler ...
   		break;
  
		case 1: // TDC
    	switch(fmode){

	  		case SINGLE_RUN:
										
				nevt = this->TDC_Read(); // parse single run, writing an output Ttree with Time stamp and edge list

      	break;
    
			case SCAN:
			//	printf(" %s SCAN MODE\n",__FUNCTION__);
      	lscanfile = new TFile(fTDCscanroot.c_str(),"RECREATE");
				if(lscanfile->IsOpen()){
					printf("File %s opened succesfully\n",fTDCscanroot.c_str());
					lscantree = new TTree("TDCscandata","Calibration data, threshold gain scan");
					lscantree->Branch("gain"			,&gain		,"gain/i");		
					lscantree->Branch("threshold" ,&thr		,"thr/i");		
					lscantree->Branch("runID" ,&runID		,"runID/i");	
					lscantree->Branch("totEvents" ,&totEvents		,"totEvents/i");	

  				lscantree->Branch("nRise",riseEdges,"riseEdges[192]/F");
  				lscantree->Branch("tRise",riseMean,"riseMean[192]/F");
  				lscantree->Branch("dtRise",riseRMS,"riseRMS[192]/F");

  				lscantree->Branch("nFall",fallEdges,"fallEdges[192]/F");
  				lscantree->Branch("tFall",fallMean,"fallMean[192]/F");
  				lscantree->Branch("dtFall",fallRMS,"fallRMS[192]/F");
      			}
      			//printf("LOOP ON FILES looking for histos\n");
     		 	for(int r=fTDC_runID_first;r<=fTDC_runID_last;r++){
				gain=0;
				thr =0;
				runID =0;
				totEvents =0;
			
				for(int i=0;i<192;i++){
					riseEdges[i]=.0;
					riseMean[i]=.0;
					riseRMS[i]=.0;
					fallEdges[i]=.0;
					fallMean[i]=.0;
					fallRMS[i]=.0;
				}
				printf("RUN %3d ",r);
				sprintf(strID, "%06d",  r);
				fileraw = frun_path + frun_prefix + strID + ".bin";
						
				this->NameRun(fileraw.c_str());

				thr = this->GetThreshold();
				printf("THR %4d ",thr);
				gain = this->GetGain();
				printf("GAIN %4d ",gain);
				runID =r;
				fTDCroot = "../../data/parsed/" + frun_name + "_TDC" + ".root";
						
				printf("fileroot[%3d] = %s ",r,fTDCroot.c_str());
						
				lfile = new TFile(fTDCroot.c_str());
				if(lfile->IsZombie()){printf("pFile is Zombie!\n");}
				if(lfile->IsOpen()){
	  				//printf("File %s is open\n",fTDCroot.c_str());
	  				//lfile->GetListOfKeys()->Print();
	  				ltree = (TTree*)lfile->Get("TDCdata"); 
	  				nevt = ltree->GetEntries();
	  				printf(" N trigger= %d",nevt);
	  				printf("\n");
					totEvents = nevt;	
	  				for(int ch=0;ch<192;ch++){
 						sprintf(lhfallname,"ch%03dTFall",ch);
	    					sprintf(lhname,"ch%03dTRise",ch);
	    					mean = .0;
	    					rms = .0;
	   					eff = .0;
	    					lhRise = NULL;
						lhFall = NULL;
	    					lhRise = (TH1F*) lfile->Get(lhname);
						if(lhRise){
	      						mean = lhRise->GetMean();
	      						rms  = lhRise->GetRMS();
	      						eff  = 1*lhRise->GetEntries()/nevt;
							riseEdges[ch]= lhRise->GetEntries();
							riseMean[ch]= mean;
							riseRMS[ch]= rms;
							if(p){printf("\tCH_%d_%2d RISE (Mean %4.0lf RMS %6.3lf Eff %.3lf) ",ch/64,ch%64,mean,rms,eff);}
	    					}	
						lhFall = (TH1F*) lfile->Get(lhfallname);
						if(lhFall){
	      						mean = lhFall->GetMean();
	      						rms  = lhFall->GetRMS();
	      						eff  = 1*lhFall->GetEntries()/nevt;
							fallEdges[ch]= lhFall->GetEntries();
							fallMean[ch]= mean;
							fallRMS[ch]= rms;
	      						if(p){printf("\tCH_%d_%2d FALL (Mean %4.0lf RMS %6.3lf Eff %.3lf)\n",ch/64,ch%64,mean,rms,eff);}
	    					}
					}// end of loop on channels
				}	
				lfile->Close(); 
				delete lfile;
				lscantree->Fill();
			} // end of loop on runs
  	    		lscanfile->cd();	
			//lscantree->Print(); 				
			lscantree->Write(); 
			delete lscantree;
      			lscanfile->Close();
			delete lscanfile;

			printf("Data from histos  ingested ");
			printf("in %s\n",fTDCscanroot.c_str()); 
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



void TRich_Analysis::ProcessTDCscan(){

	bool print = false;

  //printf("\n%s\n",__FUNCTION__);
  
  TCanvas* mycanv = new TCanvas("mycanv","",1280-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);

	std::string nomescanpdf = 	fTDCscanpdf;

  // OPEN PDF FILE
  mycanv->Print(Form("%s[",nomescanpdf.c_str()));


	// local variables associated to the Ttree
  UInt_t gain;
	UInt_t thr;
	UInt_t runID;
	UInt_t totEvents;
	
	Float_t riseEdges[192];
	Float_t riseMean[192];
	Float_t riseRMS[192];

	Float_t fallEdges[192];
	Float_t fallMean[192];
	Float_t fallRMS[192];


	// local histograms

	Int_t nbinsx = 192;
	Int_t nbinsy = 1024;
	Int_t nbinsz = 255;

	Double_t xlow = -0.5;
	Double_t ylow = -0.5;
	Double_t zlow = -0.5;

	Double_t xup = 191.5;
	Double_t yup = 1023.5;
	Double_t zup = 254.5;


	TH3F * h123 = new TH3F("hcal","hcal",nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup);

	
  TFile * lfile = new TFile(fTDCscanroot.c_str(),"OLD");  //lfile->GetListOfKeys()->Print();
  
 	TTree * ltree = (TTree*)lfile->Get("TDCscandata"); // better using a macro in general header


	ltree->SetBranchAddress("gain"			,	&gain);			
	ltree->SetBranchAddress("threshold"	,	&thr);		
	ltree->SetBranchAddress("runID"			,	&runID);		
	ltree->SetBranchAddress("totEvents"	,	&totEvents);	
	  
  ltree->SetBranchAddress("nFall"			,	fallEdges);
  ltree->SetBranchAddress("tFall"			,	fallMean);
  ltree->SetBranchAddress("dtFall"		,	fallRMS);
  ltree->SetBranchAddress("nRise"			,	riseEdges);
  ltree->SetBranchAddress("tRise"			,	riseMean);
  ltree->SetBranchAddress("dtRise"		,	riseRMS);
  

	Int_t nEntries = ltree->GetEntries();

	for(Int_t r = 0;r<nEntries;r++){ // one entry one run
		
		// RESET reading variables
		gain=0;
		thr =0;
		runID =0;
		totEvents =0;

		for(int i=0;i<192;i++){

			riseEdges[i]=.0;
			riseMean[i]=.0;
			riseRMS[i]=.0;
			fallEdges[i]=.0;
			fallMean[i]=.0;
			fallRMS[i]=.0;
		}
		

		// Get Entry
    		ltree->GetEntry(r);

		// Fill

//...
	
		// Print
		if(print){
		//	printf("Bytes %d ",nbytes);
			printf("Entry %4d ",r);
			printf("RUNID %3d ",runID);
			printf("NTRIG %6d ",totEvents);
			printf("GAIN %3d ",gain);
			printf("THR %4d ",thr);	
			printf("\n");

			for(int ch = 0;ch<192;ch++){
				if(fallEdges[ch]&&riseEdges[ch]){
					printf("\t");
					printf("ASIC_%d_CH_%2d: ",ch/64,ch%64);
					printf("FALL (");
					printf("%3.3lf, ",1*fallEdges[ch]/(totEvents));
					printf("%3.1lf, ",fallMean[ch]);
					printf("%5.0lf) ",fallRMS[ch]*1000);
					printf("RISE (");
					printf("%3.3lf, ",1*riseEdges[ch]/(totEvents));
					printf("%3.1lf, ",riseMean[ch]);
					printf("%5.0lf) ",riseRMS[ch]*1000);
					printf("\n");
					
				}
			}
		}
	} // end of loop on entries (i.e. runs)

	ltree->Draw("gain:thr");
	mycanv->Print(nomescanpdf.c_str());

	ltree->Draw("totEvents:thr","");
	mycanv->Print(nomescanpdf.c_str());


	ltree->SetMarkerStyle(7); 
	ltree->SetMarkerSize(.6);


	std::string what = "fallRMS";

	for(int ch=128;ch<192;ch++){

		if(ch>=128||ch<=63){
			//printf("Drawing ch %d\n",ch);
			ltree->Draw(Form("%s[%d]:thr",what.c_str(),ch),Form("gain==%d&&%s[%d]<=2.0",64,what.c_str(),ch),"");
			for(int g = 2;g<=4;g++){
				ltree->SetMarkerColor(g);	
				ltree->Draw(Form("%s[%d]:thr",what.c_str(),ch),Form("gain==%d&&%s[%d]<=2.0",g*64,what.c_str(),ch),"SAME");
			}
			mycanv->Print(nomescanpdf.c_str());
		}
	}

// CLOSE PDF FILE
  mycanv->Print(Form("%s]",nomescanpdf.c_str()));

//printf("Scan plots available at %s (n)\n",fTDCscanpdf.c_str()); 

	delete h123;

lfile->Close();

delete lfile;

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

	fioTDCtrigger = "../../histo/" + frun_name + "_histo" + ".root";

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


unsigned int TRich_Analysis::GetTriggerDelay(){
  unsigned int ret = this->GetFPGAParameters(0);
  return ret;
}
unsigned int TRich_Analysis::GetEventBuilderLookBack(){
  unsigned int ret =	this->GetFPGAParameters(1);
  return ret;
}	
unsigned int TRich_Analysis::GetEventBuilderWindow(){
  unsigned int ret =	this->GetFPGAParameters(2);
  return ret;
}




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
				//evt->Print(0);//evt->Print(2,34);// 0 nothing, 1 general info, 2, channel info use second argument to select channel, use 192 for all the channels			
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
    TCanvas * lmycanv = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");
    if(!lmycanv){printf("Canvas not found...\n");}
    
    lmycanv->cd(1)->SetLogy(1);
    lmycanv->cd(1)->SetGrid(1);
    
    hRise->Draw();
    hFall->Draw("same");
    tpsRise->Draw("same");
    tpsFall->Draw("same");
    lmycanv->Print(fTDCpdf.c_str());
  }
}




void TRich_Analysis::ProcessTDC(){




/*
	printf("%s... file %s\n",__FUNCTION__,fioTDCroot.c_str());
	bool printEvents 	= false;
	bool printEdges 	= false;
	bool hitreconstruction	= true;


  	// RETRIEVE CONFIGURATION PARAMETERS
  	//UInt_t thr 	= this->GetThreshold();	// get threshold from logbook (individual chip)
  	//UInt_t gain 	= this->GetGain();	// get gain from logbook (individual channel)

 	//UInt_t ltrigdly	= this->GetTriggerDelay();
 	//UInt_t llookback	= this->GetEventBuilderLookBack();
 	UInt_t lwindow	= this->GetEventBuilderWindow();
 	lwindow = 8*lwindow; // in ns
  
  	
    //	printf("Settings MAROC3:\n");
    
   // 	printf("Threshold [DAC0 unit] = %d\n",thr);
    //	printf("GAIN %4d ",gain);
    
   // 	printf("Settings FPGA:\n");
    //	printf("Trigger delay           =%5d -> %d*[8ns] = %5d [ns]\n",ltrigdly,ltrigdly,ltrigdly*8);  //128*8ns = 1024ns trigger delay
    //	printf("Event Builder LookBack  =%5d -> %d*[8ns] = %5d [ns]\n",llookback,llookback,llookback*8);
    //	printf("Event Builder Window	=%5d -> %d*[8ns] = %5d [ns]\n",lwindow,lwindow,lwindow*8);	
  	
  
  	TFile * lfile = new TFile(fioTDCroot.c_str(),"UPDATE"); if(lfile->IsZombie()){printf("pFile is Zombie!\n");}
  	TTree * ltree = (TTree*)lfile->Get("TDCdata"); // better using a macro in general header
  
  	UInt_t lEventID	= 0; ltree->SetBranchAddress("triggerID",&lEventID);
  	UInt_t ltstamp0 = 0; ltree->SetBranchAddress("tstamp0",&ltstamp0);
  	UInt_t ltstamp1 = 0; ltree->SetBranchAddress("tstamp1",&ltstamp1);
  	UInt_t lnedge =   0;ltree->SetBranchAddress("nedge"	,&lnedge);
  
  	double time_prev = 0.0;
  	double time = 0.0;
  	double dt;
  
  	int maxHz = 1000*1000; // 1 MHz
 	// int maxEdges = 3*4*64; // max 4 edges/channel per trigger (at the baseline is Billions but they will go in overflow bin)
 	int maxEdges = 250; // max 4 edges/channel per trigger (at the baseline is Billions but they will go in overflow bin)
    

  	int totEvts = ltree->GetEntries();
  
  	TH1F * hTRGtime = new TH1F("TRGtime","",totEvts,-0.5,totEvts-0.5);
  	TH1F * hTRGfreq  = new TH1F("TRGfreq","",maxHz,-0.5,maxHz-0.5);	// 1 Hz Resolution
  
	TH1F * hEdges = new TH1F("hEdges","",totEvts,-0.5,totEvts-0.5);
  	TH1F * hEdgesD = new TH1F("hEdgesD","",maxEdges,-0.5,maxEdges-0.5);

  
  	int timmax = lwindow;
  	int timmin = 0;
	
  	TH1F * hTRise = new TH1F("hTRise","",timmax-timmin,timmin-0.5,timmax-0.5);
  	TH1F * hTFall = new TH1F("hTFall","",timmax-timmin,timmin-0.5,timmax-0.5);

  	hTRise->SetTitle("All channels - Edge Time Distribution");
  	hTFall->SetTitle("All channels - Edge Time Distribution");
	
  	//TH2F * hTFallvsTot = new TH2F("hTFallvsTot","",timmax-timmin,timmin-0.5,timmax-0.5,timmax-timmin,timmin-0.5,timmax-0.5);
  	//TH2F * hTRisevsTot = new TH2F("hTRisevsTot","",timmax-timmin,timmin-0.5,timmax-0.5,timmax-timmin,timmin-0.5,timmax-0.5);

 	TH1F * hTRiseCh[192]; 
  	TH1F * hTFallCh[192];
  	TH1F * hTotCh[192];


	for (int i=0; i<192; i++) {
 		//nt ltmin = 200;
		//int ltmax = 400;
		
	//	timmin = ltmin;
	//	timmax = ltmax;
    		
hTRiseCh[i] = new TH1F(Form("ch%03dTRise",i),"",timmax-timmin,timmin-0.5,timmax-0.5);
    		hTFallCh[i] = new TH1F(Form("ch%03dTFall",i),"",timmax-timmin,timmin-0.5,timmax-0.5);
    		hTotCh[i] = new TH1F(Form("ch%03dTTot",i),"",400,-200.5,199.5); 
  	}
  
  	const int maxhit = 10000;  // a small value could create segmentation fault when analising runs with threshold  very close to pedestal
 
  	UInt_t ltime[maxhit];
  	UInt_t lpolarity[maxhit];
  	UInt_t lchannel[maxhit];
  
  	ltree->SetBranchAddress("polarity"	,lpolarity);
  	ltree->SetBranchAddress("channel"	,lchannel);
  	ltree->SetBranchAddress("time"	,ltime);
  

  	int polar;
  	int channel;
  	int hittime;
  	int polar2;
  	int channel2;
  	int hittime2;
	int duration; // time over threshold (tot)
	
	// controlla chi e' piu grande tra totevents e max event e inizializza il ciclo for
  
	int maxevent= totEvts;
	int tothit=0;

  	// LOOP ON EVENTS
  	for(int e = 0;e<maxevent;e++){
    		lEventID = 0;
    		ltstamp0 = 0;
    		ltstamp1 = 0;
   		lnedge = 0;
    
   		ltree->GetEntry(e);
    
    		time = this->DecodeTimeStamp(ltstamp0,ltstamp1);
    		dt = time-time_prev;
    		time_prev = time;
    

    		if (printEvents) {
      			if(e!=totEvts-1){
				if(e%32==0){
					printf("EVENT LIST:\n");
				}  
				printf("ID %4d ",lEventID);   
				printf("NEdges %3d ",lnedge);   
				printf("Time %12.10lf [s] ",time);
				printf("(DT %12.10lf [s],  ",dt);
      				printf("Freq %6.0lf [Hz])", 1/dt);
				printf("\n");
      			}
    		}
		hTRGtime->Fill(lEventID,time);
  	
		if(e>1&&e!=totEvts-1){
		// need 2 event to measure dt! first event is skipped,
		// last event is not filled by TDC_Read so it would give a negative dt
			hTRGfreq->Fill(1/dt);
		} 
    		hEdges->Fill(lEventID,lnedge);
    		hEdgesD->Fill(lnedge); 
   

    		// LOOP ON EDGES
    		for (int i=0; i<(int)lnedge; i++) {
      
			// Read
      			polar = lpolarity[i];
      			channel = lchannel[i];
      			hittime = ltime[i];
     
			// Fill
      			switch (polar) {
      			case 0:
				hTRise->Fill(hittime);
				hTRiseCh[channel]->Fill(hittime);
				break;
      			case 1:
				hTFall->Fill(hittime);
				hTFallCh[channel]->Fill(hittime);
				break;
      			default: printf("Error in %s: unknown edge polarity %d\n",__FUNCTION__,polar);
				break;
      			}
			

			if(hitreconstruction){			// Hit Reconstruction
				duration =0;
				// look for the same channel with opposite polarity and calculate hit duration
 				for (int j=i+1; j<(int)lnedge; j++) {
					channel2 = lchannel[j];
					if(channel2==channel){
						polar2 = lpolarity[j];
						if(polar2!=polar){
							hittime2 = ltime[j];	
							if(polar2==1){
								duration = hittime-hittime2;
							}else{
								duration = hittime2-hittime;
							}							
							tothit++;				
						
							hTotCh[channel]->Fill(duration);
	

									
						}
					}
				}
			}		
      			if(printEdges){
				if(i==0){
	  				printf("EDEGE LIST\t( Rise  = <  Fall = > )\n");
				}
				printf("%3d ",i+1);			
				printf("CH_%d",channel/64);
				printf("_%2d ",channel%64);
		
				switch(polar){
					case 0: printf("< "); break;
					case 1: printf("> ");break;
					default:break;
				}
				printf("%4d  ",hittime);
				printf("\n");	
  	    		}	// end of if printf
  	  	} // end of loop on edgess
  	} // end of loop on events
  

  	printf("HITS = %d\n",tothit);

  	hTRGtime->Write();  
  	hTRGfreq->Write();
  	hEdges->Write();
  	hEdgesD->Write();
  	hTRise->Write();
  	hTFall->Write();
//hTRisevsTot->Write();
//hTFallvsTot->Write();


  	delete hTRGtime;
  	delete hTRGfreq;
  	delete hEdges;
  	delete hEdgesD;
  	delete hTFall;
  	delete hTRise;
//	delete hTFallvsTot;
//	delete hTRisevsTot;
  
	int histc = 0;
  	for (int i = 0; i<192; i++){
		histc++;
      		hTRiseCh[i]->Write();
      		hTFallCh[i]->Write();
      		hTotCh[i]->Write();
			
      		if(hTRiseCh[i]){delete hTRiseCh[i];}
      		if(hTFallCh[i]){delete hTFallCh[i];}
      		if(hTotCh[i]){delete hTotCh[i];}
  	}
  	
	lfile->Close();	
  	delete lfile;	
	//printf("Analysis completed successfully\n");
	printf("Histograms added %d to   %s \n",histc,fioTDCroot.c_str());

*/
}



void TRich_Analysis::Plot(){

	TCanvas * c = new TCanvas("c");
	const char * out_file = "../../pdf/Pippo.pdf";

	TFile f(fioTDCtrigger.c_str());
	TH1F * h = NULL;
	h = (TH1F*) f.Get(HEVTTIME); if(h!=NULL){h->Draw(); c->Print(Form("%s(",out_file),"Title:Trigger Time"); h=NULL;}
	h = (TH1F*) f.Get(HEVTFREQ); if(h!=NULL){h->Draw(); c->SetLogx(1);c->Print(Form("%s",out_file),"Title:Trigger Frequency"); h=NULL;}
	h = (TH1F*) f.Get(HEVTEDGE); if(h!=NULL){h->Draw(); c->SetLogx(0);c->Print(Form("%s",out_file),"Title:Edges Multiplicity"); h=NULL;}
	h = (TH1F*) f.Get(HBRDRISE); if(h!=NULL){h->Draw(); c->SetLogy(1);c->Print(Form("%s",out_file),"Title:Rise Time"); h=NULL;}
	h = (TH1F*) f.Get(HBRDFALL); if(h!=NULL){h->Draw(); c->SetLogy(1);c->Print(Form("%s)",out_file),"Title:Fall Time"); h=NULL;}

	f.Close();	
	delete c;

	printf("Plots File %s\n ",out_file);
}

// Process parsed data in fioTDCroot, creates histograms and save them in fioTDCtrigger

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

	unsigned int event, nedge, channel, polarity, time,nedge_max=0;
	double etime, dt, etime_prev=0.0;
	bool dbg,ok;
	int entries;
  unsigned int lwindow	= this->GetEventBuilderWindow(); // clock ticks [8ns]
  int timmax = 8*lwindow; // ns
  int timmin = 0;
	
	TRich_TDC * tdc = new TRich_TDC();
	ok = tdc->OpenFile(fioTDCroot.c_str()); 	if(!ok) return;
	entries = tdc->RetrieveTree();

	TFile * out_file 	= new TFile(fioTDCtrigger.c_str(),"RECREATE"); 
  TH1F * trigger 		= new TH1F(HEVTTIME,"",entries				,-0.5				,entries-0.5);
	TH1F * frequency  = new TH1F(HEVTFREQ,"",1E6		 				,-0.5				,	   1E6-0.5);	
  TH1F * edge 	 		= new TH1F(HEVTEDGE,"",entries				,-0.5				,entries-0.5);
  TH1F * rise 			= new TH1F(HBRDRISE,"",timmax-timmin	,timmin-0.5	, timmax-0.5);
  TH1F * fall 			= new TH1F(HBRDFALL,"",timmax-timmin	,timmin-0.5	, timmax-0.5);


	for(int e=0; e<entries; e++){
		dbg = (e%(entries/10)==0) ? true : false;
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
/*
	// second loop is for adjustable x-axis range
	//	printf("Max edges = %u, take %d\n",nedge_max,(int)(nedge_max*1.1));
  TH1F * edgeD = new TH1F("Nedges_DISTRIB","",(int)(nedge_max*1.1),-0.5,((int)(nedge_max*1.1))-0.5);
	for(int e=0; e<entries; e++){
		dbg = (e%(entries/10)==0) ? true : false;
		tdc->Reset();
		tdc->GetEntry(e);		
		nedge = tdc->NEdges();	
		edgeD->Fill(nedge);
		if(dbg)printf("Edge = %4d \n",nedge);
	}
	edgeD->Write();
	delete edgeD;
*/	
  out_file->Close();
	delete out_file;
	delete tdc;
	printf("Histograms saved in %s\n",fioTDCtrigger.c_str());
}



void 	TRich_Analysis::ProcessTDCTEMP(){ // BACK UP copy during sw update	

	printf("%s...\n",__FUNCTION__);
	bool printEvents 			= true;
	bool printEdges 			= false;
	bool printStatistics 	= false;
	bool hitreconstruction= true;


  // RETRIEVE CONFIGURATION PARAMETERS

  UInt_t thr 	= this->GetThreshold();	// get threshold from logbook (individual chip)
  UInt_t gain 	= this->GetGain();	// get gain from logbook (individual channel)
  UInt_t ltrigdly	= this->GetTriggerDelay();
	UInt_t llookback	= this->GetEventBuilderLookBack();
  UInt_t lwindow	= this->GetEventBuilderWindow();
  lwindow = 8*lwindow; // in ns
	
  printf("Settings MAROC3:\n");
  printf("Threshold [DAC0 unit] = %d\n",thr);
  printf("GAIN %4d ",gain);
  printf("Settings FPGA:\n");
  printf("Trigger delay           =%5d -> %d*[8ns] = %5d [ns]\n",ltrigdly,ltrigdly,ltrigdly*8);  //128*8ns = 1024ns trigger delay
  printf("Event Builder LookBack  =%5d -> %d*[8ns] = %5d [ns]\n",llookback,llookback,llookback*8);
  printf("Event Builder Window	=%5d -> %d*[8ns] = %5d [ns]\n",lwindow,lwindow,lwindow*8);	
  


	// Open parsed data file  
	TFile * lfile = new TFile(fioTDCroot.c_str(),"UPDATE"); 
	
	if (!lfile->IsOpen()) {
		printf(" File %s not found, please call Ingest()\n",fioTDCroot.c_str());
		return;
	
		if(lfile->IsZombie()){ // ??
			printf("pFile is Zombie!\n");
		}
	}else {
		printf(" File %s opened\n",fioTDCroot.c_str());	
	}

	// Retrieve TTree and prepare for reading
  TTree * ltree = (TTree*)lfile->Get("TDCdata"); // better using a macro in general header
  
  UInt_t lEventID	= 0;
  UInt_t ltstamp0 = 0; 
  UInt_t ltstamp1 = 0; 
  UInt_t lnedge =   0;
  
  const int maxhit = 10000;  // a small value could create segmentation fault when analising runs with threshold  very close to pedestal
 
  UInt_t ltime[maxhit];
  UInt_t lpolarity[maxhit];
  UInt_t lchannel[maxhit];

  ltree->SetBranchAddress("triggerID"	,&lEventID);
  ltree->SetBranchAddress("tstamp0"		,&ltstamp0);
  ltree->SetBranchAddress("tstamp1"		,&ltstamp1);
  ltree->SetBranchAddress("nedge"			,&lnedge);
  ltree->SetBranchAddress("polarity"	,lpolarity);
  ltree->SetBranchAddress("channel"		,lchannel);
  ltree->SetBranchAddress("time"			,ltime);


  int maxHz = 1000*1000; // 1 MHz
 // int maxEdges = 3*4*64; // max 4 edges/channel per trigger (at the baseline is Billions but they will go in overflow bin)
 int maxEdges = 128; // max 4 edges/channel per trigger (at the baseline is Billions but they will go in overflow bin)
    

  int totEvts = ltree->GetEntries();
  
  // Trigger characterization
  TH1F * hTRGtime = new TH1F("TRGtime","",totEvts,-0.5,totEvts-0.5);
  TH1F * hTRGfreq  = new TH1F("TRGfreq","",maxHz,-0.5,maxHz-0.5);	// 1 Hz Resolution
  
	// Global response: Number of edges
  TH1F * hEdges = new TH1F("hEdges","",totEvts,-0.5,totEvts-0.5);
  TH1F * hEdgesD = new TH1F("hEdges","",maxEdges,-0.5,maxEdges-0.5);

  // Global Time Response
  int timmax = lwindow;
  int timmin = 0;
	
  TH1F * hTRise = new TH1F("hTRise","",timmax-timmin,timmin-0.5,timmax-0.5);
  TH1F * hTFall = new TH1F("hTFall","",timmax-timmin,timmin-0.5,timmax-0.5);

  hTRise->SetTitle("ASIC board - Edge Time Distribution");
  hTFall->SetTitle("ASIC board - Edge Time Distribution");

  TH1F *  hTot = new TH1F("hTot","",500,-250.5,249.5); 
   
  TH2F * hTFallvsTot = new TH2F("hTFallvsTot","Charge Correlation with Falling Edge time",500,-250.5,249.5,timmax-timmin,timmin-0.5,timmax-0.5);
  TH2F * hTRisevsTot = new TH2F("hTRisevsTot","Charge Correlation with Rising Edge time",500,-250.5,249.5,timmax-timmin,timmin-0.5,timmax-0.5);

  //TH2F * hTFallvsTot = new TH2F("hTFallvsTot","Charge Correlation with Falling Edge time",90,-10.5,79.5,80,220-0.5,300-0.5);
  //TH2F * hTRisevsTot = new TH2F("hTRisevsTot","Charge Correlation with Rising Edge time",90,-10.5,79.5,80,220-0.5,300-0.5);






// Individual Channel
  TH1F * hTRiseCh[192]; 
  TH1F * hTFallCh[192];
  TH1F * hTotCh[192];
  TH2F * hTRiseWalkCh[192];
  TH2F * hTFallWalkCh[192];

	// focus on the region of interest (time and charge)
	int ltmin = 200;
	int ltmax = 350;
	int ltotmin = -10; // must be negative!
	int ltotmax = 80;	
	for (int i=0; i<192; i++) {

		timmin = ltmin;
		timmax = ltmax;

    hTRiseCh[i] 		= new TH1F(Form("ch%03dTRise",i),"",timmax-timmin,timmin-0.5,timmax-0.5);
    hTFallCh[i] 		= new TH1F(Form("ch%03dTFall",i),"",timmax-timmin,timmin-0.5,timmax-0.5);
    hTotCh[i] 			= new TH1F(Form("ch%03dTTot",i),"",500,-250.5,249.5); 
    hTRiseWalkCh[i] = new TH2F(Form("ch%03dTRiseWalk",i),"",ltotmax-ltotmin,ltotmin-0.5,ltotmax-0.5,timmax-timmin,timmin-0.5,timmax-0.5);
    hTFallWalkCh[i] = new TH2F(Form("ch%03dTFallWalk",i),"",ltotmax-ltotmin,ltotmin-0.5,ltotmax-0.5,timmax-timmin,timmin-0.5,timmax-0.5);
  }
  

  
	
	// 3 Histograms as Channels summary
	int nhisto = 9;
	TH1F * hch[nhisto];

	TH2F * hpx[nhisto];
	
	// 0 - Rise entries vs channel
	// 1 - Fall entries vs channel
	// 2 - Rise delay vs channel
	// 3 - Fall delay vs channel
	// 4 - Rise tts vs channel
	// 5 - Fall tts vs channel	


	// 6 - Duration Entries vs channel 
	// 7 - Duration Mean vs channel
	// 8 - Duration RMS vs channel

	for (int i=0; i<nhisto; i++) {
		hch[i] = new TH1F("","",200,-0.5,199.5);
		hpx[i] = new TH2F("","",30,0,30,8,0,8);


	}


	
	TCanvas* mycanv = new TCanvas("mycanv","",1280-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
 	// TCanvas* mycanv2 = new TCanvas("mycanv2","",1280-2*SIZEOFCANV,0,2*SIZEOFCANV,3*SIZEOFCANV);
  
	//	int n_asic = 2
	//	mycanv2->Divide(2,1);
	//	mycanv->cd();
	mycanv->Print(Form("%s[",fTDCpdf.c_str()));
	
  // local variables for
  int polar;
  int channel;
  int hittime;

  int polar2;
  int channel2;
  int hittime2;
  
	int duration; // time over threshold (tot)
	
	// controlla chi e' piu grande tra totevents e max event e inizializza il ciclo for
  
	int maxevent= totEvts;

	maxevent =10;

	int tothit=0;

  double time_prev = 0.0;
  double time = 0.0;
  double dt;

  // LOOP ON EVENTS
  for(int e = 0;e<maxevent;e++){
    // reset
    lEventID = 0;
    ltstamp0 = 0;
    ltstamp1 = 0;
    lnedge = 0;
    
    ltree->GetEntry(e);
    
    time = this->DecodeTimeStamp(ltstamp0,ltstamp1);
    dt = time-time_prev;
    time_prev = time;
    

    if (printEvents) {
      if(e!=totEvts-1){
				if(e%32==0){
					printf("EVENT LIST:\n");
				}  
				printf("ID %4d ",lEventID);   
				printf("NEdges %3d ",lnedge);   
				printf("Time %12.10lf [s] ",time);
				printf("(DT %12.10lf [s],  ",dt);
      	printf("Freq %6.0lf [Hz])", 1/dt);
				printf("\n");
      }
    }

    hTRGtime->Fill(lEventID,time);
  	
		if(e>1&&e!=totEvts-1){
		// need 2 event to measure dt! first event is skipped,
		// last event is not filled by TDC_Read so it would give a negative dt
			hTRGfreq->Fill(1/dt);
		} 

    hEdges->Fill(lEventID,lnedge);
    hEdgesD->Fill(lnedge); 
   

    // LOOP ON EDGES
    for (int i=0; i<(int)lnedge; i++) {
      
			// Read
      polar = lpolarity[i];
      channel = lchannel[i];
      hittime = ltime[i];
     
			//Fill
      switch (polar) {
      case 0:
				hTRise->Fill(hittime);
				hTRiseCh[channel]->Fill(hittime);
				break;
      case 1:
				hTFall->Fill(hittime);
				hTFallCh[channel]->Fill(hittime);
				break;
      default: printf("Error in %s: unknown edge polarity %d\n",__FUNCTION__,polar);
				break;
      }
			

			if(hitreconstruction){			// Hit Reconstruction
				duration =0;
				// look for the same channel with opposite polarity and calculate hit duration
 				for (int j=i+1; j<(int)lnedge; j++){
					channel2 = lchannel[j];
					if(channel2==channel){
						polar2 = lpolarity[j];
						if(polar2!=polar){
							hittime2 = ltime[j];	
							//if(channel==1){
								if(polar2==1){ // secondo fall
									duration = hittime-hittime2;	
								}else{//	secondo e'un rise
								
									duration = hittime2-hittime;
									hTFallvsTot->Fill(duration,hittime);
									hTRisevsTot->Fill(duration,hittime2);
									hTFallWalkCh[channel]->Fill(duration,hittime);
									hTRiseWalkCh[channel]->Fill(duration,hittime2);
								}
							//}
							//duration = hittime2-hittime;
							tothit++;				
						//	if(duration<0){
						//		duration=-duration;
						//	}
							hTot->Fill(duration);
							hTotCh[channel]->Fill(duration);		
						}
					}
				}
			}      
			if(printEdges){
				if(i==0){
	  			printf("EDEGE LIST\t( Rise  = <  Fall = > )\n");
				}
				printf("%3d ",i+1);			
				printf("CH_%d",channel/64);
				printf("_%2d ",channel%64);
	
				switch(polar){
					case 0: printf("< "); break;
					case 1: printf("> ");break;
					default:break;
				}
				printf("%4d  ",hittime);
				printf("\n");	
      }	// end of if printf
    } // end of loop on edges
  } // end of loop on events
  
	printf("HITS = %d\n",tothit);

  gStyle->SetOptStat(111111);	  

	// Outputs and free memory
  
	// Event Timestamp
  mycanv->cd(1)->SetLogy(0);
  mycanv->cd(1)->SetLogx(0);
  mycanv->cd(1)->SetGrid(1);
  hTRGtime->SetTitle("Time Stamp vs Trig ID");
  hTRGtime->GetXaxis()->SetTitle("Event ID[#]");
  hTRGtime->GetYaxis()->SetTitle("Time Stamp[s]");
  hTRGtime->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hTRGtime->Write();
  delete hTRGtime;
  
  
  // Trigger Frequency
  //hTRGfreq->Scale(1./totEvts);
  mycanv->cd(1)->SetLogy(1);
  mycanv->cd(1)->SetLogx(1);
  hTRGfreq->SetTitle("Trigger Time Interval Distribution ");
  hTRGfreq->GetXaxis()->SetTitle("Trigger Rate [Hz]");
  hTRGfreq->GetYaxis()->SetTitle("Occourrency [#]");
  hTRGfreq->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hTRGfreq->Write();
  delete hTRGfreq;
  
  // Edges Multiplicity
  mycanv->cd(1)->SetLogy(0);
  mycanv->cd(1)->SetLogx(0);
  mycanv->cd(1)->SetGrid(1);
  hEdges->SetTitle("Edges Multiplicity");
  hEdges->GetXaxis()->SetTitle("Event ID[#]");
  hEdges->GetYaxis()->SetTitle("N Edges[#]");
  hEdges->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hEdges->Write();
  delete hEdges;
  
  
  mycanv->cd(1)->SetLogy(1);
  mycanv->cd(1)->SetLogx(0);
  //mycanv->cd(1)->SetGrid(1);
//  hEdgesD->SetMaximum(10E6);
  hEdgesD->SetTitle("Edges Multiplicity Distribution");
  hEdgesD->GetXaxis()->SetTitle("N Edges per event[#]");
  hEdgesD->GetYaxis()->SetTitle("Occourrency [#]");
  hEdgesD->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hEdgesD->Write();
  delete hEdgesD;
  
  // Edge Time Distributuion 
  
  this->TDC_Draw(hTRise,hTFall);
  
  hTRise->Write();
  hTFall->Write();
  
  delete hTRise;
  delete hTFall;


  mycanv->cd(1)->SetLogy(1);
  mycanv->cd(1)->SetLogx(0);
	hTot->SetLineColor(kMagenta);

  hTot->SetTitle("All channel Time Over Threshold");
  hTot->GetXaxis()->SetTitle("Time Over Threshold [ns]");
  hTot->GetYaxis()->SetTitle("Occourrency [#]");
  hTot->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hTot->Write();
  delete hTot;


	// Correlation with Rise

	gStyle->SetOptStat(11111);

  mycanv->cd(1)->SetLogy(0);
  mycanv->cd(1)->SetLogx(0);
  hTFallvsTot->SetTitle("Charge Correlation with Fall");
  hTFallvsTot->GetXaxis()->SetTitle("Time over Threshold [ns]");
  hTFallvsTot->GetYaxis()->SetTitle("Fall edge time [ns]");
  hTFallvsTot->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hTFallvsTot->Draw("COLZ");
  mycanv->Print(fTDCpdf.c_str());	
	hTFallvsTot->Write();
	delete hTFallvsTot;


	// Correlation with Rise
  mycanv->cd(1)->SetLogy(0);
  mycanv->cd(1)->SetLogx(0);
  hTRisevsTot->SetTitle("Charge Correlation with Rise");
  hTRisevsTot->GetXaxis()->SetTitle("Time over Threshold [ns]");
  hTRisevsTot->GetYaxis()->SetTitle("Rising edge time [ns]");
  hTRisevsTot->SetLineColor(kBlue);
  hTRisevsTot->Draw();
  mycanv->Print(fTDCpdf.c_str());
  hTRisevsTot->Draw("COLZ");
  mycanv->Print(fTDCpdf.c_str());
  hTRisevsTot->Write();
  delete hTRisevsTot;

  // Histograms are filled
  // Edge Time Distributuion (single channels)
	
	bool enableSingleChannelprint = true;

  TRich_Adapter adapter;
	
	int bin; 
	int bin_offset;

	int pixel;
	int pixel_offset;

	bool pixel_enable = true; // anodic view instead of electromnic channel
	bool geo_enable = true; // mapmt view 2 D

	int Xpix_offset;
	int Ypix;
	int Xpix;


  for (int i = 0; i<192; i++){
    
		//if(i%64>=32){continue;}

    int Nrise = hTRiseCh[i]->GetEntries();
    int Nfall = hTFallCh[i]->GetEntries();
    int Nhit = hTotCh[i]->GetEntries();
		    
    Double_t erise = 1.*Nrise/totEvts;
    Double_t efall = 1.*Nfall/totEvts;
    Double_t ehit =  1.*Nhit/totEvts;
    
    Double_t trise = hTRiseCh[i]->GetMean();
    Double_t tfall = hTFallCh[i]->GetMean();
    Double_t ttot = hTotCh[i]->GetMean();
		
    Double_t dtrise = hTRiseCh[i]->GetRMS();
    Double_t dtfall = hTFallCh[i]->GetRMS();
    Double_t dttot = hTotCh[i]->GetRMS();
    
    hTRiseCh[i]->SetTitle(Form(			" ASIC%d CH%2d - Pixel %d - Rise Eff %.3lf  -  THR %4d  GAIN %3d ",i/64,i%64,adapter.GetAnode(i%64),erise,thr,gain));
    hTFallCh[i]->SetTitle(Form(			" ASIC%d CH%2d - Pixel %d - Fall Eff %.3lf  -  THR %4d  GAIN %3d ",i/64,i%64,adapter.GetAnode(i%64),efall,thr,gain));
    hTotCh[i]->SetTitle(Form(				" ASIC%d CH%2d - Pixel %d - Hit Eff %.3lf   -  THR %4d  GAIN %3d ",i/64,i%64,adapter.GetAnode(i%64),ehit,thr,gain)); 
		hTRiseWalkCh[i]->SetTitle(Form(	" ASIC%d CH%2d - Pixel %d - Rise Walk %.3lf -  THR %4d  GAIN %3d ",i/64,i%64,adapter.GetAnode(i%64),ehit,thr,gain)); 
		hTFallWalkCh[i]->SetTitle(Form(	" ASIC%d CH%2d - Pixel %d - Fall Walk %.3lf -  THR %4d  GAIN %3d ",i/64,i%64,adapter.GetAnode(i%64),ehit,thr,gain)); 



		if(enableSingleChannelprint){
    	this->TDC_Draw(hTRiseCh[i],hTFallCh[i]);
		}
 
		mycanv->cd(1)->SetGrid(1);

   	if(hTotCh[i]->GetEntries()){
	  	mycanv->cd(1)->SetLogy(1);   
			hTotCh[i]->SetMaximum(100000);
    	hTotCh[i]->SetMinimum(.1);
    	hTotCh[i]->SetLineColor(kMagenta);
			hTotCh[i]->Draw();
			if(enableSingleChannelprint){
				mycanv->Print(Form("%s",fTDCpdf.c_str()));
			}
			// Time Walk
  		mycanv->cd(1)->SetLogy(0);
			hTFallWalkCh[i]->Draw();
			if(enableSingleChannelprint){
				mycanv->Print(Form("%s",fTDCpdf.c_str()));
			}
			hTRiseWalkCh[i]->Draw();
			if(enableSingleChannelprint){
				mycanv->Print(Form("%s",fTDCpdf.c_str()));
			}	
		}

   	// if( Nrise && Nfall){ //	 
		
		pixel = adapter.GetAnode(i%64);
		pixel_offset = (i/64)*64;

		//printf("channel %d -> anode %d ",i,pixel);
		//printf("asic %d ->offset %d \n",i/64,pixel_offset)	;
		
		bin= i;		
		bin_offset = 0;

		// Pixel ID
		if(pixel_enable){
			bin = pixel;
			bin_offset= pixel_offset;
		}

	 	hch[0]->Fill(bin+bin_offset,erise);		
		hch[1]->Fill(bin+bin_offset,efall);
		hch[2]->Fill(bin+bin_offset,trise);
		hch[3]->Fill(bin+bin_offset,tfall);			
		hch[4]->Fill(bin+bin_offset,dtrise);
		hch[5]->Fill(bin+bin_offset,dtfall);
		hch[6]->Fill(bin+bin_offset,ehit);
		hch[7]->Fill(bin+bin_offset,ttot);
		hch[8]->Fill(bin+bin_offset,dttot);

		


		
		if(geo_enable){
		//pixel [1..64] 
			Xpix_offset = i/64*8;
			Ypix =(pixel-1)/8;
			Xpix = (pixel-1)%8;

			printf("channel %3d pixel %2d Y %d X %d (%2d mapmt %d)\n",i,pixel,Ypix,Xpix,Xpix+Xpix_offset,i/64);
		



	 		hpx[0]->Fill(Xpix+Xpix_offset,Ypix,erise);		
			hpx[1]->Fill(Xpix+Xpix_offset,Ypix,efall);
			hpx[2]->Fill(Xpix+Xpix_offset,Ypix,trise);
			hpx[3]->Fill(Xpix+Xpix_offset,Ypix,tfall);			
			hpx[4]->Fill(Xpix+Xpix_offset,Ypix,dtrise);
			hpx[5]->Fill(Xpix+Xpix_offset,Ypix,dtfall);
			hpx[6]->Fill(Xpix+Xpix_offset,Ypix,ehit);
			hpx[7]->Fill(Xpix+Xpix_offset,Ypix,ttot);
			hpx[8]->Fill(Xpix+Xpix_offset,Ypix,dttot);

			hpx[0]->SetTitle("Efficiency Rise");
			hpx[1]->SetTitle("Efficiency Fall");
			hpx[2]->SetTitle("Rise Time");
			hpx[3]->SetTitle("Fall Time");
			hpx[4]->SetTitle("Jitter Rise");
			hpx[5]->SetTitle("Jitter Fall");
			hpx[6]->SetTitle("Efficiency Time Over Threshold");
			hpx[7]->SetTitle("Time Over THreshold (Charge)");
			hpx[8]->SetTitle("Tot REsolution (Charge RMS)");


			hpx[2]->SetMaximum(ltmax);
			hpx[2]->SetMinimum(ltmin);
			hpx[3]->SetMaximum(ltmax);
			hpx[3]->SetMinimum(ltmin);

		}

  	hTRiseCh[i]->Write();
  	hTFallCh[i]->Write();
  	hTotCh[i]->Write();
		hTFallWalkCh[i]->Write();
		hTRiseWalkCh[i]->Write();
			
		if (printStatistics) {
			printf("CH_%d_%02d ",i/64,i%64); 
			printf("RISE %5d %3.0lf%% delay %5.1lf[ns] tts %5.2lf[ns]  ",Nrise,erise*100,trise,dtrise); 
			//		printf("        "); 
			printf("FALL %5d %3.0lf%% delay %5.1lf[ns] tts %5.2lf[ns] ", Nfall,efall*100,tfall,dtfall);		

			printf("WIDTH %5d %3.0lf%% delay %5.1lf[ns] tts %5.2lf[ns] ", Nhit,ehit*100,ttot,dttot);				
			printf("\n");
		}
    if(hTRiseCh[i]){delete hTRiseCh[i];}
    if(hTFallCh[i]){delete hTFallCh[i];}
    if(hTotCh[i]){delete hTotCh[i];}
    if(hTRiseWalkCh[i]){delete hTRiseWalkCh[i];}
    if(hTFallWalkCh[i]){delete hTFallWalkCh[i];}

    //}else {
			//printf("Warning in %s: Edge distribution missing ch %d\n",__FUNCTION__,i);
		//}

  }
	
	// SUMMARY PLOTS
  mycanv->cd(1)->SetLogy(0);
  mycanv->cd(1)->SetLogx(0);
  mycanv->cd(1)->SetGrid(1);
	
	for (int i=0; i<nhisto; i++) {
		if(hch[i]){
			hch[i]->SetStats(0);
			if(pixel_enable){
				hch[i]->GetXaxis()->SetTitle("Pixel [0..63][0..63][0..63]");
			}else{
				hch[i]->GetXaxis()->SetTitle("Channel [0..192]");
			}			
			if(i%2==1&&i<6){
				hch[i]->SetLineColor(kRed);
			}
			if(i>=6){
				hch[i]->SetLineColor(kMagenta);
			}
		}
	}
	
	// Efficiency
 // hch[0]->SetMaximum(4.5);
	hch[0]->GetYaxis()->SetTitle("N Edges /Tot Events");
  hch[0]->SetTitle("Efficiency");
	hch[0]->Draw();
	hch[1]->Draw("SAME");


	mycanv->Print(Form("%s",fTDCpdf.c_str()));
	
	// Mean Delay
//	hch[2]->SetMaximum(timmax-timmin);
	hch[2]->GetYaxis()->SetTitle("Edge Time Average  [ns]");
  hch[2]->SetTitle("Time Delay");
	hch[2]->Draw();
	hch[3]->Draw("SAME");
	mycanv->Print(Form("%s",fTDCpdf.c_str()));
	
	// Jitter
//	hch[4]->SetMaximum(5);
	hch[4]->GetYaxis()->SetTitle("Edge Time RMS [ns]");
  hch[4]->SetTitle("Time Resolution");
	hch[4]->Draw();
	hch[5]->Draw("SAME");
	mycanv->Print(Form("%s",fTDCpdf.c_str()));
  	

		// Efficiency Duration
  //	hch[6]->SetMaximum(4.5);
	hch[6]->GetYaxis()->SetTitle("N Durations /Tot Events");
  hch[6]->SetTitle("Hit Efficiency");
	hch[6]->Draw();
	mycanv->Print(Form("%s",fTDCpdf.c_str()));
	
	// Mean Duration
//	hch[7]->SetMaximum(timmax-timmin);
	hch[7]->GetYaxis()->SetTitle("Mean Duration [ns]");
  hch[7]->SetTitle("Time over Threshold ");
	hch[7]->Draw();
	mycanv->Print(Form("%s",fTDCpdf.c_str()));
	
	// Duration rm
	//hch[8]->SetMaximum(5);
	hch[8]->GetYaxis()->SetTitle("RMS Duration[ns]");
  hch[8]->SetTitle("Time over Threshold RMS");
	hch[8]->Draw();
	mycanv->Print(Form("%s",fTDCpdf.c_str()));


	for (int i=0; i<nhisto; i++) {
		if(hch[i]){
			delete hch[i];
		}
		if(hpx[i]){
  gStyle->SetOptStat(0);	 
			hpx[i]->Draw("COLZ");
			mycanv->Print(Form("%s",fTDCpdf.c_str()));
			delete hpx[i];
		}
	}
	
   // close .pdf 
  mycanv->Print(Form("%s]",fTDCpdf.c_str()));
  delete mycanv;
  
  
  // CLOSE FILE
  lfile->Close();	
  delete lfile;	



	//printf("Analysis completed successfully\n");
	printf("Out Data  %s ",fioTDCroot.c_str());
	printf("\n");
	printf("Out Plots %s ",fTDCpdf.c_str());
	printf("\n");

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
		// look for redorded edges 
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

