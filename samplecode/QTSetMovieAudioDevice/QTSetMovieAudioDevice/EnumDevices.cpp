/*
*  File:    EnumDevices.cpp
* 
*  Contains:  Main Program. Displays a window which shows an enumeration of all Windows
*             DirectSound output devices. Buttons then allow user to choose an output device, 
*             and play a movie through the selected output device.
*  
*  Version:  1.0
* 
*  Created:  2/15/06
*
*  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*        ("Apple") in consideration of your agreement to the following terms, and your
*        use, installation, modification or redistribution of this Apple software
*        constitutes acceptance of these terms.  If you do not agree with these terms,
*        please do not use, install, modify or redistribute this Apple software.
*
*        In consideration of your agreement to abide by the following terms, and subject
*        to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
*        copyrights in this original Apple software (the "Apple Software"), to use,
*        reproduce, modify and redistribute the Apple Software, with or without
*        modifications, in source and/or binary forms; provided that if you redistribute
*        the Apple Software in its entirety and without modifications, you must retain
*        this notice and the following text and disclaimers in all such redistributions of
*        the Apple Software.  Neither the name, trademarks, service marks or logos of
*        Apple Computer, Inc. may be used to endorse or promote products derived from the
*        Apple Software without specific prior written permission from Apple.  Except as
*        expressly stated in this notice, no other rights or licenses, express or implied,
*        are granted by Apple herein, including but not limited to any patent rights that
*        may be infringed by your derivative works or by other works in which the Apple
*        Software may be incorporated.
*
*        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*        COMBINATION WITH YOUR PRODUCTS.
*
*        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*  Copyright:  Copyright © 2006 Apple Computer, Inc, All Rights Reserved
*/

#include "EnumDevices.h"
#include "QTCode.h"

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------

INT_PTR CALLBACK MainDialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDialog( HWND hDlg );

INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, TCHAR* strDesc, TCHAR* strDrvName,
                                  VOID* pContext );
HRESULT InitDirectSound( HWND hDlg );
HRESULT FreeDirectSound();

void CopySelectedSoundDeviceName( HWND hDlg,  TCHAR *outCurrentSoundDeviceName);
void CopySelectedSoundDeviceGUID( HWND hDlg,  GUID* outCurrentSoundGUID);

BOOL GetFile (TCHAR *theFileName);

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//-----------------------------------------------------------------------------
// Global data
//-----------------------------------------------------------------------------

LPDIRECTSOUND			g_pDS							= NULL;
Movie					g_Movie							= NULL;
QTAudioContextRef		g_QTAudioContextForDevice		= NULL;
boolean					g_QT704Installed				= false;
static TCHAR			g_MovieFileNamePath[MAX_PATH]	= TEXT("");
static TCHAR			g_MovieFileName[MAX_PATH]		= TEXT("");
ComponentInstance		g_MovieController				= NULL;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------

INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    // Initialize COM
    CoInitialize( NULL );

	// Initialize QuickTime
	InitializeQTML(0);
	require(EnterMovies() == noErr, FAILQTINIT);

	g_QT704Installed = QT_QT704OrBetterInstalled();
	g_Movie = NULL;

    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc );

FAILQTINIT:

    // Release COM
    CoUninitialize();

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: MainDialogProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------

