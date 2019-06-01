#include "ChipsetDriver.h"

OSErr getNRProp(RegEntryIDPtr node, char* propName, RegPropertyValueSize propSize, void* prop){
	RegPropertyValueSize size;
	OSErr err;
	if(!prop || !propName || !propSize){
		return -1;
	}
	err = RegistryPropertyGetSize(node, propName, &size);
	if(err!=noErr){
		return err;
	}
	if(size!=propSize){
		return paramErr;
	}
	err = RegistryPropertyGet(node, propName, prop, &size);
	return err;
}