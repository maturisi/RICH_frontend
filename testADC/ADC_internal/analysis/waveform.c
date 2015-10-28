//#include <TCanvas.h> 
void waveform(int asic)
{
	int size = 900;
	TCanvas * mycanv = new TCanvas("mycanv","Internal ADC - Triggered by MRC 1",4096-size,0,size,size);
	mycanv->Divide(8,8);
	for(int i =0; i<64;i++){
		mycanv->cd(i+1);
		data->Draw("adc:delay",Form("asic==%d&&channel==%d&&trigdelay==0",asic,i),"L");
	}
}

void waveform(int asic, int channel)
{
	int size = 900;
	TCanvas * mycanv = new TCanvas("mycanv","Internal ADC - Triggered by MRC 1",4096-size,0,size,size);
	data->Draw("adc:delay",Form("asic==%d&&channel==%d&&trigdelay==0",asic,channel),"L");
}


