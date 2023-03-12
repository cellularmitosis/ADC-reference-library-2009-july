/*
	dru_progress.c
	
	Part of the Disc Recording Utility sources for command-line tools.  This
	code provides utility functions for handling progress bars.
*/
#include <DiscRecording/DiscRecording.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/ttycom.h>
#include <math.h>
#include "dru_progress.h"


#define MAX_PROGRESS_WIDTH		80
void druTestProgressBar();
void druWinch(int signal);


typedef struct druProgress druProgress;
struct druProgress
{
	char	lastStage[100];					/* last stage that we were in */
	char	lineBuffer[MAX_PROGRESS_WIDTH];	/* the active line */
	int		lastPercent;					/* last percentage we processed */
	
	int		newlineWhenStageCompletes;		/* setting: print a newline when the stage completes? */
};


int			druInstalledWinch = 0;
int			druReceivedWinch = 0;
int			druScreenWidth = 0;
int			druTTY = -1;



#pragma mark -




/*
	druTestProgressBar
	
	Runs a test for the progress bar.
*/
void druTestProgressBar()
{
	dru_progress_t		bar = druProgressBarCreate();
	int					nlosc = 1;
	float				percent;
	
	druProgressBarSetNewLineForEachStage(bar,nlosc);
	
	for (percent = 0.0; percent <= 1.0; percent += 0.001)
	{
		druProgressBarUpdate(bar,"Preparing...",percent);
		usleep(10000);
	}
	
	for (percent = 0.0; percent <= 1.0; percent += 0.001)
	{
		druProgressBarUpdate(bar,"Writing...",percent);
		usleep(10000);
	}
	
	for (percent = 0.0; percent <= 1.0; percent += 0.001)
	{
		druProgressBarUpdate(bar,"Closing...",percent);
		usleep(1000);
	}
	
	druProgressBarUpdate(bar,"Finishing up...",0);
	
	druProgressBarDispose(bar,1);
	printf("Test complete.\n");
}



#pragma mark -



/*
	druProgressBarCreate
	
	Creates a new, empty progress bar.
*/
dru_progress_t
druProgressBarCreate()
{
	dru_progress_t	bar = (dru_progress_t)calloc(sizeof(struct druProgress),1);
	struct winsize size;
	
	/* Install SIGWINCH handler so that we correctly handle window resizes. */
	if (!druInstalledWinch)
	{
		druInstalledWinch = 1;
		signal(SIGWINCH,druWinch);
	}
	
	/* Get the width of the screen. */
	if (druTTY == -1)
		druTTY = open("/dev/tty", O_RDONLY, 0);
	if (ioctl(druTTY,TIOCGWINSZ,&size) == 0)
		druScreenWidth = size.ws_col;
	
	/* Set a few variables. */
	druReceivedWinch = 0;
	
	/* Return the progress bar. */
	return bar;
}



/*
	druProgressBarSetNewLineForEachStage
*/
void
druProgressBarSetNewLineForEachStage(dru_progress_t progress, int nl)
{
	progress->newlineWhenStageCompletes = nl;
}


/*
	druProgressBarGetNewLineForEachStage
*/
int
druProgressBarGetNewLineForEachStage(dru_progress_t progress)
{
	return progress->newlineWhenStageCompletes;
}



