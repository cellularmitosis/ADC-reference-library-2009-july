	ComponentComment ("													\
		This file describes the selectors used by this component.  The	\
		16-bit selector space is divided into consecutively numbered	\
		ranges of uniform size.  Each range has a list of zero or more	\
		consecutive selectors.											\
																		\
		ComponentDispatchHelper.c uses this file to generate function	\
		prototypes and a dispatcher for this component.					\
	")

	ComponentComment ("Count of core selectors in range 0")
	ComponentSelectorOffset (6)

	ComponentComment ("Last selector range of this component")
	ComponentRangeCount (6)

	ComponentComment ("Size of each selector range in bits")
	ComponentRangeShift (8)
	ComponentRangeMask	(FF)
	
	
	ComponentComment ("Component core selectors")
	ComponentRangeBegin (0)
		StdComponentCall	(Target)
		ComponentError		(Register)
		StdComponentCall	(Version)
		StdComponentCall	(CanDo)
		StdComponentCall	(Close)
		StdComponentCall	(Open)
	ComponentRangeEnd (0)
	
	
	ComponentRangeUnused (1)
	ComponentRangeUnused (2)
	ComponentRangeUnused (3)
	ComponentRangeUnused (4)
	ComponentRangeUnused (5)
	
	
	ComponentComment ("RTPMediaPacketizer selectors")
	ComponentRangeBegin (6)
		ComponentCall		(Initialize)
		ComponentCall		(PreflightMedia)
		ComponentDelegate	(Idle)
		ComponentCall		(SetSampleData)
		ComponentCall		(Flush)
		ComponentCall		(Reset)
		ComponentDelegate	(SetInfo)
		ComponentCall		(GetInfo)
		ComponentCall		(SetTimeScale)
		ComponentCall		(GetTimeScale)
		ComponentCall		(SetTimeBase)
		ComponentCall		(GetTimeBase)
		ComponentCall		(HasCharacteristic)
		ComponentDelegate	(0x50D)
		ComponentCall		(SetPacketBuilder)
		ComponentCall		(GetPacketBuilder)
		ComponentCall		(SetMediaType)
		ComponentCall		(GetMediaType)
		ComponentCall		(SetMaxPacketSize)
		ComponentCall		(GetMaxPacketSize)
		ComponentCall		(SetMaxPacketDuration)
		ComponentCall		(GetMaxPacketDuration)
		ComponentCall		(DoUserDialog)
		ComponentCall		(SetSettingsFromAtomContainerAtAtom)
		ComponentCall		(GetSettingsIntoAtomContainerAtAtom)
		ComponentCall		(GetSettingsAsText)
	ComponentRangeEnd (6)
