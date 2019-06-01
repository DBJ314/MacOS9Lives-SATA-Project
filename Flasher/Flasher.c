#include "Flasher.h"

ChipsetData chipsets[kChipsetsSupported] = {
	{
		"sil3124",
		0x1095,
		0x3124,
		kFlashSize_SIL3124,
		SIL3124_Reader,
		SIL3124_Writer
	}
};

enum {
	kQuitCmd = 0,
	kDumpCmd,
	kFlashCmd,
	kSkipCmd,
	kSearchCmd,
	
	kNumRepeatCmds = 2
};

UICommand mainPrompt[kMainNumCmds] = {
	{"DUMP", kDumpCmd},
	{"FLASH", kFlashCmd},
	{"SKIP", kSkipCmd},
	{"QUIT", kQuitCmd}
};

UICommand repeatPrompt[kNumRepeatCmds] = {
	{"QUIT", kQuitCmd},
	{"SEARCH", kSearchCmd}
};

enum {
	kNumOpenableTypes = 1
};
SFTypeList typeList = {'FLSH', 0, 0, 0};

void printSupportedChipsets();

void printRegEntry(RegEntryIDPtr node);

void primaryLoop();

int uiPrompt(UICommand *cmdList, int numCmds);

void devDump(RegEntryIDPtr device, int chipset);

void devFlash(RegEntryIDPtr device, int chipset);

void printFileErrs(OSErr err);

int main(){
	printf("Flasher v1.0 starting up\n\n");
	printSupportedChipsets();
	
	primaryLoop();
	return 0;
}

void printSupportedChipsets(){
	int i;
	printf("Supported Chipsets:\n");
	for(i = 0; i < kChipsetsSupported; i++){
		printf("  %s\n", chipsets[i].name);
	}
	putchar('\n');
}

void printRegEntry(RegEntryIDPtr node){
	char* path;
	RegPathNameSize pathSize;
	OSStatus status;
	status = RegistryEntryToPathSize(node, &pathSize);
	if(status){
		return;
	}
	path = NewPtr(pathSize);
	if(!path){
		return;
	}
	status = RegistryCStrEntryToPath(node, path, pathSize);
	if(status){
		return;
	}
	printf("%s\n", path);//If there are printf hijack attacks in Name Reg paths, you are probably in deep trouble anyways
	DisposePtr(path);
}

