
/**
 * main.cpp
 * 
 * analysis of Slow Shaper waveform
 *
 * Author:Matteo Turisini
 * Date: 2015 October 10th
 */


#include <iostream>  
#include <fstream>   
#include <stdlib.h>
   

#include <TFile.h>   
#include <TTree.h>  
#include <TCanvas.h>  

int ReadWaveform(std::string,int,int,int,int**,int**);
/*
 * Reconstruct the single channel waveform 
 */

void ReadSettings(std::string,int *, int *, int *);
void ReadDelay(std::string,int *);	
/*
 * Parse the file name to extract MAROC gain, RCBuffer and slow shaper settings 
 */

int main(int argc,char *argv[]){

	std::string fnList = "./filelist.txt"; 
	printf("Reading File List from %s\n",fnList.c_str());		

	std::ifstream sList;
	std::string filename; 
	std::string path = "../data/";
	int filec = 0;
	sList.open(fnList.c_str());
  
  if (!sList.good()) {
		printf("Error in %s: File %s not found...exit\n",__FUNCTION__,fnList.c_str());
    return -1; 
  }
	

	int nsamples = 80;
	int nchannels = 192;	
	int buff,shap,gain,trigdelay;	
	int asic, channel;
	int * adc = new int[nsamples];
	int * delay = new int[nsamples];
	

	std::string fileroot = "./adcwaveform.root";
	TFile * ffile = new TFile(fileroot.c_str(),"RECREATE");
	TTree * ftree;
	if (ffile->IsOpen()) {
		ftree = new TTree("data"," Slow shaper waveform");
		
		ftree->Branch("asic",&asic,"asic/I");		
		ftree->Branch("channel",&channel,"channel/I");	
		ftree->Branch("buff",&buff,"buff/I");	
		ftree->Branch("shap",&shap,"shap/I");	
		ftree->Branch("gain",&gain,"gain/I");	
		ftree->Branch("trigdelay",&trigdelay,"trigdelay/I");	
		ftree->Branch("adc",adc,"adc[80]/I");
		ftree->Branch("delay",delay,"delay[80]/I");	
		
	}
	
	
	while (1) { // LOOP ON FILES
		if (sList.eof()){break;}
		sList>>filename;

		filec++;
		
		std::string file = path+filename;

		printf("%2d) %s\n ",filec,filename.c_str());
		for (int ch=1; ch<=nchannels; ch++) { // ch range is [1..192] in case of 3ASIC board
			
			// RESET
			asic = 0;
			channel=0;
			buff =0;
			shap =0;
			gain =0;
			trigdelay =0;
			for (int i=0; i<nsamples; i++) {
				adc[i]=0;
				delay[i]=0;
			}			
			
			//ReadSettings(filename,&buff,&shap,&gain);	
			ReadDelay(filename,&trigdelay);		
			
					
			asic = (ch-1)/64;
			channel = (ch-1)%64;

			int ret =  ReadWaveform(file,nsamples,nchannels,ch,&adc,&delay);
			
			/*
			printf("asic %d ",asic);
			printf("ch %2d ", channel);
			printf("buff %4d ",buff);
			printf("shap %4d ",shap);
			printf("gain %3d ",gain);
			printf("trigdelay %3d ",trigdelay);		
			printf("sample[%d] %3d ",delay[0],adc[0]);		
			printf("sample[%d] %3d ",delay[nsamples-1],adc[nsamples-1]);		
			printf("\n");			
*/
			ftree->Fill();
			
			//for (int i=0; i<nsamples; i++) {
			//	printf("sample %d delay %d adc %d\n",i,delay[i],adc[i]);
			//}

			if (ret<0) {
				break;
			}
		}
	}
	sList.close();
	printf("data in %s \n",fileroot.c_str());
	
	ftree->Write();
	//	ftree->Print();
	//	ftree->Scan();
	
	delete ftree;
	ffile->Close();
	delete ffile;
	
	return 0;
}
		
void ReadDelay(std::string filename, int * ptrigdelay){
	
	
	std::string trigdelay_str = filename.substr(7,3);

	//printf("TRIG DLY %s\n ",trigdelay_str.c_str());
	
	*ptrigdelay = atoi(trigdelay_str.c_str());
	
}			



void ReadSettings(std::string filename, int * pbuff,int * pshap,int *pgain){
	
	
	std::string buff_str = filename.substr(9,4);
	std::string shap_str = filename.substr(22,4);
	std::string gain_str = filename.substr(31,3);

	//printf("BUFF %s ",buff_str.c_str());
	//printf("SHAP %s ",shap_str.c_str());
	//printf("GAIN %s ",gain_str.c_str());
	
	*pbuff = atoi(buff_str.c_str());
	*pshap = atoi(shap_str.c_str());
	*pgain = atoi(gain_str.c_str());
	
}


int ReadWaveform(std::string filename,int nsamples,int nchannels,int selected_channel,int **adc,int ** delay)
{
	std::ifstream fin; // single file data stream
	fin.open(filename.c_str());
	if (!fin.good()) {
		printf("Error in %s: File %s not found...exit\n",__FUNCTION__,filename.c_str());
		return -1; 
	}
	int myint;
	int hold1_delay = -1;
	for (int j=0; j<nsamples; j++) {
		fin>>myint; // first column measurementID is skipped
		fin>>hold1_delay; // the second column contains hold1_delay

		for (int i=0; i<nchannels; i++) {
			fin>>myint;	// skip all other  channels
			if (i==selected_channel-1) {
				(*adc)[j]=myint; 
				(*delay)[j]=hold1_delay;
			}
		}
	}
	fin.close();
}
