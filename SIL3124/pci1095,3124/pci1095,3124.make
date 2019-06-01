#   File:       pci1095,3124.make
#   Target:     pci1095,3124
#   Created:    Wednesday, July 18, 2018 03:18:54 PM


MAKEFILE        = pci1095,3124.make
•MondoBuild•    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

ObjDir          = :
Includes        = 

Sym-PPC         = -sym off

PPCCOptions     = {Includes} {Sym-PPC} 

PPCAOptions     = {Includes} {Sym-PPC}


### Source Files ###

SrcFiles        =  ∂
				  DriverSystemInterface.c ∂
				  NameRegUtils.c ∂
				  ChipsetDebug.c ∂
				  NKInterface.s


### Object Files ###

ObjFiles-PPC    =  ∂
				  "{ObjDir}DriverSystemInterface.c.x" ∂
				  "{ObjDir}NameRegUtils.c.x" ∂
				  "{ObjDir}ChipsetDebug.c.x" ∂
				  "{ObjDir}NKInterface.s.x"

### Libraries ###

LibFiles-PPC    =  ∂
				  "{SharedLibraries}DriverLoaderLib" ∂
				  "{SharedLibraries}DriverServicesLib" ∂
				  "{SharedLibraries}NameRegistryLib" ∂
#				  "{SharedLibraries}StdCLib" ∂
#				  "{SharedLibraries}MathLib" ∂
#				  "{PPCLibraries}StdCRuntime.o" ∂
				  "{PPCLibraries}PPCCRuntime.o"


### Default Rules ###

.c.x  ƒ  .c  {•MondoBuild•}
	{PPCC} {depDir}{default}.c -o {targDir}{default}.c.x {PPCCOptions}

.s.x  ƒ  .s  {•MondoBuild•}
	{PPCAsm} {depDir}{default}.s -o {targDir}{default}.s.x {PPCAOptions}


### Build Rules ###

pci1095,3124  ƒƒ  {ObjFiles-PPC} {LibFiles-PPC} {•MondoBuild•}
	PPCLink ∂
		-o {Targ} ∂
		{ObjFiles-PPC} ∂
		{LibFiles-PPC} ∂
		{Sym-PPC} ∂
		-mf -d ∂
		-t 'ndrv' ∂
		-c 'DanB' ∂
		-xm s ∂
		-export DoDriverIO ∂
		-export TheDriverDescription ∂
		-init CFMInitialize ∂
		-term CFMTerminate



### Required Dependencies ###

"{ObjDir}DriverSystemInterface.c.x"  ƒ  DriverSystemInterface.c

"{ObjDir}NameRegUtils.c.x"  ƒ  NameRegUtils.c

"{ObjDir}ChipsetDebug.c.x"  ƒ  ChipsetDebug.c

"{ObjDir}NKInterface.s.x"  ƒ  NKInterface.s

DriverSystemInterface.c  ƒ ChipsetDriver.h

NameRegUtils.c  ƒ ChipsetDriver.h

ChipsetDebug.c  ƒ ChipsetDriver.h


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


