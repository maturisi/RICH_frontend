/**
 * use makefile to produce executable 
 */

#include <string> // string
#include<iostream> //cerr,endl
#include "TRich_Analysis.h"

#define SCALER_MODE 0
#define TDC_MODE 1

int main(int argc, char *argv[]){

	// parse input 
	for (int i=0; i<argc; i++) { //debug
	 //printf("argv[%d] = %s\n",i,argv[i]);
  }
//printf("argc = %d\n",argc);//debug

	if(argc<=1){
	 printf("not enough arguments please specify ska or tdc\n");
	 return -1;
	}

	int daq_mode = (strcmp("ska", argv[1]) == 0) ? SCALER_MODE:TDC_MODE; 

	string runPrefix;
	int runID_first;
	int runID_last;
	int runID ; 
	int ival;

  	char numstr[7];
	string suffix;
	string run;
	string runPath;
	string Source; // Laser, Dark, Background
	string PlotType; // Rate, Efficiency
	int Norm;// Duration [s], or Laser Rate [Hz] 

  	int repetition;

  	TRich_Analysis A;

  	// this variables are related to the new TDC part
  	// maybe can be merged with the one for SCALER
  	// during the revisione process
  	string runName; // contains path and the entire name like ./data/out/TDC_TEST_000010.txt
  	int thr;
	int gain;

	float minZ,maxZ; // 3D PLOT
	float minY,maxY; // 2D PLOT
	bool laser;

	int thr_step; //DAC0 units

	A.SetDaqMode(daq_mode);

  	switch(daq_mode){
  		case SCALER_MODE:
    			if (argc==1||argc<8){
				printf("Error in %s: few arguments for scaler Analysis...Exit\n",__FUNCTION__);
				printf("Usage Example: ./ana ska prefix first last path Source PlotType Norm\n");
				return -1;
      			}
			runPrefix = argv[2];
			runID_first = atoi(argv[3]);
			runID_last = atoi(argv[4]);
			runPath = argv[5];
    	Source = argv[6]; // Laser, Dark, Background
    	PlotType = argv[7]; // Rate, Efficiency
    	Norm = atoi(argv[8]);// Duration [s], or Laser Rate [Hz]

			printf("%s %s (%d) ",Source.c_str(),PlotType.c_str(),Norm);
			printf("SCALER mode: %s,%5d,%5d ",runPrefix.c_str(),runID_first,runID_last);

			A.SetDaqMode(daq_mode);
			A.SetSource(Source,PlotType,Norm);
			repetition = A.SetNames(runPrefix,runID_first,runID_last,runPath);
			if (repetition<0){return -2;}
			A.Print();
			printf("Reading,"); 
			A.SKA_Read_Threshold_Scan(); 


			// Represent Efficiency [# counts/ Ntrigger] or Rate [Hz]
			laser = (strcmp("Laser", Source.c_str()) == 0) ? true:false;

			if(laser){
				minZ = 0.01;
  			maxZ = 1.1;
				minY =0.01;
				maxY =1.1;
				
			}else{
  			minZ = .1;
  			maxZ = 40*1000*1000;
				minY=0.1;
				maxY=40*1000*1000.;
			}
    	
    	// Single channel plot
    	printf("Plotting2D,");
    	A.SKA_Plot2(minY,maxY);
			
			// Gauguin Plot
			printf("Plotting3D,");
			thr_step = 3; //DAC0 units
			A.SKA_Plot3(minZ,maxZ,thr_step);
   		printf("done\n");
   		break;


		case TDC_MODE:
			
			if (argc<4){
      	printf("Error in %s: few arguments for TDC analysis...Exit\n",__FUNCTION__);
				printf("Usage Example:\n");
				printf("\tSINGLE RUN: ./ana tdc ../../storage/150921_testpulse_discri1/run_000031.txt 31\n");
				printf("\tMULTIRUN : ./ana tdc ../../storage/150921_testpulse_discri1/run_000031.txt 42\n");
      	return -1;
    	}

			runName = argv[2];
			runID_last = atoi(argv[3]);
			runID  = A.NameRun(runName.c_str(),runID_last);
			if(runID==-1) {printf("file doesn't exist\n");break;}			
			if(runID==-2){printf("Error with run ID\n");break;}


			A.Ingest();

			if(A.SingleRun()){

				// processing
				A.TDC_Spectra();// board
				A.TDC_SpectraSingleChannel();// single channel
				
				
				// graphics
				A.Plot();// board
				A.TDC_PlotSingleChannel();// single channel
				A.TDC_Export(); 
				A.TDC_Uniformity(); 	// single channel
				A.TDC_Image(); 				// single channel

			}
			if(A.Scan()){

				// processing
				A.TDC_Scan_Histo();		

				// graphics
				A.TDC_Scan_Plot1();		
				A.TDC_Scan_Plot2(); // Gauguin
				//A.TDC_Scan_Plot3();			
				
			}
			break;
  		default:
    			printf("Error in %s: Unknown daq_mode",__FUNCTION__);
    		break;
  	}
 	return 0;
}

