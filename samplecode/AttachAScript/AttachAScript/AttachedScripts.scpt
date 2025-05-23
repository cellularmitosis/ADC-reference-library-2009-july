FasdUAS 1.101.10   ��   ��    k             l     ������  ��        l      �� 	��   	
9
3
File:		AttachedScripts.applescript

Description:
These are the AppleScripts called by the main program.  This file is compiled
at build time into the file AttachedScripts.scpt.  We have added two new build
phases to accomplish this.

Copyright: 	Copyright (c) 2006 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
copyrights in this original Apple software (the "Apple Software"), to use,
reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions of
the Apple Software.  Neither the name, trademarks, service marks or logos of
Apple Computer, Inc. may be used to endorse or promote products derived from the
Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied,
are granted by Apple herein, including but not limited to any patent rights that
may be infringed by your derivative works or by other works in which the Apple
Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
			
      
  
 l     ������  ��        l     ������  ��        l     ������  ��        l     ������  ��        l      �� ��   �� AttachedScripts.applescript

These are the AppleScripts called by the main program.  This file is compiled
at build time into the file AttachedScripts.scpt.  We have added two new build
phases to accomplish this.


1. The first build phase executes this command:

    osacompile -d -o AttachedScripts.scpt AttachedScripts.applescript

This command compiles this source file 'AttachedScripts.applescript' saving the result
in the data fork of the file 'AttachedScripts.scpt'.


2. The second build phase simply copies both of the files 'AttachedScripts.scpt'
and 'AttachedScripts.applescript' into the final application's resources directory.


IMPORTANT:  I have noticed that you need to 'clean' the build
before it will copy the compiled versions of these files over
to the resources directory.  



Some interesting points to make here are:

(a) if at any time you want to reconfigure your application so that the scripts
do different things you can do so by editing this file and recompiling it to the
.scpt file using this command:
    osacompile -d -o AttachedScripts.scpt AttachedScripts.applescript

(b) everything here is datafork based and does not require any resource forks.  As
such,  it's easily transportable to other file systems.

(c) Recompiling this script file does not require recompilation of your main
program, but it can significantly enhance the configurability of your application.
As well, it can defer some design and interoperability decisions until later in
the development cycle.  Want to swap in a different app for some special task?
Just rewrite the script, your main program doesn't have to know about it...

(d) recompiling this script is even something that daring advanced users
with special requirements may want to do.

(c) because the main program only loads the precompiled
'AttachedScripts.scpt' your application does not bear any of the runtime
compilation costs that are involved.  From the application's point of
view, it's just 'Load and go...'.

         l     ������  ��        l     ������  ��        l     ������  ��        l     ������  ��        l      �� ��   �� HookUpToRemoteMachine 
our app calls this script at application startup time.  In this handler
we present the url selection dialog allowing the user to select
a remote machine where the iTunes application we want to control
is running.  We store the remote machine address in the script's
property 'theRemoteURL' that is used by all of the other handlers
to direct commands to the iTunes app.  This handler returns an
integer value (1 or 0) indicating success or failure.          !   l     ������  ��   !  " # " j     �� $�� 0 theremoteurl theRemoteURL $ m      % %       #  & ' & l     ������  ��   '  ( ) ( i     * + * I      �������� .0 hookuptoremotemachine HookUpToRemoteMachine��  ��   + Q     4 , - . , k    * / /  0 1 0 r     2 3 2 I   
���� 4
�� .sysochururl     ��� null��   4 �� 5��
�� 
cusv 5 m    ��
�� essvesve��   3 o      ���� 0 theurl theURL 1  6 7 6 w     8 9 8 O     : ; : l     < = < r     > ? > 1    ��
�� 
pVol ? o      ���� 0 localvariable localVariable = 1 + try some command to verify the connection     ; n     @ A @ 4    �� B
�� 
capp B m     C C  iTunes    A 4    �� D
�� 
mach D o    ���� 0 theurl theURL 9�null     ߀��   
iTunes.app��  � ��(����G���݃}���      ���    ���P~��`�hook  alis    L  Macintosh HD               ���H+     
iTunes.app                                                      /;��&�        ����  	                Applications    �JS      �Η}         $Macintosh HD:Applications:iTunes.app   
 i T u n e s . a p p    M a c i n t o s h   H D  Applications/iTunes.app   / ��   7  E F E r     ' G H G o     !���� 0 theurl theURL H o      ���� 0 theremoteurl theRemoteURL F  I�� I L   ( * J J m   ( )���� ��   - R      ������
�� .ascrerr ****      � ****��  ��   . L   2 4 K K m   2 3����   )  L M L l     ������  ��   M  N O N l     ������  ��   O  P Q P l      �� R��   R ReportRemoteVolume 
This handler calls the remote iTunes application to obtain the current
volume setting - an integer value between 0 and 100.  NOTE:  this
is the volume setting inside of iTunes and it is not the same
as the output volume setting for the entire remote machine.     Q  S T S i    
 U V U I      �������� (0 reportremotevolume ReportRemoteVolume��  ��   V k      W W  X Y X r      Z [ Z m     ����   [ o      ���� 0 	thevolume 	theVolume Y  \ ] \ w     ^ 9 ^ O     _ ` _ r     a b a 1    ��
