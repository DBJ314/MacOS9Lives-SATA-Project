#include "ChipsetDriver.h"

DebugLevel dbLevel = kDebugLevelAll;

void debugPrintStr(DebugLevel msgLevel, char* message){
	if(msgLevel>dbLevel){
		return;
	}
	NKPrintf(message);
}

void debugPrintHex(DebugLevel msgLevel, UInt32 message){
	if(msgLevel>dbLevel){
		return;
	}
	NKPrintHex(message);
}

