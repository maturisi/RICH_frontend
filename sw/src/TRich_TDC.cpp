#define __STDC_FORMAT_MACROS
#include <stdio.h> // printf
#include <stdint.h>  // uint64_t
#include <inttypes.h>  // PRIu64
#include <iostream>

#include <TROOT.h>

#include "TRich_TDC.h"

	
TRich_TDC::TRich_TDC(const char * outname)
{
	Reset();

	ftotEvents = 0;

	ffile= NULL;
	ftree = NULL;
	

	// Initialize the ROOT file and ROOT tree
	if (!outname) {
		//printf("Warning: arg is empty, no root will be created\n");
	}else {
		
		ffile= new TFile(outname, "RECREATE"); if(ffile->IsZombie()){printf("%s is Zombie!\n",outname);}
		ftree= new TTree("TDCdata","Single Run TDC data");
		
		ftree->Branch("triggerID",&ftid		,"tid/i");		
		ftree->Branch("deviceID" ,&fdid		,"did/i");		
		ftree->Branch("tstamp0"	 ,&fts0		,"tstamp0/i");	
		ftree->Branch("tstamp1"	 ,&fts1		,"tstamp1/i");	
		ftree->Branch("nedge"	 ,&fnedge	,"nedge/i");	
		
		ftree->Branch("polarity" ,fpolarity	,"polarity[nedge]/i");	
		ftree->Branch("channel"	 ,fchannel	,"channel[nedge]/i");	
		ftree->Branch("time"	 ,ftime	,"time[nedge]/i");	
	}
}

TRich_TDC::~TRich_TDC()
{

	if(ftree) delete ftree;

	if(ffile->IsOpen()){ffile->Close();delete ffile;}

}

void	TRich_TDC::Write(){
	ftree->Write(); 

}


unsigned int TRich_TDC::TrigNum( int tid){
	
	if (tid!=-1) {// setter
		ftid = (unsigned int) tid;
	}
	return ftid;
}
unsigned int TRich_TDC::DevId( int did){
	
	if (did!=-1) {// setter
		fdid = (unsigned int) did;
	}
	return fdid;
}

unsigned int TRich_TDC::Tstamp(char tag,int time){
	
	bool p = false; // print enable
	unsigned int ret = 666;
	if (time!=-1) {  
		switch (tag) {
			case 0: fts0 = time; ret = fts0; break;
			case 1: fts1 = time; ret = fts0; break;
			default: if(p) {printf("Error in %s (Setter Mode): unknown tag_idx %d\n",__FUNCTION__,tag);}
				break;
		}
	}else {
		switch (tag) {
			case 0: ret = fts0;break;
			case 1: ret = fts1;break;
			default: printf("Error in %s (Getter Mode): unknown tag_idx %d\n",__FUNCTION__,tag);break;
		}
	}
	return ret;
}


void TRich_TDC::Reset()
{	
	ftid=0;	
	fdid=0;	
	fts0=0;
	fts1=0;
	fts_sec = 0.0;
	
	fnegc =0;
	fposc =0;
	
	fnedge = 0;

	
	fEdgeList.clear();
	
	for (int i =0; i<MAX_EDGE; i++) {
		fpolarity[i]=0;
		fchannel[i]=0;
		ftime[i]=0;
	}
}

double TRich_TDC::DecodeTimeStamp(){
	
	uint64_t ltstamp1 = fts1;
	uint64_t ltstamp0 = fts0;
	
	//printf(" TIME[1] =%" PRIu64 " \n",ltstamp1); // example of printing ULong64_t
	//printf(" TIME[0] =%" PRIu64 " \n",ltstamp0); // example of printing ULong64_t	
	uint64_t timestamp = (ltstamp1<<24) + ltstamp0;	// in clock ticks		
	uint64_t timestamp_ns = timestamp<<3; // 8 ns clock
	fts_sec = timestamp_ns/1000000000.;	// expressed in second
	//	printf(" TIME[1] =%" PRIu64 " \n",ltstamp1); // example of printing ULong64_t
	return fts_sec;
}




void TRich_TDC::Edge(unsigned int chan,unsigned int time, unsigned int edge){
	
	RICH_tdc_edge_t lEdge;
	
	lEdge.chan = chan;
	lEdge.time = time;
	lEdge.edge = edge;
	
	// add it to the list
	fEdgeList.push_back(lEdge);

	// debug
	//printf("(%d ,%d, %d)\n",lEdge.chan,lEdge.time,lEdge.edge);
	//std::cout << "size: " << (int) fEdgeList.size() << '\n';
	//std::cout << "capacity: " << (int) fEdgeList.capacity() << '\n';
	//std::cout << "max_size: " << (unsigned int) fEdgeList.max_size() << '\n';


	// update nedge for this event
	int size = (int)fEdgeList.size();
  fnedge = (UInt_t)size;

	// update pos/neg edge counters	for this event
	switch (edge) {
		case 0:fposc++;break;	
		case 1:fnegc++;break;
		default:
			printf("Error edge ==2 !!\n");
			break;
	}
}