void primaryLoop(){
	int chipsetIndex;
	Boolean quitting = false;
	Boolean anythingFound = false;//to check if any devices are present at all
	Boolean searchDone;
	RegEntryID device;
	RegEntryIter searchCookie;
	RegEntryIterationOp iterOp;
	UInt32 devID;//yes, the NR stores these as u32, (but they're u16)
	UInt32 vendID;
	UInt32 returnedVendID;
	RegPropertyValueSize returnedVIDSize;
	RegPropertyValueSize dummySize;
	OSStatus status;
	while(!quitting){
		for(chipsetIndex = 0; chipsetIndex < kChipsetsSupported && !quitting; chipsetIndex++){
			printf("Searching for instances of chipset %s\n\n", chipsets[chipsetIndex].name);
			status = RegistryEntryIterateCreate(&searchCookie);
			iterOp = kRegIterDescendants;
			if(status!=noErr){
				continue;
			}
			devID = chipsets[chipsetIndex].deviceID;
			vendID = chipsets[chipsetIndex].vendorID;
			searchDone = false;
			while(!searchDone&&!quitting){
				status = RegistryEntrySearch(&searchCookie, iterOp, &device, &searchDone, kDeviceID, (void*)&devID, sizeof(devID));
				if(status!=noErr||searchDone){
					break;
				}
				iterOp = kRegIterContinue;
				
				returnedVIDSize = sizeof(returnedVendID);
				status = RegistryPropertyGet(&device, kVendorID, (void*)&returnedVendID, &returnedVIDSize);
				if(status!=noErr){
					RegistryEntryIDDispose(&device);
					continue;
				}
				if(returnedVIDSize != sizeof(returnedVendID)){
					printf("???: vendor-id property has weird length on node %s\n");
					RegistryEntryIDDispose(&device);
					continue;
				}
				if(returnedVendID!=vendID){
					//I guess it wasn't really a match
					RegistryEntryIDDispose(&device);
					continue;
				}
				/*now we are sure we have a match
				Time to note it to the console */
				printf("Found a device:\n");
				printRegEntry(&device);
				anythingFound = true;
				
				/* time to check if there is a driver for this device
				 * if there is, we probably do not want to flash it */
				
				status = RegistryPropertyGetSize(&device, kDriverDescriptor, &dummySize);//idk if size ptr can be 0
				
				if(status==noErr){
					printf("This device has a driver. You should probably SKIP it.\n");
				}
				
				/* now to prompt the user */
				printf("Should I DUMP the device rom, FLASH the device rom, SKIP this device, or QUIT?\n");
				
				switch(uiPrompt(mainPrompt, kMainNumCmds)){
					case kQuitCmd:
						printf("Quitting...\n");
						quitting = true;
						break;
					case kSkipCmd:
						printf("Skipping device...\n");
						break;
					case kDumpCmd:
						printf("Dumping device rom...\n");
						devDump(&device, chipsetIndex);
						break;
					case kFlashCmd:
						printf("Flashing device rom...\n");
						devFlash(&device, chipsetIndex);
						break;
				}
				RegistryEntryIDDispose(&device);	
			}
			RegistryEntryIterateDispose(&searchCookie);
		}
		if(quitting){
			break;
		}
		if(anythingFound){
			printf("No more matching devices. Should I SEARCH  again, or QUIT?\n");
			if(uiPrompt(repeatPrompt, kNumRepeatCmds)==kQuitCmd){
				printf("Quitting...\n");
				quitting = true;
			}else{
				printf("Searching again...\n");
			}
		}else{
			printf("Nothing found. Quitting...\n");
			quitting = true;
		}
	}
}

int uiPrompt(UICommand *cmdList, int numCmds){
	char cmdBuf[kMaxCmdLen];
	int cmdIndex;
	int i;
	while(true){
		
		printf("Please type ");
	
		for(cmdIndex = 0; cmdIndex < numCmds-1; cmdIndex++){
			printf("\"%s\", ", cmdList[cmdIndex].cmd);
		}
		printf("or \"%s\"\nFlasher> ", cmdList[numCmds-1].cmd);
		
		if(!fgets(cmdBuf, kMaxCmdLen, stdin)){
			return kCmdFailure;//also is kCmdQuit
		}
		//convert to uppercase
		for(i = 0; i < kMaxCmdLen; i++){
			if(cmdBuf[i]>='a'&&cmdBuf[i]<='z'){
				cmdBuf[i] -= 0x20;
			}
			if(cmdBuf[i]=='\n'){
				cmdBuf[i] = 0;
			}
		}
		
		for(cmdIndex = 0; cmdIndex < numCmds; cmdIndex++){
			if(strcmp(cmdBuf, cmdList[cmdIndex].cmd)==0){
				return cmdList[cmdIndex].cmdNumber;
			}
		}
		printf("\nInvalid Command \"%s\"\n", cmdBuf);
	}
}


