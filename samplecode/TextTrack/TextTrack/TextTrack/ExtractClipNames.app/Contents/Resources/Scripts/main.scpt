FasdUAS 1.101.10   ��   ��    k             l      �� ��   	�	�

File: ExtractClipNames

Abstract: Give droplet functionality to a perl script 'extract'

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright � 2005 Apple Computer, Inc., All Rights Reserved

       	  l     ������  ��   	  
  
 l     �� ��    N H NOTE: do not use periods (.) with the items in the name extensions list         l     �� ��    R L eg: {"txt", "text", "jpg", "jpeg"}, NOT: {".txt", ".text", ".jpg", ".jpeg"}         j     �� �� 0 extension_list    J         ��  m        	 xml   ��        l     �� ��    W Q whether to post an alert when non-processible items are dragged onto the droplet         j    �� �� 0 
post_alert    m    ��
�� boovtrue      l     ������  ��        l     ��  ��     ; 5 This droplet processes files dropped onto the applet      ! " ! i     # $ # I     �� %��
�� .aevtodocnull  �    alis % o      ���� 0 these_items  ��   $ Y     � &�� ' (�� & k    � ) )  * + * r     , - , n     . / . 4    �� 0
�� 
cobj 0 o    ���� 0 i   / o    ���� 0 these_items   - o      ���� 0 	this_item   +  1 2 1 r     3 4 3 I   �� 5��
�� .sysonfo4asfe        file 5 o    ���� 0 	this_item  ��   4 l      6�� 6 o      ���� 0 	item_info  ��   2  7�� 7 Z    � 8 9 :�� 8 F    : ; < ; F    , = > = l   " ?�� ? =   " @ A @ n      B C B 1     ��
�� 
asdr C l    D�� D o    ���� 0 	item_info  ��   A m     !��
�� boovfals��   > l 	 % * E�� E l  % * F�� F =  % * G H G n   % ( I J I m   & (��
�� 
alis J l  % & K�� K o   % &���� 0 	item_info  ��   H m   ( )��
�� boovfals��  ��   < E  / 8 L M L l  / 4 N�� N o   / 4���� 0 extension_list  ��   M l 	 4 7 O�� O l  4 7 P�� P n   4 7 Q R Q 1   5 7��
�� 
nmxt R l  4 5 S�� S o   4 5���� 0 	item_info  ��  ��  ��   9 I   = C�� T���� 0 process_item   T  U�� U o   > ?���� 0 	this_item  ��  ��   :  V W V =  F M X Y X o   F K���� 0 
post_alert   Y m   K L��
�� boovtrue W  Z�� Z k   P � [ [  \ ] \ r   P U ^ _ ^ m   P Q ` `  ,     _ n      a b a 1   R T��
�� 
txdl b 1   Q R��
�� 
ascr ]  c d c r   V _ e f e c   V ] g h g o   V [���� 0 extension_list   h m   [ \��
�� 
TEXT f l      i�� i o      ���� 0 display_list  ��   d  j k j r   ` e l m l m   ` a n n       m n      o p o 1   b d��
�� 
txdl p 1   a b��
�� 
ascr k  q�� q O   f � r s r k   s � t t  u v u I  s x������
�� .sysobeepnull��� ��� long��  ��   v  w�� w I  y ��� x y
�� .sysodlogaskr        TEXT x b   y � z { z b   y � | } | b   y � ~  ~ m   y | � � 7 1This droplet only processes files with extension:     l 	 |  ��� � o   | ��
�� 
ret ��   } o   � ���
�� 
ret  { o   � ����� 0 display_list   y �� ���
�� 
givu � m   � ����� ��  ��   s 4   f p�� �
�� 
capp � l  h o ��� � I  h o�� � �
�� .earsffdralis        afdr � m   h i��
�� appfegfp � �� ���
�� 
rtyp � m   j k��
�� 
TEXT��  ��  ��  ��  ��  ��  �� 0 i   ' m    ����  ( l   	 ��� � I   	�� ���
�� .corecnte****       **** � o    ���� 0 these_items  ��  ��  ��   "  � � � l     ������  ��   �  � � � l     �� ���   � ' ! this sub-routine processes files    �  ��� � i     � � � I      �� ����� 0 process_item   �  ��� � o      ���� 0 	this_item  ��  ��   � k     a � �  � � � l     �� ���   �   locate where things are    �  � � � r     	 � � � n      � � � 1    ��
�� 
psxp � l     ��� � I    �� ���
�� .earsffdralis        afdr �  f     ��  ��   � o      ���� 0 base   �  � � � r   
  � � � b   
  � � � o   
 ���� 0 base   � m     � �  ..    � o      ���� 0 	local_dir   �  � � � r     � � � b     � � � o    ���� 0 base   � m     � � ! Contents/Resources/Scripts/    � o      ���� 0 script_base   �  � � � l   ������  ��   �  � � � l   �� ���   � ' ! find out what to call the output    �  � � � r     � � � I   �� � �
�� .sysodlogaskr        TEXT � m     � �   Enter a name for the file:    � �� ���
�� 
dtxt � m     � � 
 list   ��   � o      ���� 0 r   �  � � � l     ������  ��   �  ��� � Z     a � ����� � =    % � � � n     # � � � 1   ! #��
�� 
bhit � o     !���� 0 r   � m   # $ � �  OK    � k   ( ] � �  � � � l  ( (�� ���   � D > make a bunch of strings so the 'do shell script' looks better    �  � � � r   ( / � � � n   ( - � � � 1   + -��
�� 
strq � n   ( + � � � 1   ) +��
�� 
psxp � o   ( )���� 0 	local_dir   � o      ���� 0 	local_dir   �  � � � r   0 7 � � � n   0 5 � � � 1   3 5��
�� 
strq � l  0 3 ��� � b   0 3 � � � o   0 1���� 0 script_base   � m   1 2 � �  extract   ��   � o      �� 0 perl_script   �  � � � r   8 ? � � � n   8 = � � � 1   ; =�~
�~ 
strq � n   8 ; � � � 1   9 ;�}
�} 
psxp � o   8 9�|�| 0 	this_item   � o      �{�{ 0 	user_file   �  � � � r   @ G � � � n   @ E � � � 1   C E�z
�z 
strq � n   @ C � � � 1   A C�y
�y 
ttxt � o   @ A�x�x 0 r   � o      �w�w 0 output_file   �  � � � l  H H�v�u�v  �u   �  ��t � I  H ]�s ��r
�s .sysoexecTEXT���     TEXT � b   H Y � � � b   H W � � � b   H S � � � b   H Q � � � b   H O � � � b   H M � � � b   H K � � � l 	 H I ��q � m   H I � � 	 cd    �q   � o   I J�p�p 0 	local_dir   � m   K L    ;     � l 	 M N�o o   M N�n�n 0 perl_script  �o   � m   O P       � o   Q R�m�m 0 	user_file   � m   S V 	  >     � o   W X�l�l 0 output_file  �r  �t  ��  ��  ��  ��       �k�j�k   �i�h�g�f�i 0 extension_list  �h 0 
post_alert  
�g .aevtodocnull  �    alis�f 0 process_item   �e�e    
�j boovtrue �d $�c�b	
�a
�d .aevtodocnull  �    alis�c 0 these_items  �b  	 �`�_�^�]�\�` 0 these_items  �_ 0 i  �^ 0 	this_item  �] 0 	item_info  �\ 0 display_list  
 �[�Z�Y�X�W�V�U�T `�S�R�Q n�P�O�N�M�L ��K�J�I�H
�[ .corecnte****       ****
�Z 
cobj
�Y .sysonfo4asfe        file
�X 
asdr
�W 
alis
�V 
bool
�U 
nmxt�T 0 process_item  
�S 
ascr
�R 
txdl
�Q 
TEXT
�P 
capp
�O appfegfp
�N 
rtyp
�M .earsffdralis        afdr
�L .sysobeepnull��� ��� long
�K 
ret 
�J 
givu�I 
�H .sysodlogaskr        TEXT�a � �k�j  kh ��/E�O�j E�O��,f 	 	��,f �&	 b   ��,�& *�k+ Y Qb  e  F���,FOb   �&E�O���,FO*����l / *j Oa _ %_ %�%a a l UY h[OY�s �G ��F�E�D�G 0 process_item  �F �C�C   �B�B 0 	this_item  �E   �A�@�?�>�=�<�;�:�A 0 	this_item  �@ 0 base  �? 0 	local_dir  �> 0 script_base  �= 0 r  �< 0 perl_script  �; 0 	user_file  �: 0 output_file   �9�8 � � ��7 ��6�5 ��4 ��3 � �2
�9 .earsffdralis        afdr
�8 
psxp
�7 
dtxt
�6 .sysodlogaskr        TEXT
�5 
bhit
�4 
strq
�3 
ttxt
�2 .sysoexecTEXT���     TEXT�D b)j  �,E�O��%E�O��%E�O���l E�O��,�  :��,�,E�O��%�,E�O��,�,E�O��,�,E�O��%�%�%�%�%a %�%j Y h ascr  ��ޭ