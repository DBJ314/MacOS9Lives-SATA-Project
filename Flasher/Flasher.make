#   File:       Flasher.make
#   Target:     Flasher
#   Created:    Tuesday, May 28, 2019 07:23:05 PM


MAKEFILE        = Flasher.make
•MondoBuild•    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

ObjDir          = :
Includes        = 

Sym-PPC         = -sym off

PPCCOptions     = {Includes} {Sym-PPC} 


### Source Files ###

SrcFiles        =  ∂
				  Chipset_SIL3124.c ∂
				  Flasher.c


### Object Files ###

ObjFiles-PPC    =  ∂
				  "{ObjDir}Chipset_SIL3124.c.x" ∂
				  "{ObjDir}Flasher.c.x"


### Libraries ###

LibFiles-PPC    =  ∂
				  "{SharedLibraries}DriverLoaderLib" ∂
				  "{SharedLibraries}DriverServicesLib" ∂
				  "{SharedLibraries}NameRegistryLib" ∂
				  "{SharedLibraries}PCILib" ∂
				  "{PPCLibraries}PPCSIOW.o" ∂
				  "{SharedLibraries}InterfaceLib" ∂
				  "{SharedLibraries}StdCLib" ∂
				  "{SharedLibraries}MathLib" ∂
				  "{PPCLibraries}StdCRuntime.o" ∂
				  "{PPCLibraries}PPCCRuntime.o" ∂
				  "{PPCLibraries}PPCToolLibs.o"


### Default Rules ###

.c.x  ƒ  .c  {•MondoBuild•}
	{PPCC} {depDir}{default}.c -o {targDir}{default}.c.x {PPCCOptions}


### Build Rules ###

Flasher  ƒƒ  {ObjFiles-PPC} {LibFiles-PPC} {•MondoBuild•}
	PPCLink ∂
		-o {Targ} ∂
		{ObjFiles-PPC} ∂
		{LibFiles-PPC} ∂
		{Sym-PPC} ∂
		-mf -d ∂
		-t 'APPL' ∂
		-c 'siow'


Flasher  ƒƒ  "{RIncludes}"SIOW.r {•MondoBuild•}
	Rez "{RIncludes}"SIOW.r -o {Targ} -append -d __kPrefSize=800 -d __kMinSize=800


### Required Dependencies ###

"{ObjDir}Chipset_SIL3124.c.x"  ƒ  Chipset_SIL3124.c
"{ObjDir}Flasher.c.x"  ƒ  Flasher.c


### Optional Dependencies ###
### Build this target to generate "include file" dependencies. ###

Dependencies  ƒ  $OutOfDate
	MakeDepend ∂
		-append {MAKEFILE} ∂
		-ignore "{CIncludes}" ∂
		-objdir "{ObjDir}" ∂
		-objext .x ∂
		{Includes} ∂
		{SrcFiles}