/*
	druProgressBarUpdate
*/
void
druProgressBarUpdate(dru_progress_t progress, const char *stage, float percentage)
{
	int		availableWidth = druScreenWidth;
	char	newLineBuffer[MAX_PROGRESS_WIDTH];
	int		stageLen = strlen(stage);
	int		percent = (int)(percentage * 100);
	
	/* Handle stage changes. */
	if (strcmp(stage,progress->lastStage))
	{
		if (progress->lastStage[0] == 0 && progress->lineBuffer[0] == 0)
		{
			/* This is the first stage.  We don't need to do anything special for this. */
		}
		else
		{
			/* We just changed stages. Mark the previous stage as finished. */
			druProgressBarUpdate(progress,progress->lastStage,1.0);
			druProgressBarUpdate(progress,progress->lastStage,INFINITY);
			
			/* If we've been told to generate a new line for each stage, do so. */
			if (progress->newlineWhenStageCompletes)
			{
				printf("\n");
				progress->lineBuffer[0] = 0;
			}
		}
		
		/* Remember the new stage. */
		strncpy(progress->lastStage,stage,sizeof(progress->lastStage));
		progress->lastStage[sizeof(progress->lastStage)-1] = 0;
	}
	
	/* If we received a SIGWINCH, erase the current progress bar. */
	if (druReceivedWinch)
	{
		struct winsize size;
		
		druReceivedWinch = 0;
		
		if (ioctl(druTTY,TIOCGWINSZ,&size) == 0)
			druScreenWidth = size.ws_col;
		
		printf("\r%*s\r", (int)strlen(progress->lineBuffer), "");
		progress->lineBuffer[0] = 0;
		fflush(stdout);
	}
	
	/* Figure out how much space is available now. */
	if (availableWidth > MAX_PROGRESS_WIDTH)
		availableWidth = MAX_PROGRESS_WIDTH;
	
	/* Build a new line buffer. The first thing in it is the stage. */
	strncpy(newLineBuffer,stage,availableWidth);
	newLineBuffer[availableWidth-1] = 0;
	
	/* Next, if we're exactly done, say so.  We check for either the
		special INFINITY param, or for a percentage that jumps from 0 to 100%. */
	if (percentage > 10000 || (percent == 100 && progress->lastPercent == 0))
	{
		snprintf(&newLineBuffer[stageLen],availableWidth - stageLen," done.");
	}
	/* Otherwise add the progress bar and percentage.  We only need to draw a
		progress bar if the percentage is greater than zero.  We treat negative
		or zero progress as indeterminate. */
	else if (percent > 0)
	{
		/* How much space is available? */
		if (availableWidth - stageLen < 5)
		{
			/* No space for anything. */
		}
		else if (availableWidth - stageLen < 15)
		{
			/* Only enough space for a percentage. */
			snprintf(&newLineBuffer[stageLen],availableWidth - stageLen," %d%% ", percent);
		}
		else
		{
			/* There's enough room for a progress bar and a percentage. */
			int		barTotalWidth = availableWidth - stageLen - 10;
			int		barFullWidth = (int)(barTotalWidth * percentage);
			
			if (barFullWidth < 0) barFullWidth = 0;
			if (barFullWidth > barTotalWidth) barFullWidth = barTotalWidth;
			
			/* First fill in an empty progress bar. */
			snprintf(&newLineBuffer[stageLen],availableWidth - stageLen," [%*s] %d%% ",
						barTotalWidth, "", percent);
			
			/* It's full of stars! */
			while (barFullWidth > 0)
				newLineBuffer[stageLen + 2 + --barFullWidth] = '*';
		}
	}
	
	/* We've got our new line buffer.  Now let's print out enough characters to
		update it.  To reduce flicker, we don't erase and redraw the entire line
		every time; we just redraw what changed. */
	if (strcmp(progress->lineBuffer,newLineBuffer))
	{
		unsigned int		i,j;
		
		/* Find the first changed character. */
		for (i = 0; i < sizeof(newLineBuffer); ++i)
			if (progress->lineBuffer[i] != newLineBuffer[i])
				break;
		
		/* Back up that many characters. */
		j = strlen(progress->lineBuffer) - i;
		while (j-- > 0)
			printf("\b \b");
		
		/* Print the rest of the line. */
		printf("%s",&newLineBuffer[i]);
		
		/* Remember the line as it now stands. */
		strncpy(progress->lineBuffer,newLineBuffer,sizeof(progress->lineBuffer));
		progress->lineBuffer[sizeof(progress->lineBuffer)-1] = 0;
		
		/* Last but not least, flush the display. */
		fflush(stdout);
		
		/* If we just hit 100%, sleep for a half a second so that the user
			sees it and feels a sense of closure, before we jump to the "done" state. */
		if (percent == 100)
			usleep(500000);
	}
	
	/* Remember the last percentage. */
	progress->lastPercent = (percent > 0) ? percent:0;
}



/*
	druProgressBarDispose
	
	Disposes a progress bar, with an indicator of whether the progress bar is being
	terminated with successfuls status or not.  (Unsuccessful status means that
	we leave the bar visible the the user.)
*/
void druProgressBarDispose(dru_progress_t progress, int success)
{
	/* Close the current stage, if any. */
	if (progress->lastStage[0] != 0 && success)
	{
		druProgressBarUpdate(progress,progress->lastStage,1.0);
		druProgressBarUpdate(progress,progress->lastStage,INFINITY);
	}
	
	/* If we're doing a newline on stage changes, print a newline. Otherwise,
		this counts as a stage change, so erase the progress if we're successful. */
	if (progress->newlineWhenStageCompletes || !success)
		printf("\n");
	else if (!progress->newlineWhenStageCompletes && success)
	{
		printf("\r%*s\r", (int)strlen(progress->lineBuffer), "");
		progress->lineBuffer[0] = 0;
		fflush(stdout);
	}
	
	/* That's it - free our buffer and we're done. */
	free(progress);
}



#pragma mark -



/*
	druWinch
	
	Signal handler, simply sets druReceivedWinch to true.  The progress callback
	will notice next time it draws.
*/
void
druWinch(int signal)
{
#pragma unused(signal)
	druReceivedWinch = 1;
}



