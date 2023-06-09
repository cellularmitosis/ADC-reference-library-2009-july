	ComponentComment ("SGPanelDispatch.h for the SG panel")

	ComponentSelectorOffset (6)

	ComponentRangeCount (3)
	ComponentRangeShift (8)
	ComponentRangeMask	(FF)

	ComponentStorageType (Handle)

	ComponentRangeBegin (0)
		ComponentError (Target)
		ComponentError (Register)
		StdComponentCall	(Version)
		StdComponentCall	(CanDo)
		StdComponentCall	(Close)
		StdComponentCall	(Open)
	ComponentRangeEnd (0)


	ComponentRangeUnused (1)
	ComponentRangeUnused (2)

	ComponentRangeBegin (3)
		ComponentCall						(PanelGetDitl)
		ComponentCall						(PanelGetTitle)
		ComponentCall						(PanelCanRun)
		ComponentCall						(PanelInstall)
		ComponentCall						(PanelEvent)
		ComponentCall						(PanelItem)
		ComponentCall						(PanelRemove)
		ComponentCall						(PanelSetGrabber)
		ComponentCall						(PanelSetResFile)
		ComponentCall						(PanelGetSettings)
		ComponentCall						(PanelSetSettings)
		ComponentCall						(PanelValidateInput)
		ComponentError						(PanelSetEventFilter)
	ComponentRangeEnd (3)
	
