#include "windows.h"



static HINSTANCE	ghInst=0;



/* ----------------------------------------------------------- */



BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)

{

	ghInst = hInst;



	switch( ul_reason_for_call ) {

		case DLL_PROCESS_ATTACH:

			break;



		case DLL_THREAD_ATTACH:

			break;



		case DLL_THREAD_DETACH:

			break;



		case DLL_PROCESS_DETACH:

 			break;

    }

    return TRUE;

}



/* ----------------------------------------------------------- */

