#include "ChipsetDriver.h"

OSErr CFMInitialize(){
	return noErr;
}

OSErr CFMTerminate(){
	return noErr;
}

DriverDescription TheDriverDescription = {
	kTheDescriptionSignature,
	kInitialDriverDescriptor,
	{
		"\pci1095,3124",
		1, 0, 0, 0,
	},
	{
		kDriverIsLoadedUponDiscovery
		|kDriverIsOpenedUponLoad,
		"\p.Driver_Chipset_sil3124",
	},
	{
		1,
		{
			kServiceCategoryNdrvDriver,
			kNdrvTypeIsGeneric,
			1, 0, 0, 0,
		}
	}
};

OSErr DoDriverIO (
					AddressSpaceID spaceID,
					IOCommandID ID,
					IOCommandContents contents,
					IOCommandCode code,
					IOCommandKind kind)
{
#pragma unused(spaceID, ID, kind)
	OSErr result;
	switch(code){
		case	kInitializeCommand:
				result = HandleInitialize(contents.initialInfo);
				break;
		case	kFinalizeCommand:
				result = HandleFinalize(contents.finalInfo);
				break;
		default:
				result = noErr;
				break;
	}
	return result;



}
OSErr HandleInitialize(DriverInitInfoPtr initialInfo){
	OSErr err;
	int i;
	if(isInitialized != 0){
		return paramErr;//don't init twice
	}
	
	myRefNum = initialInfo->refNum;
	myDeviceRegID = initialInfo->deviceEntry;
	err |= getNRProp(&myDeviceRegID, kAAPLDeviceLogicalAddress, kChipsetAddressCnt*sizeof(LogicalAddress),(void*)&chipsetVirtAddrs);
	err |= getNRProp(&myDeviceRegID, kPCIAssignedAddressProperty, kChipsetAddressCnt*sizeof(PCIAssignedAddress),(void*)&chipsetPhysAddrs);
	if(err!=noErr){
		PRNT_ERROR_STR("SIL3124_INIT: unable to get assigned-addresses and/or AAPL,address properties\n");
		return paramErr;
	}
	for(i = 0; i < kChipsetAddressCnt; i++){
		switch(chipsetPhysAddrs[i].registerNumber){
			case kChipsetGlobalBase:
				chipsetGlobals = chipsetVirtAddrs[i];
				break;
			case kChipsetPortBase:
				chipsetPorts = chipsetVirtAddrs[i];
				break;
			case kChipsetIOBase:
				chipsetIO = chipsetVirtAddrs[i];
				break;
			case kChipsetROMBase:
				chipsetROM = chipsetVirtAddrs[i];
				break;
			default:
				PRNT_ERROR_STR("SIL3124_INIT: unable to translate assigned address [");
				PRNT_ERROR_HEX(i);
				PRNT_ERROR_STR("]\n");
				return paramErr;
		}
	}
	PRNT_MESSAGE_STR("pci1095,3124 init\n");
	isInitialized = 1;
	return noErr;
}
OSErr HandleFinalize(DriverFinalInfoPtr finalInfo){
#pragma unused(finalInfo)
	isInitialized = 0;
	openCount = 0;
	return noErr;
}
OSErr HandleOpen(ParmBlkPtr pb){
#pragma unused(pb)
	return noErr;
}