void devDump(RegEntryIDPtr device, int chipset){
	StandardFileReply sfReply;
	Ptr romImage;
	Size flashSize = chipsets[chipset].flashSize;
	short fRefNum;
	OSErr err;
	StandardPutFile(0, "\pFlash File", &sfReply);
	if(!sfReply.sfGood){
		printf("File select failed or canceled\n");
		return;
	}
	romImage = NewPtr(flashSize);
	if(!romImage){
		printf("Memory allocation failed\n");
		return;
	}
	if(chipsets[chipset].reader(device, (void*)romImage, flashSize)!=noErr){
		printf("Flash read failed\n");
		DisposePtr(romImage);
		return;
	}
	if(sfReply.sfReplacing){
		err = FSpDelete(&(sfReply.sfFile));
		if(err!=noErr){
			printf("File replace failed.\n");
			printFileErrs(err);
			DisposePtr(romImage);
			return;
		}
	}
	
	err = FSpCreate(&(sfReply.sfFile), 'FLSH', 'FLSH', sfReply.sfScript);
	if(err!=noErr){
		printf("File creation failed.\n");
		printFileErrs(err);
		DisposePtr(romImage);
		return;
	}
	
	err = FSpOpenDF(&(sfReply.sfFile), fsRdWrPerm, &fRefNum);
	if(err!=noErr){
		printf("File open failed.\n");
		printFileErrs(err);
		DisposePtr(romImage);
		FSpDelete(&(sfReply.sfFile));
		return;
	}
	
	err = FSWrite(fRefNum, &flashSize, romImage);
	
	if(err!=noErr){
		printf("File write failed.\n");
		printFileErrs(err);
	}
	
	FSClose(fRefNum);
	
	//if FSWrite had an error, delete the file*/
	if(err==noErr){
		printf("Flash dump complete\n");
	}else{
		FSpDelete(&(sfReply.sfFile));
	}
	
	DisposePtr(romImage);
}

void devFlash(RegEntryIDPtr device, int chipset){
	StandardFileReply sfReply;
	Ptr romImage;
	Size flashSize = chipsets[chipset].flashSize;
	short fRefNum;
	OSErr err;
	StandardGetFile(0, kNumOpenableTypes, typeList, &sfReply);
	if(!sfReply.sfGood){
		printf("File select failed or canceled.\n");
		return;
	}
	
	err = FSpOpenDF(&(sfReply.sfFile), fsRdPerm, &fRefNum);
	if(err){
		printf("File open failed.\n");
		printFileErrs(err);
		return;
	}
	
	romImage = NewPtr(flashSize);
	
	if(!romImage){
		printf("Memory allocation failed.\n");
		FSClose(fRefNum);
		return;
	}
	
	err = FSRead(fRefNum, &flashSize, romImage);
	if(err!=noErr){
		printf("File read failed\n");
		FSClose(fRefNum);
		DisposePtr(romImage);
	}
	
	printf("Flashing 0x%08x bytes\n", flashSize);
	
	err = chipsets[chipset].writer(device, (void*)romImage, flashSize);
	
	if(err==noErr){
		printf("Flash completed successfully.\n");
	}
	FSClose(fRefNum);
	DisposePtr(romImage);
}

void printFileErrs(OSErr err){
	switch(err){
			case dirFulErr:
				printf("Directory full\n");
				break;
			case dskFulErr:
				printf("Disk full\n");
				break;
			case nsvErr:
				printf("No such volume\n");
				break;
			case ioErr:
				printf("I/O error\n");
				break;
			case bdNamErr:
				printf("Bad filename\n");
				break;
			case fnfErr:
				printf("Directory not found or incomplete filename\n");
				break;
			case wPrErr:
				printf("Hardware volume lock\n");
				break;
			case vLckdErr:
				printf("Software volume lock\n");
				break;
			case dupFNErr:
				printf("Duplicate filename and version\n");
				break;
			case dirNFErr:
				printf("Directory not found or incomplete pathname\n");
				break;
			case afpAccessDenied:
				printf("User does not have correct AFP access\n");
				break;
			case afpObjectTypeErr:
				printf("A directory exists with that name\n");
				break;
			case tmfoErr:
				printf("Too many files open\n");
				break;
			case opWrErr:
				printf("File already open for writing\n");
				break;
			case permErr:
				printf("Attempt to open locked file for writing\n");
				break;
			case fnOpnErr:
				printf("File not open\n");
				break;
			case rfNumErr:
				printf("Bad reference number\n");
				break;
			/*case Err:
				printf("\n");
				break;*/
			default:
				printf("unknown error code: %06x\n", err&0xFFFF);
				break;
		}
}