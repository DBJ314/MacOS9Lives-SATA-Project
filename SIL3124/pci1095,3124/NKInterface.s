	MACRO
	MakeFunction &fnName
		EXPORT &fnName[DS]
 		EXPORT .&fnName[PR]
		
		TC &fnName[TC], &fnName[DS]
			
		CSECT &fnName[DS]
			DC.L .&fnName[PR]
 			DC.L TOC[tc0]
		
		CSECT .&fnName[PR]
		FUNCTION .&fnName[PR]	
		
	ENDM
	
	MakeFunction NKPrintf
	li r0, 96
	sc
	blr
	
	
	MakeFunction NKPrintHex
	li r0, 97
	li r4, 0
	sc
	blr
	