/*
	dru_progress.h
	
	Part of the Disc Recording Utility sources for command-line tools.  This
	code provides utility functions for handling progress bars.
*/

#ifndef _H_dru_progress
#define _H_dru_progress

#include <DiscRecording/DiscRecording.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct druProgress * dru_progress_t;

/* Creates a progress bar. After this call, the cursor is being tracked by the bar,
	and you should no longer call printf() yourself. */
dru_progress_t	druProgressBarCreate();

/* Sets the behavior of the progress bar. */
void druProgressBarSetNewLineForEachStage(dru_progress_t progress, int newline);

/* Gets the behavior of the progress bar. */
int druProgressBarGetNewLineForEachStage(dru_progress_t progress);

/* Draws a progress bar.  Percent is a floating-point value between 0 and 1. 
	The on-screen progress bar is updated automatically. */
void druProgressBarUpdate(dru_progress_t progress, const char *stage, float percentage);

/* Disposes a progress bar.  Success is an indicator of whether the operation was
	successful (in which case the progress bar is erased and "done" is printed)
	or if it was aborted (in which case the progress bar is left standing.) */
void druProgressBarDispose(dru_progress_t progress, int success);

#ifdef __cplusplus
}
#endif

#endif /* _H_dru_progress */

