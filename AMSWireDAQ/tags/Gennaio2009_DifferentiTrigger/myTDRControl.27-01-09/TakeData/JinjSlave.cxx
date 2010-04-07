#include <sys/time.h>
#include <sys/stat.h> 
#include <cstring> // needed to add for gcc 4.3
#include "JinjSlave.h"
#include "PUtil.h"


ConfPars::ConfPars() {
  //  logfile=log;
	type=0; //0 = Jinf, 1= JLV1
  Ntdrs=NTDRS;
  delay=0x96; // units of 20 ns --> 0x96= 3 mu sec //Non sò se ci sarà un qualche parametro di questo tipo sul JLV1, magari la frequenza del generatore...
  JINFflash=0;
  TDRflash=0; 
	JLV1flash=0;
  memset(mode,0,sizeof(mode));
  sprintf(DATAPATH,"");
  sprintf(CALPATH,"");
  refmask=0;
	memset(SLowTrash,0,sizeof(SLowTrash));
	memset(SHighTrash,0,sizeof(SHighTrash));
	memset(KLowTrash,0,sizeof(KLowTrash));
	memset(KHighTrash,0,sizeof(KHighTrash));
	memset(SigmaRowTrash,0,sizeof(SigmaRowTrash));
	memset(DeltaTdoubTrig,0,sizeof(DeltaTdoubTrig[NTDRS]));
}

//----------------------------------------------------------
int ConfPars::ReadConfig(char * conf_file) {
  int ret=0;
  char dummy[100];
  FILE *file=fopen(conf_file,"r");

  if (file==NULL) {
    PRINTF("ERROR: The configuration file %s was not found, I need this file\n", conf_file);
    return 1;
  }


  fscanf(file,"%s  %s",&dummy,&DATAPATH);
  fscanf(file,"%s  %s",&dummy,&CALPATH);
	
	if (strstr(conf_file,"JLV1")){
		type=1;
		fscanf(file,"%s %hx  ",&dummy, &JLV1flash);
		fscanf(file,"%s %d  ",&dummy, &delay);
		delay/=20;
  
		LPRINTF("JLV1 program: 0x%04x\n", JLV1flash);
	}

	else if (strstr(conf_file,"JINF")){
		type=0;
		fscanf(file,"%s %hx  ",&dummy, &JINFflash);
		fscanf(file,"%s %hx  ",&dummy, &TDRflash);
		fscanf(file,"%s %d  ",&dummy, &delay);
		delay/=20;
		fscanf(file,"%s %s %s %s %s %s %s %s %s",&dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy);
  
		int a, tmp, inutile;
		for (int tdr=0; tdr<NTDRS; tdr++) {
//		printf("%d %d\n",refmask,tdr);//only for debug
			inutile = refmask;
			fscanf(file, "%d %d %d %d %d %d %d %d %d", &a, &tmp, &mode[tdr], &SLowTrash[tdr], &SHighTrash[tdr]
						, &KLowTrash[tdr], &KHighTrash[tdr], &SigmaRowTrash[tdr], &DeltaTdoubTrig[tdr]);
//			printf("SLowTrash: %d, SHighTrash: %d, KLowTrash: %d, KHighTrash: %d, SigmaRowTrash: %d\n",
//						SLowTrash[tdr], SHighTrash[tdr], KLowTrash[tdr], KHighTrash[tdr], SigmaRowTrash[tdr], DeltaTdoubTrig[tdr]);//only for debug
			refmask = inutile;
//		printf("Tdr number %d : %d, %d; refmask : %d\n",a,tmp,mode[tdr],refmask);//only for debug    
			if(tmp) refmask|=1<<tdr;
			tmp=0;
		}
  
		LPRINTF("TDR program: 0x%04x\n", TDRflash);
		LPRINTF("JINF program: 0x%04x\n", JINFflash);

  }

	else PRINTF("The configuration file isn't named with 'JLV1' or 'JINF'\n");

  fclose(file);
  return ret;
}

//--------------------------------------------------------------
JinjSlave::JinjSlave(char* name,char* conf_file,int address,AMSWcom* node_in){
	  //  logfile=log;
  sprintf(myname,"%s",name);
  selfaddress=address;
  if(selfaddress==0xffff) broadcastadd=0x4000;
  else broadcastadd=(0x4000<<16)|selfaddress;
  node=node_in;
  CPars= new ConfPars();
  CPars->ReadConfig(conf_file);
  mask.Nmask=0;
  for (int ii=0;ii<16;ii++) mask.ID[ii]=0; 
	jlv1events=0;
	jinfevents=0;
	for (int ii=0;ii<NTDRS;ii++)tdrevents[ii]=0;
}
//--------------------------------------------------------------
JinjSlave::~JinjSlave(){

  if(CPars) delete CPars;
}
