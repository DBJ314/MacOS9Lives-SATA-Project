#include "Flasher.h"

enum {
	kSIL3124_FADR_L = 0x70,//Flash Address Register Low
	kSIL3124_FADR_M = 0x71,//Flash Address Register Mid
	kSIL3124_FADR_H = 0x72,//Flash Address Register High
	kSIL3124_FADR_C = 0x73,//Flash Address Register Control
	kSIL3124_FDAT	= 0x74,//Flash Data Register
	
	kFADR_GPIO_EN 	= 0x80000000,
	kFADR_MEM_PRSNT = 0x04000000,
	kFADR_MEM_START = 0x02000000,
	kFADR_MEM_READ	= 0x01000000,
	kFADR_ADR_MASK	= 0x0007FFFF,
	
	kCmd_Mem_Map	= 0x00000002,
	kExp_ROM_En		= 0x00000001
};

enum {
	kChipsetGlobalBase	= kPCIConfigBaseAddress0,
	kChipsetPortBase	= kPCIConfigBaseAddress1,
	kChipsetIOBase		= kPCIConfigBaseAddress2,
	kChipsetROMBase		= kPCIConfigExpansionROMBase,
	kChipsetAddressCnt	= 4

};

typedef struct SIL3124_Info SIL3124_Info;
typedef SIL3124_Info* SIL3124_InfoPtr;

struct SIL3124_Info{
	UInt8 *flashVirt;
	UInt32 prevFlashPhys;
	UInt16 prevCmdReg;
};

OSErr SIL3124_Mem_Init(RegEntryIDPtr device, SIL3124_InfoPtr info, UInt32 accessSize);
OSErr SIL3124_Mem_Term(RegEntryIDPtr device, SIL3124_InfoPtr info);

OSErr SIL3124_Reader(RegEntryIDPtr device, void* image, UInt32 size){
	SIL3124_Info info;
	OSErr err = SIL3124_Mem_Init(device, &info, size);
	UInt8 *flash_adr = info.flashVirt;
	UInt8 *data = (UInt8*)image;
	if(err!=noErr){
		__DEBUG__ printf("error calling Mem_Init\n");
		return err;
	}
	while(size--){
		SynchronizeIO();
		*data++ = *flash_adr++;
	}
	SynchronizeIO();
	return SIL3124_Mem_Term(device, &info);
}

OSErr SIL3124_Writer(RegEntryIDPtr device, void* image, UInt32 size){
	SIL3124_Info info;
	OSErr err = SIL3124_Mem_Init(device, &info, size);
	UInt8 *flash_adr = info.flashVirt;
	UInt8 *data = (UInt8*)image;
	if(err!=noErr){
		__DEBUG__ printf("error calling Mem_Init\n");
		return err;
	}
	while(size--){
		SynchronizeIO();
		*flash_adr++ = *data++;
	}
	SynchronizeIO();
	return SIL3124_Mem_Term(device, &info);
}

OSErr SIL3124_Mem_Init(RegEntryIDPtr device, SIL3124_InfoPtr info, UInt32 accessSize){
	PCIAssignedAddress chipsetPhysAddrs[kChipsetAddressCnt];
	LogicalAddress chipsetVirtAddrs[kChipsetAddressCnt];
	LogicalAddress chipsetFlashVirt = 0;
	RegPropertyValueSize size;
	int index;
	UInt32 romBasePhys;
	UInt16 cmdReg;
	OSErr err;
	if(accessSize > kFlashSize_SIL3124){
		__DEBUG__ printf("attempted flash access too big\n");
		return paramErr;
	}
	size = kChipsetAddressCnt * sizeof(PCIAssignedAddress);
	err = RegistryPropertyGet(device, "assigned-addresses", (void*)chipsetPhysAddrs, &size);
	if(err!=noErr || size!=kChipsetAddressCnt * sizeof(PCIAssignedAddress)){
		__DEBUG__ printf("failure to get assigned-addresses property\n");
		return err;
	}
	
	size = kChipsetAddressCnt * sizeof(LogicalAddress);
	err = RegistryPropertyGet(device, "AAPL,address", (void*)chipsetVirtAddrs, &size);
	if(err!=noErr || size!=kChipsetAddressCnt * sizeof(LogicalAddress)){
		__DEBUG__ printf("failure to get AAPL,address property\n");
		return err;
	}
	for(index = 0; index < kChipsetAddressCnt; index++){
		__DEBUG__ printf("chipset register %i: %x\n", index, chipsetPhysAddrs[index].registerNumber);
		if(chipsetPhysAddrs[index].registerNumber == kChipsetROMBase){
			chipsetFlashVirt = chipsetVirtAddrs[index];
			__DEBUG__ printf("Expansion ROM base is %x\n", chipsetFlashVirt);
			break;
		}
	}
	if(chipsetFlashVirt==0){
		__DEBUG__ printf("Expansion ROM base not found\n");
		return -1;
	}
	
	info->flashVirt = (UInt8*)chipsetFlashVirt;
	
	err = ExpMgrConfigReadLong(device, (LogicalAddress)kPCIConfigExpansionROMBase, &romBasePhys);
	if(err!=noErr){
		__DEBUG__ printf("error reading Expansion ROM base register\n");
		return err;
	}
	info->prevFlashPhys = romBasePhys;
	
	romBasePhys |= kExp_ROM_En;//set Expansion ROM enable
	
	err = ExpMgrConfigReadWord(device, (LogicalAddress)kPCIConfigCommand, &cmdReg);
	
	if(err != noErr){
		__DEBUG__ printf("error reading PCI config command register\n");
		return err;
	}
	
	info->prevCmdReg = cmdReg;
	
	cmdReg |= kCmd_Mem_Map;//set memory space enable
	
	err = ExpMgrConfigWriteWord(device, (LogicalAddress)kPCIConfigCommand, cmdReg);
	
	if(err != noErr){
		__DEBUG__ printf("error writing PCI config command register\n");
		return err;
	}
	err = ExpMgrConfigWriteLong(device, (LogicalAddress)kPCIConfigExpansionROMBase, romBasePhys);
	
	if(err!=noErr){
		__DEBUG__ printf("error writing Expansion ROM base register\n");
		ExpMgrConfigWriteWord(device, (LogicalAddress)kPCIConfigCommand, info->prevCmdReg);
	}
	SynchronizeIO();
	return err;
}

OSErr SIL3124_Mem_Term(RegEntryIDPtr device, SIL3124_InfoPtr info){
	OSErr err = noErr;
	err = ExpMgrConfigWriteWord(device, (LogicalAddress)kPCIConfigExpansionROMBase, info->prevFlashPhys);
	if(err){
		__DEBUG__ printf("error restoring Expansion ROM base register\n");
	}
	err = ExpMgrConfigWriteWord(device, (LogicalAddress)kPCIConfigCommand, info->prevCmdReg);
	
	SynchronizeIO();
	if(err){
		__DEBUG__ printf("error restoring PCI config command register\n");
	}
	return err;
}
