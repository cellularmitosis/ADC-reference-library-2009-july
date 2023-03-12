#include <glob.h>
#include "dru_devices.h"
#include "dru_burning.h"


int main(int argc, char *argv[]);

void promptForFolder(const char *prompt, const char *defaultPath, FSRef *outRef);
void expandPathname(char *path);
DRFilesystemMask promptForFilesystems();



int main(int argc, char *argv[])
{
#pragma unused(argc, argv)
	DRDeviceRef				device = NULL;
	FSRef					folderOnDisk;
	DRBurnRef				burn = NULL;
	DRFilesystemTrackRef	track = NULL;
	DRFolderRef				folder = NULL;
	DRFilesystemMask		mask = 0;
	
	/* Hello world! */
	printf("Welcome to the data burn sample code!\n\n");
	
	/* First, let's use DRU to prompt the user to pick a device.
		If there's only one device to choose, the selection is automatic. */
	device = druPromptForDevice(NULL,druFilter_AnyBurner);
	
	/* We know we're going to burn to this device - acquire blank media
		reservations just in case the user inserts something. 
		Just because we ask for a shot at blank media doesn't mean that
		we'll get it. Some other app might not be willing to give 
		up the blank media for our use. */
	DRDeviceAcquireMediaReservation(device);
	
	/* Prompt the user to pick a folder to burn. */
	promptForFolder(NULL,"/Applications",&folderOnDisk);
	
	/* Prompt the user to pick the filesystems to put on the disk.
		Normally you wouldn't do this - instead, you'd just leave the
		mask at the default and allow the burn engine to generate
		the most compatible disc possible.  Shown here for demonstration
		purposes only. */
	mask = promptForFilesystems();
	
	/* Next, use DRU to prompt the user to insert blank media. */
	druPromptForBlankMediaInDevice(device);
	
	/* Check to see if we have the media reservation. If we don't 
		have it that means that some other application has it and 
		won't give it up. We might not get it if, for instance, if
		the Finder has claimed the media for it's CD proxy and the user
		has placed files onto that proxy image. In this case the 
		Finder wouldn't want to unmount the proxy image since the
		user's intention is to burn that information to the inserted
		disc. */
	if (druMediaIsReserved(device))
	{
		/* We've got blank media, and a folder to burn ... now set up the burn objects. */
		burn = DRBurnCreate(device);
		folder = DRFolderCreateReal(&folderOnDisk);
		DRFSObjectSetFilesystemMask(folder,mask);
		track = DRFilesystemTrackCreate(folder);
		CFRelease(folder);
		
		/* Use DRU to run the burn and report progress. */
		druBurn(burn,track);
	}
	else
	{
		printf("The media in the selected device is reserved for use by another application.\n");
	}
	
	/* Clean up after ourselves. */
	if (burn != NULL)
		CFRelease(burn);
	if (track != NULL)
		CFRelease(track);
		
	if (device != NULL)	
	{
		DRDeviceReleaseMediaReservation(device);
		CFRelease(device);
	}
	return 0;
}



void promptForFolder(const char *prompt, const char *defaultPath, FSRef *outRef)
{
	char		path[PATH_MAX];
	OSStatus	err;
	Boolean		isDirectory;
	int			len;
	
	/* Display the prompt. */
	if (prompt == NULL)
		prompt = "Please enter the path to a folder:";
	printf("%s ", prompt);
	if (defaultPath != NULL)
		printf("[%s] ",defaultPath);
	fflush(stdout);
	
	/* Get user input, and trim trailing newlines. */
	fgets(path,sizeof(path),stdin);
	for (len = strlen(path); len > 0 && path[len-1] == '\n';)
		path[--len] = 0;
	if (path[0] == 0)
		strcpy(path,defaultPath);
	
	/* Expand magic characters just like a shell (mostly so ~ will work) */
	expandPathname(path);
	
	/* Convert the path into an FSRef, which is what the burn engine needs. */
	err = FSPathMakeRef((const UInt8*)path,outRef,&isDirectory);
	if (err != noErr)
	{
		printf("Bad path. Aborting. (%d)\n", (int)err);
		exit(1);
	}
	if (!isDirectory)
	{
		printf("That's a file, not a directory!  Aborting.\n");
		exit(1);
	}
}



void expandPathname(char *path)
{
	char	original[PATH_MAX];
	int		i;
	glob_t	g = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	/* Preserve the incoming path */
	strcpy(original,path);
	
	/* Call glob to expand it. */
	glob(path,GLOB_NOSORT | GLOB_TILDE | GLOB_QUOTE,NULL,&g);
	if (g.gl_matchc == 1)
		strcpy(path,g.gl_pathv[0]);
	else
	{
		printf("Bad path! ");
		if (g.gl_matchc == 0)
			printf("Not found. ");
		else
		{
			printf("%d matches found:\n",g.gl_matchc);
			for (i=0; i<g.gl_matchc; ++i)
				printf(" %s\n", g.gl_pathv[i]);
		}
		printf("Aborted.\n");
		exit(1);
	}
	globfree(&g);
}



DRFilesystemMask
promptForFilesystems()
{
	int		i, numChoices;
	char	input[10];
	DRFilesystemMask	choices[] = {
		kDRFilesystemMaskHFSPlus | kDRFilesystemMaskISO9660 | kDRFilesystemMaskJoliet,
		kDRFilesystemMaskISO9660 | kDRFilesystemMaskJoliet,
		kDRFilesystemMaskISO9660
	};
	const char *		names[] = {
		"HFS+/ISO/Joliet",
		"ISO/Joliet",
		"ISO only"
	};
	
	/* Give the user a menu of choices. */
	numChoices = sizeof(choices)/sizeof(DRFilesystemMask);
	printf("\n");
	for (i=0; i<numChoices; ++i)
		printf(" %d) %s\n", i+1, names[i]);
	printf("Select the filesystems you'd like to write: ");
	fflush(stdout);
	
	/* Get user input. */
	fgets(input,sizeof(input),stdin);
	i = -1;
	sscanf(input,"%d",&i);
	if (i<1 || i> numChoices) {
		printf("Aborted.\n");
		exit(1);
	}
	
	return choices[i-1];
}