void	TRich_TDC::Print(	int verbosity, unsigned int channel){

	if (verbosity == 0) return;

	this->DecodeTimeStamp(); // set fts_sec

	printf("EventID=%d ",ftid);
  printf("TIME=%lf ",fts_sec);
  printf("EDGES=%3d ",fnedge);
  //printf("(rising %3d ,falling %3d )",fposc,fnegc);// useful only while parsing
	//printf("Tree entry = %d",ftotEvents); // useful only while parsing
  printf("\n");

	if (verbosity == 1) return;
	

	for(int i=0;i<(int)fnedge;i++){
		if(fchannel[i]==channel||channel>=192){
			printf("\t%3d) CH%4d ",i+1,fchannel[i]);  
			if(fpolarity[i]==0) printf(" RISE %4d \n",ftime[i]); else  printf(" FALL %4d \n",ftime[i]);  
		}
	}	
}


void 	TRich_TDC::EdgeList(){


	// Event ID select
	unsigned int curr = ftid;
	unsigned int skip = 2; // Trigger ID 0 and 1 are skipped
	if(curr<skip) return;

	// Set the edge list variables associated with the tree
	for (unsigned int i=0; i<fnedge; i++){ 
		fpolarity[i]=fEdgeList[i].edge;
		fchannel[i]=fEdgeList[i].chan;
		ftime[i]=fEdgeList[i].time;
	}
}


int	TRich_TDC::Fill(){


	// now the new entry is ready to be filled in the tree
	// you must have set all the variables associated eith the tree using the specific method of this class

	int Nbyte = ftree->Fill(); 

	if(Nbyte<=0) printf("Error during Filling Tree. Fill returned %d\n",Nbyte);

	ftotEvents++;
	
	// Reset variables associated with the tree
	Reset();

	return Nbyte;
}

unsigned int TRich_TDC::NEdges(){

	return fnedge;
}


unsigned int TRich_TDC::Polarity(unsigned int edge){


	if (edge<fnedge) return fpolarity[edge];

	return 2; // only 0 and 1 are allowed
}

unsigned int TRich_TDC::Channel(unsigned int edge){


	if (edge<fnedge) return fchannel[edge];

	return 192; // only [0,1,191] are allowed
}

unsigned int TRich_TDC::Time(unsigned int edge){


	if (edge<fnedge) return ftime[edge];


	return 8192; // only [0.8191] are allowed (to be confirmed...)
}























// TO DO: rewrite the code in half lines!
// 
// (?)when identification is done the function returns (check that all the data you wants are with you ;-)
// raccogliere il print

