README 

SoundPlayer v1.0
Sept. 6, 2002

SoundPlayer is based on PlaySoundFillBuffer.c, originally part of the MP3 Player sample. It has now been updated and shows how to play VBR / Non-VBR encoded audio using the SoundConverterFillBuffer API and QuickTime. This includes VBR formats like MP3 and AAC encoded audio. 

SoundConverterFillBuffer is the preferred API to use with the SoundConverter. SoundConverterConvertBuffer should no longer be used as it does not support VBR formats and cannot be used with MPEG4 Audio. 

NOTE: This sample requires QuickTime 6.0 or greater.