#include "TRich_Calib.h"

void TRich_Calib::Reset(){

 	frunID=0;
	fNtrig=0;
	fPolarDiscri=0;

	for(int asic=0;asic<NASIC;asic++)
	{
		fThreshold[asic]=0;
	}

	for(int ch=0;ch<NCH;ch++)
	{
		fGain[ch]  = 0;
		fNRise[ch] = 0.0; 
		fNFall[ch] = 0.0;
		fTRise[ch] = 0.0; 
		fTFall[ch] = 0.0;
		fJRise[ch] = 0.0; 	
		fJFall[ch] = 0.0;	
		fNDura[ch] = 0.0;
		fTDura[ch] = 0.0;
		fJDura[ch] = 0.0; 
	}
}
void	TRich_Calib::Print(){
	
	printf("RUN %4d: ",frunID);
	printf("NTrig %6d ",fNtrig);
	
	printf("Thresholds = (");
	for(int asic=0;asic<NASIC-1;asic++) printf("%3d,",fThreshold[asic]);
	printf("%3d)\n",fThreshold[NASIC-1]);	
/*
	for(int ch=0;ch<NCH;ch++)
	{
		//printf("%d ",fGain[ch]);

		//printf("%6.0lf ",fNRise[ch]);
		printf("%6.0lf ",fNFall[ch]);
		//printf("%6.0lf ",fNDura[ch]);


	//	printf("%6.3lf ",fTRise[ch]);
	//	printf("%6.3lf ",fTFall[ch]);
	//	printf("%6.3lf ",fTDura[ch]);

	//	printf("%6.3lf ",fJRise[ch]);
	//	printf("%6.3lf ",fJFall[ch]);
	//	printf("%6.3lf ",fJDura[ch]);



		if(ch%8==7)printf("\n");
		if(ch%64==63)printf("\n");	
	}
	printf("\n");
*/

}

int 	TRich_Calib::Gain(int val, int ch){

	if(val<=0) // getter
	{
		if(ValidChannel(ch)) return fGain[ch]; 
		
		else{printf("Warning in %s: invalid channel %d \n",__FUNCTION__,ch); return fGain[0];} 

	}else{

		if(ValidChannel(ch)) {fGain[ch]=val; return fGain[ch];} // setter single channel

		else{for(int ch=0;ch<NCH;ch++) {fGain[ch] =val;} return fGain[0];} // setter all channels (with the same value)
	}
}



int 	TRich_Calib::Threshold(int val,int  asic)
{
	if(val<0) // getter
	{
		if(ValidAsic(asic)){return fThreshold[asic];} 
		
		else{printf("Warning in %s: invalid asic %d \n",__FUNCTION__,asic); return -1;} 

	}else{ // setter

		if(ValidAsic(asic)){
			 fThreshold[asic]=val; return fThreshold[asic];
		} 

		else{
			for(int i=0;i<NASIC;i++) {
				fThreshold[i] =val;
			}
		return fThreshold[0];} 
	}
} 

int 	TRich_Calib::NTrigger(int val){
	
	if(val>=0){fNtrig = val;}
	
	return fNtrig;
}

int 	TRich_Calib::RunID(int val){
	
	if(val>=0){frunID = val;}
	
	return frunID;
}

double TRich_Calib::Entries(int what, double val, int ch)
{
	if(!ValidChannel(ch)){printf("Error: Channel not in range [0.191]\n"); return -1.0;}
	
	double ret = -2.0;

	switch(what){
		case 0: if(val>=0){fNRise[ch] = val;} ret = fNRise[ch]; break;
		case 1: if(val>=0){fNFall[ch] = val;} ret = fNFall[ch]; break;
		case 2: if(val>=0){fNDura[ch] = val;} ret = fNDura[ch]; break;
		default : printf("Error in %s: Invalid choice\n",__FUNCTION__);break;
	}
	return ret;
}


double TRich_Calib::Mean(int what, double val, int ch)
{
	if(!ValidChannel(ch)){printf("Error: Channel not in range [0.191]\n"); return -1.0;}
	
	double ret = -2.0;

	switch(what){
		case 0: if(val>=0){fTRise[ch] = val;} ret = fTRise[ch]; break;
		case 1: if(val>=0){fTFall[ch] = val;} ret = fTFall[ch]; break;
		case 2: if(val>=0){fTDura[ch] = val;} ret = fTDura[ch]; break;
		default : printf("Error in %s: Invalid choice\n",__FUNCTION__);break;
	}
	return ret;
}


