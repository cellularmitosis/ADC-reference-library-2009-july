FasdUAS 1.101.10   ��   ��    k             l     �� ��      testAS.applescript       	  l     �� 
��   
  
 TextEdit+    	     l     ������  ��        l     �� ��    e _ When user clicks Update button get statistics information and then set contents of text fields         i         I     �� ��
�� .coVScliInull���    obj   o      ���� 0 	theobject 	theObject��    k            r         n         m    ��
�� 
cwin  o     ���� 0 	theobject 	theObject  o      ���� 0 	thewindow 	theWindow   ��  Q         I   	 ��  ���� (0 populatetextfields populateTextFields    !�� ! o   
 ���� 0 	thewindow 	theWindow��  ��    R      �� "��
�� .ascrerr ****      � **** " o      ���� 0 errmsg errMsg��    I   �� #��
�� .ascrcmnt****      � **** # o    ���� 0 errmsg errMsg��  ��     $ % $ l     ������  ��   %  & ' & l     �� (��   ( � | When Document Properties window is open, clear text fields, get statistics information and then set contents of text fields    '  ) * ) i     + , + I     �� -��
�� .appSopeEnull���    obj  - o      ���� 0 	theobject 	theObject��   , k     3 . .  / 0 / r      1 2 1 o     ���� 0 	theobject 	theObject 2 o      ���� 0 	thewindow 	theWindow 0  3 4 3 O    , 5 6 5 k    + 7 7  8 9 8 r     : ; : m    	 < <       ; n       = > = 1    ��
�� 
pcnt > n   	  ? @ ? 4    �� A
�� 
texF A m     B B  CharacterCount    @ 4   	 �� C
�� 
boxO C m     D D  
Statistics    9  E F E r     G H G m     I I       H n       J K J 1    ��
�� 
pcnt K n     L M L 4    �� N
�� 
texF N m     O O  	WordCount    M 4    �� P
�� 
boxO P m     Q Q  
Statistics    F  R�� R r     + S T S m     ! U U       T n       V W V 1   ( *��
�� 
pcnt W n   ! ( X Y X 4   % (�� Z
�� 
texF Z m   & ' [ [  ParagraphCount    Y 4   ! %�� \
�� 
boxO \ m   # $ ] ]  
Statistics   ��   6 o    ���� 0 	thewindow 	theWindow 4  ^�� ^ I   - 3�� _���� (0 populatetextfields populateTextFields _  `�� ` o   . /���� 0 	thewindow 	theWindow��  ��  ��   *  a b a l     ������  ��   b  c d c l     �� e��   e C = Ask TextEdit for document text information using AppleScript    d  f g f i     h i h I      �������� 0 doccount DocCount��  ��   i k     7 j j  k l k O     ) m n m k    ( o o  p q p I   	������
�� .miscactvnull��� ��� null��  ��   q  r s r r   
  t u t n   
  v w v m    ��
�� 
ctxt w l  
  x�� x 4  
 �� y
�� 
docu y m    ���� ��   u o      ���� 0 thedata theData s  z { z r     | } | n     ~  ~ 1    ��
�� 
leng  o    ���� 0 thedata theData } o      ���� 0 	charcount 	charCount {  � � � r      � � � n     � � � m    ��
�� 
nmbr � n    � � � 2   ��
�� 
cwor � o    ���� 0 thedata theData � o      ���� 0 	wordcount 	wordCount �  ��� � r   ! ( � � � n   ! & � � � m   $ &��
�� 
nmbr � n  ! $ � � � 2  " $��
�� 
cpar � o   ! "���� 0 thedata theData � o      ����  0 paragraphcount paragraphCount��   n m      � ��null     ߀��   TextEdit.app������I�������^ �    ���������G�������I�������ʐ  ttxt   alis    T  Macintosh HD               ���H+     TextEdit.app                                                     h���3�        ����  	                Applications    �JS      �Ԥ         &Macintosh HD:Applications:TextEdit.app    T e x t E d i t . a p p    M a c i n t o s h   H D  Applications/TextEdit.app   / ��   l  � � � r   * 4 � � � K   * 2 � � �� � ��� 0 	charcount 	charCount � o   + ,���� 0 	charcount 	charCount � �� � ��� 0 	wordcount 	wordCount � o   - .���� 0 	wordcount 	wordCount � �� �����  0 paragraphcount paragraphCount � o   / 0����  0 paragraphcount paragraphCount��   � o      ���� (0 doccountinforecord DocCountInfoRecord �  ��� � L   5 7 � � o   5 6���� (0 doccountinforecord DocCountInfoRecord��   g  � � � l     ������  ��   �  � � � l     �� ���   � = 7Set contents of text fields with statistics information    �  � � � i     � � � I      �� ����� (0 populatetextfields populateTextFields �  ��� � o      ���� 0 	thewindow 	theWindow��  ��   � Q     n � � � � k    a � �  � � � O    � � � I   ������
�� .coVSstaAnull���    obj ��  ��   � n    
 � � � 4    
�� �
�� 
proI � m    	 � �  DocCount    � n     � � � 4    �� �
�� 
boxO � m     � �  
Statistics    � o    ���� 0 	thewindow 	theWindow �  � � � r     � � � I    �������� 0 doccount DocCount��  ��   � o      ���� (0 doccountinforecord DocCountInfoRecord �  ��� � O    a � � � k     ` � �  � � � r     - � � � n     # � � � o   ! #���� 0 	charcount 	charCount � o     !���� (0 doccountinforecord DocCountInfoRecord � n       � � � 1   * ,��
�� 
pcnt � n   # * � � � 4   ' *�� �
�� 
texF � m   ( ) � �  CharacterCount    � 4   # '�� �
�� 
boxO � m   % & � �  
Statistics    �  � � � r   . ; � � � n   . 1 � � � o   / 1���� 0 	wordcount 	wordCount � o   . /���� (0 doccountinforecord DocCountInfoRecord � n       � � � 1   8 :��
�� 
pcnt � n   1 8 � � � 4   5 8�� �
�� 
texF � m   6 7 � �  	WordCount    � 4   1 5�� �
�� 
boxO � m   3 4 � �  
Statistics    �  � � � r   < K � � � n   < ? � � � o   = ?����  0 paragraphcount paragraphCount � o   < =���� (0 doccountinforecord DocCountInfoRecord � n       � � � 1   H J��
�� 
pcnt � n   ? H � � � 4   C H�� �
�� 
texF � m   D G � �  ParagraphCount    � 4   ? C�� �
�� 
boxO � m   A B � �  
Statistics    �  ��� � O  L ` � � � I  Z _������
�� .coVSstoTnull���    obj ��  ��   � n   L W � � � 4   R W�� �
�� 
proI � m   S V � �  DocCount    � 4   L R�� �
�� 
boxO � m   N Q � �  
Statistics   ��   � o    ���� 0 	thewindow 	theWindow��   � R      �� ���
�� .ascrerr ****      � **** � o      ���� 0 errmsg errMsg��   � I  i n�� ���
�� .ascrcmnt****      � **** � o   i j�� 0 errmsg errMsg��   �  � � � l     �~�}�~  �}   �  � � � l      �| ��|   �	>	8
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

Copyright © 2006 Apple Computer, Inc., All Rights Reserved
    �  � � � l     �{�z�{  �z   �  ��y � j    �x ��x 60 asdscriptuniqueidentifier ASDScriptUniqueIdentifier � m     � �  testAS.applescript   �y       �w � �  ��w   � �v�u�t�s�r
�v .coVScliInull���    obj 
�u .appSopeEnull���    obj �t 0 doccount DocCount�s (0 populatetextfields populateTextFields�r 60 asdscriptuniqueidentifier ASDScriptUniqueIdentifier � �q �p�o�n
�q .coVScliInull���    obj �p 0 	theobject 	theObject�o   �m�l�k�m 0 	theobject 	theObject�l 0 	thewindow 	theWindow�k 0 errmsg errMsg �j�i�h�g�f
�j 
cwin�i (0 populatetextfields populateTextFields�h 0 errmsg errMsg�g  
�f .ascrcmnt****      � ****�n ��,E�O *�k+ W X  �j   �e ,�d�c�b
�e .appSopeEnull���    obj �d 0 	theobject 	theObject�c   �a�`�a 0 	theobject 	theObject�` 0 	thewindow 	theWindow  <�_ D�^ B�] I Q O U ] [�\
�_ 
boxO
�^ 
texF
�] 
pcnt�\ (0 populatetextfields populateTextFields�b 4�E�O� %�*��/��/�,FO�*��/��/�,FO�*��/��/�,FUO*�k+  �[ i�Z�Y�X�[ 0 doccount DocCount�Z  �Y   �W�V�U�T�S�W 0 thedata theData�V 0 	charcount 	charCount�U 0 	wordcount 	wordCount�T  0 paragraphcount paragraphCount�S (0 doccountinforecord DocCountInfoRecord  ��R�Q�P�O�N�M�L�K�J�I�H
�R .miscactvnull��� ��� null
�Q 
docu
�P 
ctxt
�O 
leng
�N 
cwor
�M 
nmbr
�L 
cpar�K 0 	charcount 	charCount�J 0 	wordcount 	wordCount�I  0 paragraphcount paragraphCount�H �X 8� &*j O*�k/�-E�O��,E�O��-�,E�O��-�,E�UO����E�O� �G ��F�E	
�D�G (0 populatetextfields populateTextFields�F �C�C   �B�B 0 	thewindow 	theWindow�E  	 �A�@�?�A 0 	thewindow 	theWindow�@ (0 doccountinforecord DocCountInfoRecord�? 0 errmsg errMsg
 �> ��= ��<�;�: ��9 ��8�7 � ��6 � � � ��5�4�3�2
�> 
boxO
�= 
proI
�< .coVSstaAnull���    obj �; 0 doccount DocCount�: 0 	charcount 	charCount
�9 
texF
�8 
pcnt�7 0 	wordcount 	wordCount�6  0 paragraphcount paragraphCount
�5 .coVSstoTnull���    obj �4 0 errmsg errMsg�3  
�2 .ascrcmnt****      � ****�D o c���/��/ *j UO*j+ E�O� B��,*��/��/�,FO��,*��/��/�,FO��,*��/�a /�,FO*�a /�a / *j UUW X  �j  ascr  ��ޭ