#ifndef FLASHER_H
#define FLASHER_H

#include <DriverServices.h>
#include <Files.h>
#include <NameRegistry.h>
#include <MacErrors.h>
#include <MacMemory.h>
#include <MacTypes.h>
#include <Pci.h>
#include <StandardFile.h>
#include <stdio.h>
#include <string.h>

#define DEBUGGING 1
#define __DEBUG__ if(DEBUGGING)

typedef OSErr (*FlashReader)(RegEntryIDPtr device, void* image, UInt32 size);
typedef OSErr (*FlashWriter)(RegEntryIDPtr device, void* image, UInt32 size);

typedef struct ChipsetData ChipsetData;

enum {
	kChipsetsSupported = 1,
	kFlashSize_SIL3124 = 512 * 1024
};
extern ChipsetData chipsets[kChipsetsSupported];

struct ChipsetData{
	char* name;
	UInt16 vendorID;
	UInt16 deviceID;
	UInt32 flashSize;
	FlashReader reader;
	FlashWriter writer;
};

#define kDeviceID "device-id"
#define kVendorID "vendor-id"

#define kDriverDescriptor "driver-descriptor"

typedef struct UICommand UICommand;

enum {
	kMaxCmdLen = 8,
	kCmdFailure = 0,
	kMainNumCmds = 4
};

extern UICommand mainPrompt[kMainNumCmds];

struct UICommand {
	char cmd[kMaxCmdLen];
	int cmdNumber;

};

/* Flasher.c */

int main();

int uiPrompt(UICommand *cmdList, int numCmds);

/* Chipset_SIL3124.c */

OSErr SIL3124_Reader(RegEntryIDPtr device, void* image, UInt32 size);
OSErr SIL3124_Writer(RegEntryIDPtr device, void* image, UInt32 size);

#endif FLASHER_H