�� 
pVol b o      ���� 0 	thevolume 	theVolume ` n     c d c 4    �� e
�� 
capp e m     f f  iTunes    d 4    �� g
�� 
mach g o    ���� 0 theremoteurl theRemoteURL ]  h�� h L     i i o    ���� 0 	thevolume 	theVolume��   T  j k j l     ������  ��   k  l m l l     ������  ��   m  n o n l      �� p��   p SetRemoteVolume 
This handler calls the remote iTunes application to obtain the current
volume setting - an integer value between 0 and 100.  NOTE:  this
is the volume setting inside of iTunes and it is not the same
as the output volume setting for the entire remote machine.     o  q r q i     s t s I      �� u���� "0 setremotevolume SetRemoteVolume u  v�� v o      ���� 0 	newvolume 	newVolume��  ��   t w      w 9 w O     x y x r     z { z o    ���� 0 	newvolume 	newVolume { 1    ��
�� 
pVol y n     | } | 4   
 �� ~
�� 
capp ~ m        iTunes    } 4    
�� �
�� 
mach � o    	���� 0 theremoteurl theRemoteURL r  � � � l     ������  ��   �  � � � l     ������  ��   �  � � � l      �� ���   � � � ReportRemotePlayerState 
This handler calls the remote iTunes application to obtain the current
status of the player - a list of seven elements including
playing (0 or 1), playlist, track, position, duration,
statusstr, and volume .      �  � � � i     � � � I      �������� 20 reportremoteplayerstate ReportRemotePlayerState��  ��   � k     � � �  � � � r      � � � J     	 � �  � � � m     ����   �  � � � m     � �       �  � � � m     � �       �  � � � m    ����   �  � � � m    ����   �  � � � m     � �  Not Playing    �  ��� � m    ����  ��   � o      ���� 0 	theresult 	theResult �  � � � w    � � 9 � O    � � � � Z    � � ��� � � =   ! � � � 1    ��
�� 
pPlS � m     ��
�� ePlSkPSP � k   $ } � �  � � � l  $ $�� ���   �   set up the status string    �  � � � r   $ / � � � b   $ - � � � b   $ + � � � m   $ % � �  	Playing '    � n   % * � � � 1   ( *��
�� 
pnam � 1   % (��
�� 
pTrk � m   + , � �  ' by '    � o      ���� 0 	statusstr 	statusStr �  � � � r   0 ; � � � b   0 9 � � � b   0 7 � � � o   0 1���� 0 	statusstr 	statusStr � n   1 6 � � � 1   4 6��
�� 
pArt � 1   1 4��
�� 
pTrk � m   7 8 � �  ' from playlist '    � o      ���� 0 	statusstr 	statusStr �  � � � r   < K � � � b   < I � � � b   < E � � � o   < =���� 0 	statusstr 	statusStr � n   = D � � � 1   B D��
�� 
pnam � 1   = B��
�� 
pPla � m   E H � �  '    � o      ���� 0 	statusstr 	statusStr �  � � � l  L L�� ���   � #  put together the result list    �  � � � r   L ] � � � J   L [ � �  � � � m   L M����  �  � � � n   M T � � � 1   R T��
�� 
pnam � 1   M R��
�� 
pPla �  ��� � n   T Y � � � 1   W Y��
�� 
pnam � 1   T W��
�� 
pTrk��   � o      ���� 0 	theresult 	theResult �  � � � r   ^ p � � � b   ^ n � � � o   ^ _���� 0 	theresult 	theResult � J   _ m � �  � � � 1   _ d��
�� 
pPos �  ��� � n   d k � � � 1   g k��
�� 
pDur � 1   d g��
�� 
pTrk��   � o      ���� 0 	theresult 	theResult �  ��� � r   q } � � � b   q { � � � o   q r���� 0 	theresult 	theResult � J   r z � �  � � � o   r s���� 0 	statusstr 	statusStr �  ��� � 1   s x��
�� 
pVol��   � o      �� 0 	theresult 	theResult��  ��   � r   � � � � � J   � � � �  � � � m   � ��~�~   �  � � � m   � � � �       �    m   � �        m   � ��}�}    m   � ��|�|    m   � �		  Not Playing    
�{
 1   � ��z
�z 
pVol�{   � o      �y�y 0 	theresult 	theResult � n     4    �x
�x 
capp m      iTunes    4    �w
�w 
mach o    �v�v 0 theremoteurl theRemoteURL � �u L   � � o   � ��t�t 0 	theresult 	theResult�u   �  l     �s�r�s  �r    l     �q�p�q  �p    l      �o�o   � � GongCurrentTrack is called when the user clicks on the
gong button.  This handler disables the track that is currently
playing and skips ahead to the next track.  If the player is not
playing, this handler does nothing.       i     I      �n�m�l�n $0 gongcurrenttrack GongCurrentTrack�m  �l   w     * 9 O    * Z    ) !�k�j  =   "#" 1    �i
�i 
pPlS# m    �h
�h ePlSkPSP! k    %$$ %&% r    '(' m    �g
�g boovfals( n      )*) 1    �f
�f 
enbl* 1    �e
�e 
pTrk& +�d+ I    %�c�b�a
�c .hookNextnull        null�b  �a  �d  �k  �j   n    ,-, 4   
 �`.
