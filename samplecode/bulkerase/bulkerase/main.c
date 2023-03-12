#include <glob.h>
#include "dru_devices.h"
#include "dru_burning.h"


int main(int argc, char *argv[]);



int main(int argc, char *argv[])
{
#pragma unused(argc, argv)
	DRDeviceRef		device = NULL;
	char			buffer[80];
	int				fullErase = 0;
	
	/* Hello world! */
	printf("Bulk Erase\n");
	printf("\n");
	printf("  This tool will erase discs in bulk.  When an erasable disc is\n");
	printf("  inserted into the selected drive, it is automatically erased\n");
	printf("  and then ejected.  Loops forever until killed - hit ^C to exit.\n");
	printf("\n");
	
	/* First, use DRU to prompt the user to pick a device.
		If there's only one device to choose, the selection is automatic. */
	device = druPromptForDevice(NULL,druFilter_AnyEraser);
	
	/* Print out a description of the device. */
	printf("Selected device: %s\n", druGetDeviceDescription(device,buffer,sizeof(buffer)));
	printf("\n");
	
	/* Next, ask the user if they want to do quick or full erases. */
	printf("Do you want to perform quick or full erases? \n");
	printf("\n");
	printf("  A quick erase finishes quickly, but does not erase the entire\n");
	printf("  disc.  Sometimes one drive may be able to read a disc that\n");
	printf("  another has quick-erased.  Quick erases are useful if you are\n");
	printf("  going to re-burn the disc immediately, or when data security is\n");
	printf("  not a concern.\n");
	printf("\n");
	printf("  A full erase takes a lot longer to complete, sometimes as much as\n");
	printf("  20-40 minutes or more, but the entire disc is erased.  Fully-erased\n");
	printf("  discs are effectively like new and can be stored for later use or used\n");
	printf("  in a different drive.\n");
	printf("\n");
	printf("Please select (Q)uick or (F)ull [default is Quick]: ");
	fflush(stdout);
	fgets(buffer,sizeof(buffer),stdin);
	if (buffer[0] == 'f' || buffer[0] == 'F')
		fullErase = 1;
	printf("%s erase selected.\n", fullErase ? "Full":"Quick");
	
	if (DRDeviceAcquireExclusiveAccess(device) == noErr)
	{
		/* Loop forever, until the user kills our process. */
		while (1)
		{
			/* Use DRU to prompt the user to insert erasable media. */
			printf("\n");
			druPromptForErasableMediaInDevice(device);
			
			/* Time to erase.  DRU automatically handles the erase and progress. */
			druErase(device,fullErase);
			
			/* Eject the media. */
			DRDeviceEjectMedia(device);
		}
	}
	else
	{
		printf("The device is in use by another application.\n");
	}
	
	/* Clean up after ourselves. This code never runs because of
		the infinite loop above, but is displayed here for completeness. */
	if (device != NULL)	
	{
		DRDeviceReleaseExclusiveAccess(device);
		CFRelease(device);
	}
	
	return 0;
}