double TRich_Calib::RMS(int what, double val, int ch)
{
	if(!ValidChannel(ch)){printf("Error: Channel not in range [0.191]\n"); return -1.0;}
	
	double ret = -2.0;

	switch(what){
		case 0: if(val>=0){fJRise[ch] = val;} ret = fJRise[ch]; break;
		case 1: if(val>=0){fJFall[ch] = val;} ret = fJFall[ch]; break;
		case 2: if(val>=0){fJDura[ch] = val;} ret = fJDura[ch]; break;
		default : printf("Error in %s: Invalid choice\n",__FUNCTION__);break;
	}
	return ret;


}


void TRich_Calib::Fill(){

	fTree->Fill();
}

void TRich_Calib::Write(){

	fFile->cd();
	fTree->Write();
}

TRich_Calib::TRich_Calib(const char * filename)
{

	if(filename){ // writing mode
		printf("Filename = %s\n",filename);

		fFile = NULL;
		fTree = NULL;

		fFile = new TFile(filename,"RECREATE");

		if(!fFile->IsOpen()){
			printf("Error in %s: file %s not opened\n",__FUNCTION__,filename);
		}
		else{ // let's create a tree
			fTree = new TTree(CALTREENAME,"Calibration data, threshold gain scan");
			// associate variables to the tree
			fTree->Branch("runID", &frunID, "frunID/I");
			fTree->Branch("ntrig", &fNtrig, "fNtrig/I");
			fTree->Branch("thr", fThreshold,"fThreshold[3]/I");
			fTree->Branch("gain", fGain,"fGain[192]/I");
		
			fTree->Branch("nRise", fNRise,"fNRise[192]/D");
			fTree->Branch("nFall", fNFall,"fNFall[192]/D");
			fTree->Branch("nDura", fNDura,"fNDura[192]/D");

			fTree->Branch("tRise", fTRise,"fTRise[192]/D");
			fTree->Branch("tFall", fTFall,"fTFall[192]/D");
			fTree->Branch("tDura", fTDura,"fTDura[192]/D");

			fTree->Branch("jRise", fJRise,"fJRise[192]/D");
			fTree->Branch("jFall", fJFall,"fJFall[192]/D");
			fTree->Branch("jDura", fJDura,"fJDura[192]/D");
		}
	}else{ //reading mode

		// do nothing, later use methods like OpenFile and GetTRee (see below)
		//printf("Warning: filename not specified\n");

	}
	this->Reset();	
}

TRich_Calib::~TRich_Calib()
{

	if(fTree) delete fTree;

	if(fFile){
		
		if(fFile->IsOpen())fFile->Close();

		delete fFile;
	}
}

bool TRich_Calib::ValidChannel(int i){

	if (i<0 || i>191 ) return false;
	
	return true;

}
bool TRich_Calib::ValidAsic(int i){

	if (i<0 || i>3 ) return false;
	
	return true;
}


/***********/
/* Reading */
/***********/


bool TRich_Calib::OpenFile(const char * filename){

	if(filename==NULL) return false;

	fFile = new TFile(filename,"READ"); 
	
	if (!fFile->IsOpen()){
		printf(" File %s not found, please call TRich_Analysis::Ingest() in scan mode\n",filename); 
		return false;
	}else{ 
		//printf(" File %s opened\n",filename);	
		return true;
	}

} 
int TRich_Calib::GetTree(){
	
	int nentries = 0;

	if(!fFile){
		printf("Sorry I need an open file to retrieve the tree, please call TRich_TDC::OpenFile before!\n");
		return nentries;
	}

  fTree = (TTree*)fFile->Get(CALTREENAME); 
 
	nentries = fTree->GetEntries();

	fTree->SetBranchAddress("runID", &frunID);
	fTree->SetBranchAddress("ntrig", &fNtrig);
	fTree->SetBranchAddress("thr"	 , fThreshold);
	fTree->SetBranchAddress("gain" , fGain);
	
	fTree->SetBranchAddress("nRise", fNRise);
	fTree->SetBranchAddress("nFall", fNFall);
	fTree->SetBranchAddress("nDura", fNDura);

	fTree->SetBranchAddress("tRise", fTRise);
	fTree->SetBranchAddress("tFall", fTFall);
	fTree->SetBranchAddress("tDura", fTDura);

	fTree->SetBranchAddress("jRise", fJRise);
	fTree->SetBranchAddress("jFall", fJFall);
	fTree->SetBranchAddress("jDura", fJDura);

	return nentries;
}
bool TRich_Calib::GetEntry(int entryID){

	this->Reset();

	fTree->GetEntry(entryID);

	return true;
} 