�` 
capp. m    //  iTunes   - 4    
�_0
�_ 
mach0 o    	�^�^ 0 theremoteurl theRemoteURL 121 l     �]�\�]  �\  2 343 l     �[�Z�[  �Z  4 565 l      �Y7�Y  7 � � SwitchRemotePlayerState is called when the user clicks on the
play/pause button.  This routine simply turns the remote player on
or off.     6 898 i    :;: I      �X<�W�X 20 switchremoteplayerstate SwitchRemotePlayerState< =�V= o      �U�U 0 newstate newState�V  �W  ; w     $> 9> O    $?@? Z    #AB�TCA l   D�SD =   EFE o    �R�R 0 newstate newStateF m    �Q�Q �S  B I   �P�O�N
�P .hookPlaynull    ��� obj �O  �N  �T  C I   #�M�L�K
�M .hookPausnull        null�L  �K  @ n    GHG 4   
 �JI
�J 
cappI m    JJ  iTunes   H 4    
�IK
�I 
machK o    	�H�H 0 theremoteurl theRemoteURL9 LML l     �G�F�G  �F  M NON l     �E�D�E  �D  O PQP l      �CR�C  R � ~ GoToNextTrack is called when the user clicks on the
skip ahead button.  This routine advances the player to the
next track.     Q STS i    UVU I      �B�A�@�B 0 gotonexttrack GoToNextTrack�A  �@  V w     W 9W O    XYX I   �?�>�=
�? .hookNextnull        null�>  �=  Y n    Z[Z 4   
 �<\
�< 
capp\ m    ]]  iTunes   [ 4    
�;^
�; 
mach^ o    	�:�: 0 theremoteurl theRemoteURLT _`_ l     �9�8�9  �8  ` aba l     �7�6�7  �6  b cdc l      �5e�5  e � � GoToPreviousTrack is called when the user clicks on the
skip back button.  This routine asks the player to go back
to the previous track.     d fgf i    "hih I      �4�3�2�4 &0 gotoprevioustrack GoToPreviousTrack�3  �2  i w     j 9j O    klk I   �1�0�/
�1 .hookPrevnull        null�0  �/  l n    mnm 4   
 �.o
�. 
cappo m    pp  iTunes   n 4    
�-q
�- 
machq o    	�,�, 0 theremoteurl theRemoteURLg rsr l     �+�*�+  �*  s tut l     �)�(�)  �(  u vwv l      �'x�'  x � � GetPlaylistListing is called during program startup to retrieve
a list of the names of all of all of the playlists on the remote machine.     w yzy i   # &{|{ I      �&�%�$�& (0 getplaylistlisting GetPlaylistListing�%  �$  | k     !}} ~~ r     ��� J     �#�#  � o      �"�" 0 namelist nameList ��� w    � 9� O    ��� r    ��� e    �� n    ��� 1    �!
�! 
pnam� 2    � 
�  
cPly� o      �� 0 namelist nameList� n    ��� 4    ��
� 
capp� m    ��  iTunes   � 4    ��
� 
mach� o   	 �� 0 theremoteurl theRemoteURL� ��� L    !�� o     �� 0 namelist nameList�  z ��� l     ���  �  � ��� l     ���  �  � ��� l      ���  � � � PlayTrackFromPlaylist is when the user double clicks on a track name
in the track list.  This handler receives a playlist name and the name of
the track and it asks the player to play that track.    � ��� i   ' *��� I      ���� .0 playtrackfromplaylist PlayTrackFromPlaylist� ��� o      �� 0 playlistname playlistName� ��� o      �� 0 	trackname 	trackName�  �  � w     .� 9� O    .��� O    -��� O    ,��� O    +��� I  % *���
� .hookPlaynull    ��� obj �  �  � 4    "��
� 
cTrk� o     !�� 0 	trackname 	trackName� 4    �
�
�
 
cPly� o    �	�	 0 playlistname playlistName� 4    ��
� 
cSrc� m    ��  Library   � n    ��� 4   
 ��
� 
capp� m    ��  iTunes   � 4    
��
� 
mach� o    	�� 0 theremoteurl theRemoteURL� ��� l     ���  �  � ��� l     ���  �  � ��� l      � ��   � � � GetPlaylistTracks is called when ever the user clicks on a new playlist
name in the list of displayed playlists.  Here we return a list containing
all of the names of the tracks in the selected playlist.    � ��� i   + .��� I      ������� &0 getplaylisttracks GetPlaylistTracks� ���� o      ���� 0 playlistname playlistName��  ��  � k     1�� ��� r     ��� J     ����  � o      ���� 0 	thetracks 	theTracks� ��� w    .� 9� O    .��� O    -��� O    ,��� r   # +��� e   # )�� n   # )��� 1   & (��
�� 
pnam� 2   # &��
�� 
cTrk� o      ���� 0 	thetracks 	theTracks� 4     ���
�� 
cPly� o    ���� 0 playlistname playlistName� 4    ���
�� 
cSrc� m    ��  Library   � n    ��� 4    ���
�� 
capp� m    ��  iTunes   � 4    ���
�� 
mach� o   	 ���� 0 theremoteurl theRemoteURL� ���� L   / 1�� o   / 0���� 0 	thetracks 	theTracks��  � ��� l     ������  ��  � ��� l     ������  ��  � ��� l      �����  � � | GetPlaylistShuffle returns an integer value (0 or 1) reflecting
the status of the shuffle setting for the named playlist.     � ��� i   / 2��� I      ������� (0 getplaylistshuffle GetPlaylistShuffle� ���� o      ���� 0 playlistname playlistName��  ��  � k     8�� ��� r     ��� m     ����  � o      ����  0 shufflesetting shuffleSetting� ��� w    5� 9� O    5��� O    4��� O    3��� Z   " 2������ 1   " &��
�� 
pShf� r   ) ,��� m   ) *���� � o      ����  0 shufflesetting shuffleSetting��  � r   / 2��� m   / 0����  � o      ����  0 shufflesetting shuffleSetting� 4    �� 
�� 
cPly  o    ���� 0 playlistname playlistName� 4    ��
�� 
cSrc m      Library   � n     4    ��
�� 
capp m      iTunes    4    ��
�� 
mach o    ���� 0 theremoteurl theRemoteURL� �� L   6 8		 o   6 7����  0 shufflesetting shuffleSetting��  � 

 l     ������  ��    l     ������  ��    l      ����   � � SetPlaylistShuffle changes the current shuffle setting for
the named playlist to shuffleSetting.  shuffleSetting should
be an integer value of either 0 (for off) or 1 (for on).      i   3 6 I      ������ (0 setplaylistshuffle SetPlaylistShuffle  o      ���� 0 playlistname playlistName �� o      ����  0 shufflesetting shuffleSetting��  ��   w     4 9 O    4 O    3 O    2 Z    1 !��"  =   !#$# o    ����  0 shufflesetting shuffleSetting$ m     ���� ! r   $ )%&% m   $ %��
�� boovtrue& 1   % (��
�� 
pShf��  " r   , 1'(' m   , -��
�� boovfals( 1   - 0��
�� 
pShf 4    ��)
�� 
cPly) o    ���� 0 playlistname playlistName 4    ��*
�� 
cSrc* m    ++  Library    n    ,-, 4   
 ��.
�� 
capp. m    //  iTunes   - 4    
��0
�� 
mach0 o    	���� 0 theremoteurl theRemoteURL 121 l     ������  ��  2 343 l     ������  ��  4 565 l      ��7��  7 � � GetPlaylistRepeat returns an integer value of 0, for repeat off,
1, for repeat all, or 2, for repeat one, reflecting the state of
the repeat setting for the named playlist.      6 898 i   7 ::;: I      ��<���� &0 getplaylistrepeat GetPlaylistRepeat< =��= o      ���� 0 playlistname playlistName��  ��  ; k     S>> ?@? r     ABA m     ����  B o      ���� 0 repeatsetting repeatSetting@ CDC w    PE 9E O    PFGF O    OHIH O    NJKJ Z   " MLMN��L l  " 'O��O =  " 'PQP 1   " %��
�� 
pRptQ m   % &��
�� eRptkRpO��  M r   * -RSR m   * +����  S o      ���� 0 repeatsetting repeatSettingN TUT l  0 5V��V =  0 5WXW 1   0 3��
�� 
pRptX m   3 4��
�� eRptkRpA��  U YZY r   8 ;[\[ m   8 9���� \ o      ���� 0 repeatsetting repeatSettingZ ]^] l  > C_��_ =  > C`a` 1   > A��
�� 
pRpta m   A B��
�� eRptkRp1��  ^ b��b r   F Icdc m   F G���� d o      ���� 0 repeatsetting repeatSetting��  ��  K 4    ��e
�� 
cPlye o    ���� 0 playlistname playlistNameI 4    ��f
�� 
cSrcf m    gg  Library   G n    hih 4    ��j
�� 
cappj m    kk  iTunes   i 4    ��l
�� 
machl o    ���� 0 theremoteurl theRemoteURLD m��m L   Q Snn o   Q R���� 0 repeatsetting repeatSetting��  9 opo l     ������  ��  p qrq l     ������  ��  r sts l      ��u��  u � � SetPlaylistRepeat is called to change the repeat setting
for the named playlist.  repeatSetting should be a either
0, 1 or 2 representing 'repeat off', 'repeat all', or 
'repeat one' respectively.     t vwv i   ; >xyx I      ��z���� &0 setplaylistrepeat SetPlaylistRepeatz {|{ o      ���� 0 playlistname playlistName| }��} o      ���� 0 repeatsetting repeatSetting��  ��  y w     L~ 9~ O    L� O    K��� O    J��� Z    I������ l   !���� =   !��� o    ���� 0 repeatsetting repeatSetting� m     ����  ��  � r   $ )��� m   $ %��
�� eRptkRpO� 1   % (��
�� 
pRpt� ��� l  , /���� =  , /��� o   , -���� 0 repeatsetting repeatSetting� m   - .���� ��  � ��� r   2 7��� m   2 3��
�� eRptkRpA� 1   3 6��
�� 
pRpt� ��� l  : =���� =  : =��� o   : ;���� 0 repeatsetting repeatSetting� m   ; <���� ��  � ���� r   @ E��� m   @ A��
�� eRptkRp1� 1   A D��
�� 
pRpt��  ��  � 4    ��
� 
cPly� o    �~�~ 0 playlistname playlistName� 4    �}�
�} 
cSrc� m    ��  Library   � n    ��� 4   
 �|�
�| 
capp� m    ��  iTunes   � 4    
�{�
�{ 
mach� o    	�z�z 0 theremoteurl theRemoteURLw ��� l     �y�x�y  �x  � ��� l     �w�v�w  �v  � ��u� l     �t�s�t  �s  �u       �r� %����������������r  � �q�p�o�n�m�l�k�j�i�h�g�f�e�d�c�b�q 0 theremoteurl theRemoteURL�p .0 hookuptoremotemachine HookUpToRemoteMachine�o (0 reportremotevolume ReportRemoteVolume�n "0 setremotevolume SetRemoteVolume�m 20 reportremoteplayerstate ReportRemotePlayerState�l $0 gongcurrenttrack GongCurrentTrack�k 20 switchremoteplayerstate SwitchRemotePlayerState�j 0 gotonexttrack GoToNextTrack�i &0 gotoprevioustrack GoToPreviousTrack�h (0 getplaylistlisting GetPlaylistListing�g .0 playtrackfromplaylist PlayTrackFromPlaylist�f &0 getplaylisttracks GetPlaylistTracks�e (0 getplaylistshuffle GetPlaylistShuffle�d (0 setplaylistshuffle SetPlaylistShuffle�c &0 getplaylistrepeat GetPlaylistRepeat�b &0 setplaylistrepeat SetPlaylistRepeat� �a +�`�_���^�a .0 hookuptoremotemachine HookUpToRemoteMachine�`  �_  � �]�\�] 0 theurl theURL�\ 0 localvariable localVariable� 
�[�Z�Y 9�X�W C�V�U�T
�[ 
cusv
�Z essvesve
�Y .sysochururl     ��� null
�X 
mach
�W 
capp
�V 
pVol�U  �T  �^ 5 ,*��l E�O�Z*�/��/ *�,E�UO�Ec   OkW 	X  	j� �S V�R�Q���P�S (0 reportremotevolume ReportRemoteVolume�R  �Q  � �O�O 0 	thevolume 	theVolume�  9�N�M f�L
�N 
mach
�M 
capp
�L 
pVol�P jE�O�Z*�b   /��/ *�,E�UO�� �K t�J�I���H�K "0 setremotevolume SetRemoteVolume�J �G��G �  �F�F 0 	newvolume 	newVolume�I  � �E�E 0 	newvolume 	newVolume�  9�D�C �B
�D 
mach
�C 
capp
�B 
pVol�H �Z*�b   /��/ �*�,FU� �A ��@�?���>�A 20 reportremoteplayerstate ReportRemotePlayerState�@  �?  � �=�<�= 0 	theresult 	theResult�< 0 	statusstr 	statusStr�  � � ��; 9�:�9�8�7 ��6�5 ��4 ��3 ��2�1�0 �	�; 
�: 
mach
�9 
capp
�8 
pPlS
�7 ePlSkPSP
�6 
pTrk
�5 
pnam
�4 
pArt
�3 
pPla
�2 
pPos
�1 
pDur
�0 
pVol�> �j��jj�j�vE�O�Z*�b   /��/ {*�,�  ^�*�,�,%�%E�O�*�,�,%�%E�O�*a ,�,%a %E�Ok*a ,�,*�,�,mvE�O�*a ,*�,a ,lv%E�O��*a ,lv%E�Y ja a jja *a ,�vE�UO�� �/�.�-���,�/ $0 gongcurrenttrack GongCurrentTrack�.  �-  �  � 	 9�+�*/�)�(�'�&�%
�+ 
mach
�* 
capp
�) 
pPlS
�( ePlSkPSP
�' 
pTrk
�& 
enbl
�% .hookNextnull        null�, +�Z*�b   /��/ *�,�  f*�,�,FO*j Y hU� �$;�#�"���!�$ 20 switchremoteplayerstate SwitchRemotePlayerState�# � ��  �  �� 0 newstate newState�"  � �� 0 newstate newState�  9��J��
� 
mach
� 
capp
� .hookPlaynull    ��� obj 
� .hookPausnull        null�! %�Z*�b   /��/ �k  
*j Y *j U� �V������ 0 gotonexttrack GoToNextTrack�  �  �  �  9��]�
� 
mach
� 
capp
� .hookNextnull        null� �Z*�b   /��/ *j U� �i������ &0 gotoprevioustrack GoToPreviousTrack�  �  �  �  9��p�
� 
mach
� 
capp
� .hookPrevnull        null� �Z*�b   /��/ *j U� �|�
�	���� (0 getplaylistlisting GetPlaylistListing�
  �	  � �� 0 namelist nameList�  9�����
� 
mach
� 
capp
� 
cPly
� 
pnam� "jvE�O�Z*�b   /��/ 
*�-�,EE�UO�� ���� ����� .0 playtrackfromplaylist PlayTrackFromPlaylist� ����� �  ������ 0 playlistname playlistName�� 0 	trackname 	trackName�   � ������ 0 playlistname playlistName�� 0 	trackname 	trackName� 	 9��������������
�� 
mach
�� 
capp
�� 
cSrc
�� 
cPly
�� 
cTrk
�� .hookPlaynull    ��� obj �� /�Z*�b   /��/ *��/ *�/ *�/ *j UUUU� ������������� &0 getplaylisttracks GetPlaylistTracks�� ����� �  ���� 0 playlistname playlistName��  � ������ 0 playlistname playlistName�� 0 	thetracks 	theTracks� 	 9��������������
�� 
mach
�� 
capp
�� 
cSrc
�� 
cPly
�� 
cTrk
�� 
pnam�� 2jvE�O�Z*�b   /��/ *��/ *�/ 
*�-�,EE�UUUO�� ������������� (0 getplaylistshuffle GetPlaylistShuffle�� ����� �  ���� 0 playlistname playlistName��  � ������ 0 playlistname playlistName��  0 shufflesetting shuffleSetting�  9����������
�� 
mach
�� 
capp
�� 
cSrc
�� 
cPly
�� 
pShf�� 9jE�O�Z*�b   /��/ "*��/ *�/ *�,E kE�Y jE�UUUO�� ������������ (0 setplaylistshuffle SetPlaylistShuffle�� ����� �  ������ 0 playlistname playlistName��  0 shufflesetting shuffleSetting��  � ������ 0 playlistname playlistName��  0 shufflesetting shuffleSetting�  9����/��+����
�� 
mach
�� 
capp
�� 
cSrc
�� 
cPly
�� 
pShf�� 5�Z*�b   /��/ %*��/ *�/ �k  
e*�,FY f*�,FUUU� ��;���������� &0 getplaylistrepeat GetPlaylistRepeat�� ����� �  ���� 0 playlistname playlistName��  � ������ 0 playlistname playlistName�� 0 repeatsetting repeatSetting�  9����k��g����������
�� 
mach
�� 
capp
�� 
cSrc
�� 
cPly
�� 
pRpt
�� eRptkRpO
�� eRptkRpA
�� eRptkRp1�� TjE�O�Z*�b   /��/ =*��/ 5*�/ -*�,�  jE�Y *�,�  kE�Y *�,�  lE�Y hUUUO�� ��y���������� &0 setplaylistrepeat SetPlaylistRepeat�� ����� �  ������ 0 playlistname playlistName�� 0 repeatsetting repeatSetting��  � ������ 0 playlistname playlistName�� 0 repeatsetting repeatSetting�  9������������������
�� 
mach
�� 
capp
�� 
cSrc
�� 
cPly
�� eRptkRpO
�� 
pRpt
�� eRptkRpA
�� eRptkRp1�� M�Z*�b   /��/ =*��/ 5*�/ -�j  
�*�,FY �k  
�*�,FY �l  
�*�,FY hUUU ascr  ��ޭ