INT_PTR CALLBACK MainDialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr; 
	static CFStringRef movieFilePathRef = NULL;
	static HANDLE movieFileHandle = NULL;

	// give the movie controller this message first
	if (g_MovieController) 
	{
		MSG				myMsg = {0};
		EventRecord		myMacEvent;
		LONG			myPoints = GetMessagePos();

		myMsg.hwnd = hDlg;
		myMsg.message = msg;
		myMsg.wParam = wParam;
		myMsg.lParam = lParam;
		myMsg.time = GetMessageTime();
		myMsg.pt.x = LOWORD(myPoints);
		myMsg.pt.y = HIWORD(myPoints);

		// translate a Windows event to a Mac event
		WinEventToMacEvent(&myMsg, &myMacEvent);

		// pass the Mac event to the movie controller, but only if the movie window isn't minimized
		if (!IsIconic(hDlg))
		{
			MCIsPlayerEvent(g_MovieController, (EventRecord *)&myMacEvent);
		}
	}

    switch( msg ) 
    {
		case WM_NOTIFY:
			break;

		case WM_CREATE:
			break;

		case WM_NULL:

			// we receive this message only to idle the movie
			if (g_MovieController)
			{
				MCIdle(g_MovieController);
			}
			break;

        case WM_COMMAND:

            switch( LOWORD(wParam) )
            {
                case IDC_CREATE:

                    // Init DirectSound
                    if( SUCCEEDED( hr = InitDirectSound( hDlg ) ) )
                    {
                        MessageBox( hDlg, TEXT("DirectSound interface created successfully"), 
                                          TEXT("EnumDevices"), MB_OK );
                    }
                    else
                    {
                        DXTRACE_ERR_MSGBOX( TEXT("InitDirectSound"), hr );
                        MessageBox( hDlg, TEXT("DirectSound interface creatation failed"), 
                                          TEXT("EnumDevices"), MB_OK | MB_ICONERROR );
                    }

                    break;

				case IDC_SETMOVIE:
					{
						// get a movie file from the user

						TCHAR newMovieFileNamePath[MAX_PATH] = TEXT("");
						BOOL gotAFile = GetFile((TCHAR *)&newMovieFileNamePath);
						if (gotAFile)
						{
							// if we got a new file, suspend any currently playing movie

							if( g_MovieController )
							{
								MCDoAction(g_MovieController, mcActionSuspend, (void *)NULL);
				
								// dispose of the existing movie controller and movie resource
								DisposeMovieController(g_MovieController);
								DisposeMovie(g_Movie);
							}

							// copy movie path
							StringCchCopy( g_MovieFileNamePath, MAX_PATH, newMovieFileNamePath );
							WCHAR* strLastSlash = wcsrchr( g_MovieFileNamePath, '\\' );
							// copy movie file name
							StringCchCopy( g_MovieFileName, MAX_PATH, strLastSlash + 1 );

							// create a new movie using the movie file path
							movieFilePathRef = CFStringCreateWithCharacters(kCFAllocatorDefault, 
																	g_MovieFileNamePath, 
																	MAX_PATH);
							require(movieFilePathRef != NULL, bail);
							OSStatus status = QT_CreateMovieWithProperties(movieFilePathRef, NULL, &g_Movie);

							// show the movie name in the dialog box
							SetDlgItemText( hDlg, IDC_MOVIENAME, g_MovieFileName );

							// create a new movie controller for the movie
							Rect movieRect;
							GetMovieBox(g_Movie, &movieRect);
							g_MovieController = NewMovieController(g_Movie, &movieRect, mcNotVisible);
							
						bail:
						;
						}
					}
					break;

					// play button
				case IDC_BUTTON2:
					{
						if (g_Movie)
						{
							// First create a QuickTime Audio Context from either:
							//
							//	- GUID	(QT 7.0.4 or better only)
							//	- Device Name

							if (g_QT704Installed )
							{
								// Use GUID

								GUID currentSoundDeviceGUID;

								CopySelectedSoundDeviceGUID( hDlg,  &currentSoundDeviceGUID);

								// Create audio context from GUID
								OSStatus status = QT_CreateAudioContextFromGUID(&currentSoundDeviceGUID, &g_QTAudioContextForDevice);
							}
							else
							{
								// Use Device Name

								TCHAR currDeviceName[MAX_PATH];
								CopySelectedSoundDeviceName( hDlg, (TCHAR *)&currDeviceName);

								// Create audio context from Device Name
								OSStatus status = QT_CreateAudioContextFromDeviceName((TCHAR *)&currDeviceName, &g_QTAudioContextForDevice);
							}

							// Set the QuickTime Audio Context for the current movie
							OSStatus status = SetMovieAudioContext(g_Movie, g_QTAudioContextForDevice);

							// Make a time record for the beginning of the movie
							TimeRecord currentTime;
							GetMovieTime(g_Movie, &currentTime);
							currentTime.value.hi = 0; currentTime.value.lo = 0;

							// Set movie back to the beginning and play!
							MCDoAction(g_MovieController, mcActionGoToTime, (void *)&currentTime);
							MCDoAction(g_MovieController, mcActionPrerollAndPlay, (void *)GetMoviePreferredRate(g_Movie));
						}
					}
					break;

				case IDC_SOUND_DEVICE_COMBO:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						printf("CBN_SELCHANGE\n");
					}
					break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;


        case WM_INITDIALOG:
            if( FAILED( hr = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR_MSGBOX( TEXT("OnInitDialog"), hr );
                MessageBox( hDlg, L"Error enumerating DirectSound devices. "
                                  L"Sample will now exit.", L"DirectSound Sample", 
                                  MB_OK | MB_ICONERROR );
                EndDialog( hDlg, IDABORT );
            }

			// Associate a GrafPort with the native window
			CreatePortAssociation(hDlg, NULL, 0L);

            break;

        case WM_DESTROY:
			{
	            // Cleanup everything

				// Release DirectSound object
				FreeDirectSound();

				// Remove GrafPort association for native window
				DestroyPortAssociation((CGrafPtr)GetNativeWindowPort(hDlg));
			}
            break; 

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}

//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------

HRESULT OnInitDialog( HWND hDlg )
{
    HRESULT hr;

    // Enumerate the sound devices and place them in the combo box
    HWND hSoundDeviceCombo = GetDlgItem( hDlg, IDC_SOUND_DEVICE_COMBO );
    if( FAILED( hr = DirectSoundEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback,
                                           (VOID*)hSoundDeviceCombo ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("DirectSoundEnumerate"), hr );

    // Select the first device in the combo box
    SendMessage( hSoundDeviceCombo,   CB_SETCURSEL, 0, 0 );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DSoundEnumCallback()
// Desc: Enumeration callback called by DirectSoundEnumerate
//-----------------------------------------------------------------------------

INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, TCHAR* strDesc, TCHAR* strDrvName,
                                  VOID* pContext )
{
    static DWORD dwAudioDriverIndex = 0;
	static WORD guidArrayIndex = 0;

	// Note:   The first device enumerated is always called the 
	// Primary Sound Driver, and the lpGUID parameter of the 
	// callback is NULL. This device represents the preferred 
	// playback device set by the user in Control Panel. It is 
	// enumerated separately to make it easy for the application 
	// to add "Primary Sound Driver" to a list when presenting 
	// the user with a choice of devices. The primary device is 
	// also enumerated with its proper name and GUID.

	// For purposes of this example we will ignore Primary Sound
	// Driver, and only add the primary device when it is enumerated
	// with its proper name and GUID.
	if (!pGUID)
		return TRUE;

	size_t strDescLen = _tcslen(strDesc) * sizeof(TCHAR);

	dwAudioDriverIndex += strDescLen;

	// Now add driver name string to our popup control

    HWND hSoundDeviceCombo = (HWND)pContext;

    // Add the string to the combo box
    LRESULT result = SendMessage( hSoundDeviceCombo, CB_ADDSTRING, 
                 0, (LPARAM) (LPCTSTR) strDesc );

    // Get the index of the string in the combo box
    INT nIndex = (INT)SendMessage( hSoundDeviceCombo, CB_FINDSTRING, 
                                   0, (LPARAM) (LPCTSTR) strDesc );

	// Save the GUID
	if (g_QT704Installed)
	{
		SendMessage( hSoundDeviceCombo, CB_SETITEMDATA, 
					nIndex, (LPARAM) pGUID );

		++guidArrayIndex;
	}

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT InitDirectSound( HWND hDlg )
{
    HRESULT hr;

    // Free any previous DirectSound objects
    FreeDirectSound();

    // Get the HWNDs the combo boxes
    HWND hSoundDeviceCombo   = GetDlgItem( hDlg, IDC_SOUND_DEVICE_COMBO );

    // Get the index of the currently selected devices
    INT nSoundIndex   = (INT)SendMessage( hSoundDeviceCombo,   CB_GETCURSEL, 0, 0 ); 

    // Get the GUID attached to the combo box item
    GUID* pSoundGUID = (GUID*) SendMessage( hSoundDeviceCombo, CB_GETITEMDATA, 
                                            nSoundIndex, 0 );

    // Create IDirectSound using the select sound device
    if( FAILED( hr = DirectSoundCreate( pSoundGUID, &g_pDS, NULL ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("DirectSoundCreate"), hr );

    // Release the IDirectSound object immediately since we don't want
    // to limit this sample to only computers that support full duplex audio
    SAFE_RELEASE( g_pDS ); 

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FreeDirectSound()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT FreeDirectSound()
{
    // Release DirectSound interfaces
    SAFE_RELEASE( g_pDS ); 

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CopySelectedSoundDeviceGUID()
// Desc: Makes a copy of the currently selected sound device GUID
//-----------------------------------------------------------------------------
void CopySelectedSoundDeviceGUID( HWND hDlg,  GUID *outCurrentSoundGUID)
{
    // Get the HWNDs the combo boxes
    HWND hSoundDeviceCombo   = GetDlgItem( hDlg, IDC_SOUND_DEVICE_COMBO );

    // Get the index of the currently selected devices
    INT nSoundIndex   = (INT)SendMessage( hSoundDeviceCombo, CB_GETCURSEL, 0, 0 ); 

	GUID *pGUID = NULL;

	// Get the GUID attached to the combo box item
    pGUID = (GUID*) SendMessage( hSoundDeviceCombo, CB_GETITEMDATA, 
                                            nSoundIndex, 0 );
	if (pGUID)
	{
		memcpy(outCurrentSoundGUID, pGUID, sizeof(GUID));
	}

}

//-----------------------------------------------------------------------------
// Name: CopySelectedSoundDeviceName()
// Desc: Makes a copy of the currently selected sound device name
//-----------------------------------------------------------------------------
void CopySelectedSoundDeviceName( HWND hDlg,  TCHAR *outCurrentSoundDeviceName)
{
    // Get the HWNDs the combo boxes
    HWND hSoundDeviceCombo   = GetDlgItem( hDlg, IDC_SOUND_DEVICE_COMBO );

    // Get the index of the currently selected devices
    INT nSoundIndex   = (INT)SendMessage( hSoundDeviceCombo, CB_GETCURSEL, 0, 0 ); 

	// Get the Device Name attached to the combo box item
    LRESULT strLength = SendMessage( hSoundDeviceCombo, CB_GETLBTEXT, 
                                     nSoundIndex, (LPARAM)outCurrentSoundDeviceName );
}

//-----------------------------------------------------------------------------
// Name: GetFile()
// Desc: Show the open file dialog and allow the user to select a file
//-----------------------------------------------------------------------------
BOOL GetFile (TCHAR *theFileName)
{
    static TCHAR strPath[MAX_PATH] = TEXT("");

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                         TEXT("Movie Files\0*.mov\0All Files\0*.*\0\0"), NULL,
                         0, 1, theFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open a movie file"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".mov"), 0, NULL, NULL };

    // Get the default media path (something like C:\WINDOWS\MEDIA)
    if( '\0' == strPath[0] )
    {
        if( GetWindowsDirectory( strPath, MAX_PATH ) != 0 )
        {
            StringCchCat( strPath, MAX_PATH, TEXT("\\MEDIA") );
        }
    }

     // Display the OpenFileName dialog. Then, try to load the specified file
    return (GetOpenFileName( &ofn ) );
}