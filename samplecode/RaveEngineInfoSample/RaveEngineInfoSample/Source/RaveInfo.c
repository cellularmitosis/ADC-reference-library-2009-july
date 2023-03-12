/******************************************************************************
 **																			 **
 ** 	Module:		RAVEInfo.c												 **
 ** 																		 **	
 ** 	Purpose: 	Outputs RAVE gestalt information. 						 **
 ** 																		 **
 ** 	Author:		Robert Dierkes											 **
 ** 																		 **
 ** 	Copyright (C) 1998-99 Apple Computer, Inc.  All rights reserved.	 **
 ** 																		 **
 *****************************************************************************/

#include <ConditionalMacros.h>
#include <RAVE.h>
#include <QD3DAcceleration.h>

#include <stdio.h>

#include "RaveInfo.h"

/******************************************************************************
 ** 																		 **
 ** 	Macros																 **
 ** 																		 **
 *****************************************************************************/
#define	kMaxGestalts	100
#define	TITLE_WIDTH		36

#if	defined(TARGET_OS_MAC) && (TARGET_OS_MAC)
	/* Write standard output to SIOUX console */
	#define	PRINT_1(_a)					printf(_a)
	#define	PRINT_2(_a, _b)				printf(_a, _b)
	#define	PRINT_3(_a, _b, _c)			printf(_a, _b, _c)
	#define	PRINT_4(_a, _b, _c, _d)		printf(_a, _b, _c, _d)
	#define	PRINT_5(_a, _b, _c, _d, _e)	printf(_a, _b, _c, _d, _e)

	#define	kCharMark	'Ã'

#else
	/* Write standard output to file */
	#define	PRINT_1(_a)					fprintf(fp, _a);fflush(fp);
	#define	PRINT_2(_a, _b)				fprintf(fp, _a, _b);fflush(fp);
	#define	PRINT_3(_a, _b, _c)			fprintf(fp, _a, _b, _c);fflush(fp);
	#define	PRINT_4(_a, _b, _c, _d)		fprintf(fp, _a, _b, _c, _d);fflush(fp);
	#define	PRINT_5(_a, _b, _c, _d, _e)	fprintf(fp, _a, _b, _c, _d, _e);fflush(fp);

	#define	kCharMark	'+'
	#define	FILE_NAME	"RAVEInfo.out"

#endif /* TARGET_OS_WIN32 */


#define	QA_ENGINE_GESTALT(_engine,_mask,_response)								\
		{	errs[g] = QAEngineGestalt(_engine,	_mask,	_response);				\
			g++;																\
		}


#define	PRINT_1_VALUE(_fmt, _title, _value)										\
		{	if (errs[g] == kQANoErr) {											\
				PRINT_4(_fmt, TITLE_WIDTH, _title, _value);						\
			} else {															\
				PRINT_4("%-*s%s", TITLE_WIDTH, _title, "<Gestalt unsupported>\n");\
			}																	\
			g++;																\
		}


#define	PRINT_2_VALUE(_fmt, _title, _value)										\
		{	if (errs[g] == kQANoErr) {											\
				PRINT_5(_fmt, TITLE_WIDTH, _title, _value, _value);				\
			} else {															\
				PRINT_4("%-*s%s", TITLE_WIDTH, _title, "<Gestalt unsupported>\n");\
			}																	\
			g++;																\
		}


