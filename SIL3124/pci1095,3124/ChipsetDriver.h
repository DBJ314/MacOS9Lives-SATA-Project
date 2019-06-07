#ifndef CHIPSET_DRIVER_H
#define CHIPSET_DRIVER_H

#include <DriverFamilyMatching.h>
#include <DriverServices.h>
#include <NameRegistry.h>
#include <Pci.h>

int isInitialized;
int openCount = 0;

DriverRefNum myRefNum;
RegEntryID myDeviceRegID;

LogicalAddress chipsetGlobals;
LogicalAddress chipsetPorts;
LogicalAddress chipsetIO;
LogicalAddress chipsetROM;


enum {
	kChipsetGlobalBase	= kPCIConfigBaseAddress0,
	kChipsetPortBase	= kPCIConfigBaseAddress1,
	kChipsetIOBase		= kPCIConfigBaseAddress2,
	kChipsetROMBase		= kPCIConfigExpansionROMBase,
	kChipsetAddressCnt	= 4

};
PCIAssignedAddress chipsetPhysAddrs[kChipsetAddressCnt];
LogicalAddress chipsetVirtAddrs[kChipsetAddressCnt];

InterruptSetMember chipsetInterrupts;

/* DriverSystemInterface.c */
OSErr CFMInitialize();

OSErr CFMTerminate();

OSErr DoDriverIO (
					AddressSpaceID spaceID,
					IOCommandID ID,
					IOCommandContents contents,
					IOCommandCode code,
					IOCommandKind kind);

OSErr HandleInitialize(DriverInitInfoPtr initialInfo);

OSErr HandleFinalize(DriverFinalInfoPtr finalInfo);

/* NameRegUtils.c */

OSErr getNRProp(RegEntryIDPtr node, char* propName, RegPropertyValueSize propSize, void* prop);

/* ChipsetDebug.c */

typedef UInt8 DebugLevel;
extern DebugLevel dbLevel;
enum {
	kDebugLevelNone = 0,
	kDebugLevelError = 1,
	kDebugLevelMessage = 2,
	kDebugLevelAll = 255
};

void debugPrintStr(DebugLevel msgLevel, char* message);

void debugPrintHex(DebugLevel msgLevel, UInt32 message);

#define PRNT_ERROR_STR(str) debugPrintStr(kDebugLevelError, str)
#define PRNT_ERROR_HEX(hex) debugPrintHex(kDebugLevelError, hex)
#define PRNT_MESSAGE_STR(str) debugPrintStr(kDebugLevelMessage, str)
#define PRNT_MESSAGE_HEX(hex) debugPrintHex(kDebugLevelMessage, hex)
/* NanoKernel Interface.s */

void NKPrintf(char*);

void NKPrintHex(int hex);

#endif // CHIPSET_DRIVER_H