#define CALL_IN_SPOCKETS_BUT_NOT_IN_CARBON 1

#include <Appearance.h>
#include <Sound.h>
#include <DrawSprocket.h>
#include <InputSprocket.h>
#include "input.h"
#include "screen.h"
#include "error.h"
#include "sprites.h"
#include "trig.h"
#include "gamesounds.h"
#include "gameinitexit.h"
#include "interface.h"
#include "preferences.h"
#include "random.h"
#include "packs.h"
#include "register.h"

#if __option(profile)
#include <profiler.h>
#endif


float gSinTab[kSinTabSize];
int gInitSuccessful=false;
int gOSX;

void InitToolbox()
{
/*	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);*/
	InitCursor();
}

#undef sin
void InitTrig()
{
	int i;
	for(i=0;i<kSinTabSize;i++)
		gSinTab[i]=sin(2*PI*(float)i/(float)kSinTabSize);
}

UInt32 U32Version(NumVersion v)
{
	return *((UInt32*)(&v));
}

int ReqCheck()
{
	int hit;
	long resp;
	AlertStdAlertParamRec alertParam={
		false,false,nil,
		"\pExit",
		nil,
		nil,
		kAlertStdAlertOKButton,
		0,
		kWindowDefaultPosition};
	if((Ptr)RegisterAppearanceClient==(Ptr)kUnresolvedCFragSymbolAddress)
	{
		StopAlert(131,nil);
		return false;
	}
	if(U32Version(SndSoundManagerVersion())<=0x03100000)
	{
		StandardAlert(kAlertStopAlert,
		"\pYour Sound Manager version is too old.",
		"\p",
		&alertParam,
		&hit);
		return false;
	}
	DoError(Gestalt(gestaltSystemVersion,&resp));
	gOSX=resp>=0x00001000;
	if(gOSX&resp<0x00001002)
	{
		StandardAlert(kAlertStopAlert,
		"\pTo run Reckless Drivin' under Mac OS X you need at least Mac OS X 10.0.2.",
		"\pUse The Software Update Panel under Sytem preferences to update your system.",
		&alertParam,
		&hit);
		return false;
	}
	return true;	
}

void InitAE();

void Init()
{	
	InitToolbox();
	if(!ReqCheck())
		ExitToShell();
	InitAE();
	gAppResFile=CurResFile();
	//DoError(RegisterAppearanceClient());
	Randomize();
	LoadPrefs();
	CheckRegi();
	if(!gRegistered)
		Register(false);
	InitScreen(0);
	ShowPicScreen(1003);
	LoadPack(kPackSnds);
	LoadPack(kPackObTy);
	LoadPack(kPackOgrp);
	LoadPack(kPackRoad);
	if(gPrefs.hiColor)
	{
		LoadPack(kPacksR16);
		LoadPack(kPackcR16);
		LoadPack(kPackTx16);
	}
	else
	{
		LoadPack(kPacksRLE);
		LoadPack(kPackcRLE);
		LoadPack(kPackTxtR);
	}
	LoadSprites();
	InitTrig();
	InitInput();
	SetGameVolume(-1);
	InitChannels();
	InitInterface();
	#if __option(profile)
	{
		FSSpec spec;
		OSErr err;
		
		err=FSMakeFSSpec(0,0,"\pProfiler Dump",&spec);
		DoError(err==fnfErr?noErr:err);
		err=FSpDelete(&spec);
		DoError(err==fnfErr?noErr:err);
		DoError(ProfilerInit(collectSummary,bestTimeBase,200,5));
	}
	#endif	
	gInitSuccessful=true;
}

void Exit()
{
	#if __option(profile)
	DoError(ProfilerDump("\pProfiler Dump"));
	#endif
	if(gInitSuccessful)
	{
		WritePrefs(false);
		FadeScreen(1);
		ScreenMode(kScreenSuspended);
		SetSystemVolume();
		FadeScreen(256);
		FadeScreen(0);
		ScreenMode(kScreenStopped);
		InputMode(kInputStopped);
	}
	ExitToShell();
}