#define	PRINT_OPTION(_opt, _mask, _maskLast)									\
			PRINT_4("%*s%c "#_maskLast"\n",										\
				TITLE_WIDTH, "",												\
				(_opt & _mask ## _maskLast) ? kCharMark : ' ');


#define	PRINT_PIXEL_1_TYPE(_value, _maskLast)									\
			PRINT_4("%*s%c "# _maskLast"\n",									\
			TITLE_WIDTH, "",													\
			(_value & (1 << (kQAPixel_ ## _maskLast)))	? kCharMark : ' ');


#define	PRINT_PIXEL_TYPES(_fmt, _title, _value)									\
		{	if (errs[g] == kQANoErr) {											\
				PRINT_4(_fmt, TITLE_WIDTH, _title, _value);						\
				PRINT_PIXEL_1_TYPE(_value, Alpha1);								\
				PRINT_PIXEL_1_TYPE(_value, RGB16);								\
				PRINT_PIXEL_1_TYPE(_value, ARGB16);								\
				PRINT_PIXEL_1_TYPE(_value, RGB32);								\
				PRINT_PIXEL_1_TYPE(_value, ARGB32);								\
				PRINT_PIXEL_1_TYPE(_value, CL4);								\
				PRINT_PIXEL_1_TYPE(_value, CL8);								\
				PRINT_PIXEL_1_TYPE(_value, RGB16_565);							\
				PRINT_PIXEL_1_TYPE(_value, RGB24);								\
				PRINT_PIXEL_1_TYPE(_value, RGB8_332);							\
				PRINT_PIXEL_1_TYPE(_value, ARGB16_4444);						\
				PRINT_PIXEL_1_TYPE(_value, ACL16_88);							\
				PRINT_PIXEL_1_TYPE(_value, I8);									\
				PRINT_PIXEL_1_TYPE(_value, AI16_88);							\
				PRINT_PIXEL_1_TYPE(_value, YUVS);								\
				PRINT_PIXEL_1_TYPE(_value, YUVU);								\
				PRINT_PIXEL_1_TYPE(_value, YVYU422);							\
				PRINT_PIXEL_1_TYPE(_value, UYVY422);							\
			} else {															\
				PRINT_4("%-*s%s", TITLE_WIDTH, _title, "<Gestalt unsupported>\n");\
			}																	\
			g++;																\
		}



/******************************************************************************
 ** 																		 **
 ** 	RAVEGestaltInfo()													 **
 ** 																		 **
 *****************************************************************************/
void RAVEGestaltInfo(void)
{
	TQAEngine		*engine = NULL;
	TQAError		errs[kMaxGestalts];
	unsigned long	count, g;
	unsigned long	optionalFeatures;
	unsigned long	fastFeatures;
	long			vendorID;
	long			engineID;
	long			nameLength;
	char 			name[200];
	long			revision;
	unsigned long	textureMemory;
	unsigned long	fastTextureMemory;
	/* 1.6 -> */
	unsigned long	dcPixelTypesAllowed;
	unsigned long	dcPixelTypesPreferred;
	unsigned long	txPixelTypesAllowed;
	unsigned long	txPixelTypesPreferred;
	unsigned long	bmPixelTypesAllowed;
	unsigned long	bmPixelTypesPreferred;
	unsigned long	optionalFeatures2;
	unsigned long	multiTextureMax;

#if	defined(TARGET_OS_WIN32) && (TARGET_OS_WIN32)
	FILE 			*fp;

	fp = fopen(FILE_NAME, "w");
	if (fp == NULL) {
		return;
	}
#endif /* TARGET_OS_WIN32 */


#if	defined(TARGET_OS_MAC) && (TARGET_OS_MAC)
	PRINT_4("\n%-*s%s", TITLE_WIDTH, "RAVE Gestalt Information", "Power Macintosh");
#elif defined(TARGET_OS_WIN32) && (TARGET_OS_WIN32)
	PRINT_4("\n%-*s%s", TITLE_WIDTH, "RAVE Gestalt Information", "Win32");
#else
	PRINT_4("\n%-*s%s", TITLE_WIDTH, "RAVE Gestalt Information", "<Unknown Platform>");
#endif
	PRINT_1("\n--------------------------------------------------------------\n\n");

	QAEngineEnable (kQAVendor_Apple, kQAEngine_AppleHW);

	count = 0;
	engine = QADeviceGetFirstEngine(NULL);
	while (engine) {

		g = 0;
		QA_ENGINE_GESTALT(engine, kQAGestalt_ASCIINameLength, &nameLength);
		if ((nameLength < 200) && (nameLength > 0)) {
			QA_ENGINE_GESTALT(engine, kQAGestalt_ASCIIName,	name);
			PRINT_5("Engine %u:%-*s\"%s\"\n", count+1, TITLE_WIDTH-9, "", name);
		} else {
			PRINT_5("Engine %u:%-*s\"%s\"\n", count+1, TITLE_WIDTH-9, "", "<Unknown>");
		}

		count++;
		engine = QADeviceGetNextEngine(NULL, engine);
	}
	PRINT_1("\n--------------------------------------------------------------\n");


	/* Cycle through all available engines getting gestalt information */
	engine = QADeviceGetFirstEngine(NULL);
	while (engine) {

		/* Get gestalt responses */
		for (g = kMaxGestalts; --g;) {
			errs[g] = kQANoErr;
		}
		g = 0;

		QA_ENGINE_GESTALT(engine,	kQAGestalt_ASCIINameLength,		&nameLength);
		if ((nameLength < 200) && (nameLength > 0)) {
			QA_ENGINE_GESTALT(engine,	kQAGestalt_ASCIIName,		name);
		} else {
			name[0] = 0;
			g++;
		}
		QA_ENGINE_GESTALT(engine,	kQAGestalt_VendorID,			&vendorID);
		QA_ENGINE_GESTALT(engine,	kQAGestalt_EngineID,			&engineID);
		QA_ENGINE_GESTALT(engine,	kQAGestalt_Revision,			&revision);
		QA_ENGINE_GESTALT(engine,	kQAGestalt_TextureMemory,		&textureMemory);
		QA_ENGINE_GESTALT(engine,	kQAGestalt_FastTextureMemory,	&fastTextureMemory);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_MultiTextureMax,		&multiTextureMax);

/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_DrawContextPixelTypesAllowed,	&dcPixelTypesAllowed);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_DrawContextPixelTypesPreferred,	&dcPixelTypesPreferred);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_TexturePixelTypesAllowed,		&txPixelTypesAllowed);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_TexturePixelTypesPreferred,		&txPixelTypesPreferred);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_BitmapPixelTypesAllowed,			&bmPixelTypesAllowed);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_BitmapPixelTypesPreferred,		&bmPixelTypesPreferred);

		QA_ENGINE_GESTALT(engine,	kQAGestalt_OptionalFeatures,	&optionalFeatures);
/*1.6*/	QA_ENGINE_GESTALT(engine,	kQAGestalt_OptionalFeatures2,	&optionalFeatures2);
		QA_ENGINE_GESTALT(engine,	kQAGestalt_FastFeatures,		&fastFeatures);


		/* Print responses (must be in same order as above) */
		g = 1;	/* Don't print ASCIINameLength */
		PRINT_1("\n");
		PRINT_1_VALUE("%-*s\"%s\"\n",		"Engine Name:",			name);
		PRINT_2_VALUE("%-*s%08u\t0x%08X\n",	"Vendor ID:",			vendorID);
		PRINT_2_VALUE("%-*s%08u\t0x%08X\n",	"Engine ID:",			engineID);
		PRINT_2_VALUE("%-*s%08u\t0x%08X\n",	"Revision:",			revision);
		PRINT_2_VALUE("%-*s%08u\t0x%08X\n",	"Texture Memory:",		textureMemory);
		PRINT_2_VALUE("%-*s%08u\t0x%08X\n",	"Fast Texture Memory:",	fastTextureMemory);
/*1.6*/	PRINT_2_VALUE("%-*s%08u\t0x%08X\n",	"Multi Texture Maximum:",multiTextureMax);

/*1.6*/	PRINT_PIXEL_TYPES("%-*s0x%08X\n",	"Draw Context Pixel Types Allowed:",	dcPixelTypesAllowed);
/*1.6*/	PRINT_PIXEL_TYPES("%-*s0x%08X\n",	"Draw Context Pixel Types Preferred:",	dcPixelTypesPreferred);
/*1.6*/	PRINT_PIXEL_TYPES("%-*s0x%08X\n",	"Texture Pixel Types Allowed:",			txPixelTypesAllowed);
/*1.6*/	PRINT_PIXEL_TYPES("%-*s0x%08X\n",	"Texture Pixel Types Preferred:",		txPixelTypesPreferred);
/*1.6*/	PRINT_PIXEL_TYPES("%-*s0x%08X\n",	"Bitmap Pixel Types Allowed:",			bmPixelTypesAllowed);
/*1.6*/	PRINT_PIXEL_TYPES("%-*s0x%08X\n",	"Bitmap Pixel Types Preferred:",		bmPixelTypesPreferred);

		PRINT_1_VALUE("%-*s0x%08X\n",		"Optional Features:",	optionalFeatures);
		if (errs[g-1] == kQANoErr) {
			PRINT_OPTION(optionalFeatures, kQAOptional_, DeepZ);
			PRINT_OPTION(optionalFeatures, kQAOptional_, Texture);
			PRINT_OPTION(optionalFeatures, kQAOptional_, TextureHQ);
			PRINT_OPTION(optionalFeatures, kQAOptional_, TextureColor);
			PRINT_OPTION(optionalFeatures, kQAOptional_, Blend);
			PRINT_OPTION(optionalFeatures, kQAOptional_, BlendAlpha);
			PRINT_OPTION(optionalFeatures, kQAOptional_, Antialias);
			PRINT_OPTION(optionalFeatures, kQAOptional_, ZSorted);
			PRINT_OPTION(optionalFeatures, kQAOptional_, PerspectiveZ);
			PRINT_OPTION(optionalFeatures, kQAOptional_, OpenGL);
			PRINT_OPTION(optionalFeatures, kQAOptional_, NoClear);
			PRINT_OPTION(optionalFeatures, kQAOptional_, CSG);
			PRINT_OPTION(optionalFeatures, kQAOptional_, BoundToDevice);
			PRINT_OPTION(optionalFeatures, kQAOptional_, CL4);
			PRINT_OPTION(optionalFeatures, kQAOptional_, CL8);
			PRINT_OPTION(optionalFeatures, kQAOptional_, BufferComposite);
			PRINT_OPTION(optionalFeatures, kQAOptional_, NoDither);
			/* 1.6 ->*/
			PRINT_OPTION(optionalFeatures, kQAOptional_, FogAlpha);
			PRINT_OPTION(optionalFeatures, kQAOptional_, FogDepth);
			PRINT_OPTION(optionalFeatures, kQAOptional_, MultiTextures);
			PRINT_OPTION(optionalFeatures, kQAOptional_, MipmapBias);
			PRINT_OPTION(optionalFeatures, kQAOptional_, ChannelMask);
			PRINT_OPTION(optionalFeatures, kQAOptional_, ZBufferMask);
			PRINT_OPTION(optionalFeatures, kQAOptional_, AlphaTest);
			PRINT_OPTION(optionalFeatures, kQAOptional_, AccessTexture);
			PRINT_OPTION(optionalFeatures, kQAOptional_, AccessBitmap);
			PRINT_OPTION(optionalFeatures, kQAOptional_, AccessDrawBuffer);
			PRINT_OPTION(optionalFeatures, kQAOptional_, AccessZBuffer);
			PRINT_OPTION(optionalFeatures, kQAOptional_, ClearDrawBuffer);
			PRINT_OPTION(optionalFeatures, kQAOptional_, ClearZBuffer);
			PRINT_OPTION(optionalFeatures, kQAOptional_, OffscreenDrawContexts);
		}

/*1.6*/	PRINT_1_VALUE("%-*s0x%08X\n",		"Optional Features 2:",	optionalFeatures2);
		if (errs[g-1] == kQANoErr) {
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, TextureDrawContexts);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, BitmapDrawContexts);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, Busy);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, SwapBuffers);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, Chromakey);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, NonRelocatable);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, NoCopy);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, PriorityBits);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, FlipOrigin);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, BitmapScale);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, DrawContextScale);
			PRINT_OPTION(optionalFeatures2, kQAOptional2_, DrawContextNonRelocatable);
		}

		PRINT_1_VALUE("%-*s0x%08X\n",		"Fast Features:",		fastFeatures);
		if (errs[g-1] == kQANoErr) {
			PRINT_OPTION(fastFeatures, kQAFast_, Line);
			PRINT_OPTION(fastFeatures, kQAFast_, Gouraud);
			PRINT_OPTION(fastFeatures, kQAFast_, Texture);
			PRINT_OPTION(fastFeatures, kQAFast_, TextureHQ);
			PRINT_OPTION(fastFeatures, kQAFast_, Blend);
			PRINT_OPTION(fastFeatures, kQAFast_, Antialiasing);
			PRINT_OPTION(fastFeatures, kQAFast_, ZSorted);
			PRINT_OPTION(fastFeatures, kQAFast_, CL4);
			PRINT_OPTION(fastFeatures, kQAFast_, CL8);
/*1.6*/		PRINT_OPTION(fastFeatures, kQAFast_, FogAlpha);
/*1.6*/		PRINT_OPTION(fastFeatures, kQAFast_, FogDepth);
/*1.6*/		PRINT_OPTION(fastFeatures, kQAFast_, MultiTextures);
/*1.6*/		PRINT_OPTION(fastFeatures, kQAFast_, BitmapScale);
/*1.6*/		PRINT_OPTION(fastFeatures, kQAFast_, DrawContextScale);
		}

		PRINT_1("\n--------------------------------------------------------------\n");

		engine = QADeviceGetNextEngine(NULL, engine);
	}

#if	defined(TARGET_OS_WIN32) && (TARGET_OS_WIN32)
	fclose(fp);
#endif /* TARGET_OS_WIN32 */
}