int	TRich_TDC::Process(){

  bool p =true;

	if(p){printf("EVTID %5d ",ftid);}

	unsigned int eventtoskip=2; // Trigger ID 0 and 1 are skipped

	if(ftid<eventtoskip){
		if(p){printf("First %d events are skipped\n",eventtoskip);}
		return 0;
	}

	// Get the size of the list
  int size = (int)fEdgeList.size();
  fnedge = (UInt_t)size;
  
	// Print Event Info
  if (p) {
		this->DecodeTimeStamp();
    printf("TIME %lf ",fts_sec);
    printf("EDGES %3d ",size);
	}

  if (size>0)
	{ 

      
      // Edge list 
			if(p){printf("\n\t\t\t\t\t EDGE LIST\n");}
      for (int i=0; i<size; i++){ 
				fpolarity[i]=fEdgeList[i].edge;
				fchannel[i]=fEdgeList[i].chan;
				ftime[i]=fEdgeList[i].time;
				if(p){
	  			//printf("\tedge %3d) %4d, %4d,%4d\n",i,fEdgeList[i].chan,fEdgeList[i].time,fEdgeList[i].edge);
				printf("\t\t\t\t\t %3d) CH%4d ",i+1,fchannel[i]);  
					if(fpolarity[i]==0){
						printf(" RISE %4d \n",ftime[i]);  
					}else{
						printf(" FALL %4d \n",ftime[i]);  
					}
				}
			}	
      
      // Loop on channels
      

      int lepos[192]; // rising edge counters 
      int leneg[192]; 
      unsigned int ch;
      int letype; // type of edge rising =0, falling  =1
      for(ch=0;ch<192;ch++){lepos[ch]=0;leneg[ch]=0;}
      
      for (int i=0; i<size; i++){
				letype = fEdgeList[i].edge;
				ch = fEdgeList[i].chan;
				switch (letype) {
				case 0: // rising edge
	  			lepos[ch]++;
	  			break;
				case 1: // negative edge
	  			leneg[ch]++;
	  			break;
				default:
	  			printf("Error: Unknown Edge");
	  			break;
				}      
			}
      
      // Loop on channels - Hit "reconstruction" 
      // 
      // good hits have rising- falling sequence of edge
      // the other sequences FR,RR,FF are tagged
      // for good hits duration is calculated (as the TIME difference between edges)
      // 
      // TO DO: validate (the logic) and export to higher level hit description
      
			if(p){printf("\n\t\t\t\t\t CHANNEL LIST\n");}

      int lepair; // a pair of edges 		
      for(ch=0;ch<192;ch++){
				lepair = lepos[ch]+leneg[ch];
				if(lepair!=0) // the channel is active in this event  
	  		{ 
					if(p){
							printf("\t\t\t\t\t    CH%4d ",ch);							
							printf("NRISE %4d  NFALL%4d  ",lepos[ch],leneg[ch]);
					}		
					if ((lepair%2)||lepos[ch]-leneg[ch]!=0) // ODD EVENTS
					{
						if(p){printf("noisy  ");}
					}else {
					if (lepair<=2)  // SINGLE HIT
					{
						if(p){printf("single hit  ");}
						
						// take the duration as tfall- trise
						int trise = 0;
						int tfall = 0;
						int duration = 0;
						
						// scan the edge list to find all edges related to ch
						for (int i=0; i<size; i++)
						{
							if (fEdgeList[i].chan==ch) 
							{
								letype = fEdgeList[i].edge;
								
								switch (letype) {
									case 0: // rising edge
										trise = fEdgeList[i].time;
										break;
									case 1: // negative edge
										tfall = fEdgeList[i].time;;
										break;
									default:
										printf("Error: Unknown Edge");
										break;
								}
								// put duration calculation here!
							}
						}
						
						duration = tfall-trise;
	
					if(p){printf(" WIDTH %4d ",duration);}
					
						//if(p){printf(" Rise %4d Fall %4d ",trise,tfall);}
					
						if (duration<=0) {
						//	if(p){printf(" duration %d inverted pulse",-duration);}
						}else {
						//	if(p){printf("duration %d",duration);}
						}
					}else {
						if(p){printf("multiple hit");}
					}
				}
				if(p){printf("\n");}
			}else {
				// the channels has no hit
			}
		}
		//printf("\n");
	}else { // size = 0
		if(p){printf(" NO HITS\n");}
	}
	if(p){printf("\n");}

	int Nbyte = ftree->Fill(); 
	ftotEvents++;
		if(p){printf("Filled Events = %d\n",ftotEvents);}

	if(Nbyte<=0){printf("Error during Filling Tree. Fill returned %d\n",Nbyte);}
	Reset();
	
	return 0;
}



bool TRich_TDC::OpenFile(const char * filename){

	if(filename==NULL) return false;


	ffile = new TFile(filename,"READ"); 
	
	if (!ffile->IsOpen()){
		printf(" File %s not found, please call TRich_Analysis::Ingest()\n",filename); 
		return false;
	}else{ 
		//printf(" File %s opened\n",filename);	
		return true;
	}
}

int TRich_TDC::RetrieveTree(){

	int nentries = 0;

	if(!ffile){
		printf("Sorry I need an open file to retrieve the tree, please call TRich_TDC::OpenFile before!\n");
		return nentries;
	}

  ftree = (TTree*)ffile->Get("TDCdata"); // better using a macro in general header
  nentries = ftree->GetEntries();


  ftree->SetBranchAddress("triggerID"	,&ftid);
  ftree->SetBranchAddress("tstamp0"		,&fts0);
  ftree->SetBranchAddress("tstamp1"		,&fts1);
  ftree->SetBranchAddress("nedge"			,&fnedge);
  ftree->SetBranchAddress("polarity"	,fpolarity);
  ftree->SetBranchAddress("channel"		,fchannel);
  ftree->SetBranchAddress("time"			,ftime);



	return nentries;
}
bool TRich_TDC::GetEntry(int entryID){

	this->Reset();

	ftree->GetEntry(entryID);

	this->DecodeTimeStamp();

	return true;
}



