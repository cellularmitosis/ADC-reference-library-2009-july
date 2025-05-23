FasdUAS 1.101.10   ��   ��    k             l      �� ��    � �
This script will automatically update any referenced projects in the currently active Xcode project to version 2.1, recursively updating paths and sub-referenced projects.
       	  l     ������  ��   	  
  
 l     ������  ��        i         I     ������
�� .aevtoappnull  �   � ****��  ��    k            l     �� ��    F @ This updates the subprojects in the frontmost project in Xcode.      ��  I     �������� 40 updatecurrentprojectrefs updateCurrentProjectRefs��  ��  ��        l     ������  ��        i        I      �������� 40 updatecurrentprojectrefs updateCurrentProjectRefs��  ��    k    8       l     �� ��    ; 5 Set up a queue of subprojects and their subprojects.          r      ! " ! J     ����   " o      ���� 0 projreflist projRefList    # $ # O    i % & % k   	 h ' '  ( ) ( l  	 	�� *��   * 5 / Add current project's subprojects to the queue    )  + , + r   	  - . - 1   	 ��
�� 
apdc . o      ���� 0 
activeproj 
activeProj ,  / 0 / r     1 2 1 c     3 4 3 n     5 6 5 1    ��
�� 
rgru 6 o    ���� 0 
activeproj 
activeProj 4 m    ��
�� 
list 2 o      ���� 0 	grouplist 	groupList 0  7�� 7 W    h 8 9 8 k   % c : :  ; < ; r   % / = > = b   % - ? @ ? o   % &���� 0 	grouplist 	groupList @ n   & , A B A 2  * ,��
�� 
grup B n   & * C D C 4  ' *�� E
�� 
cobj E m   ( )����  D o   & '���� 0 	grouplist 	groupList > o      ���� 0 	grouplist 	groupList <  F G F r   0 C H I H b   0 A J K J o   0 1���� 0 projreflist projRefList K l  1 @ L�� L 6  1 @ M N M n   1 7 O P O 2   5 7��
�� 
filr P l  1 5 Q�� Q n   1 5 R S R 4  2 5�� T
�� 
cobj T m   3 4����  S o   1 2���� 0 	grouplist 	groupList��   N =  8 ? U V U 1   9 ;��
�� 
filT V m   < > W W  wrapper.pb-project   ��   I o      ���� 0 projreflist projRefList G  X Y X Z  D U Z [���� Z =   D M \ ] \ l  D K ^�� ^ I  D K�� _��
�� .corecnte****       **** _ n   D G ` a ` 2  E G��
�� 
cobj a o   D E���� 0 	grouplist 	groupList��  ��   ] m   K L����  [  S   P Q��  ��   Y  b�� b r   V c c d c n   V a e f e 7  W a�� g h
�� 
cobj g m   [ ]����  h m   ^ `������ f o   V W���� 0 	grouplist 	groupList d o      ���� 0 	grouplist 	groupList��   9 =    $ i j i l   " k�� k I   "�� l��
�� .corecnte****       **** l n     m n m 2   ��
�� 
cobj n o    ���� 0 	grouplist 	groupList��  ��   j m   " #����  ��   & m     o o�null     ߀�� �a	Xcode.app���0    �2@<��(���P  ��(   )       �(�1l���׀ xcde   alis    N  Nimitz                     ��1�H+   �a	Xcode.app                                                       ���PK        ����  	                Applications    ����      ����     �a �#  'Nimitz:Developer:Applications:Xcode.app    	 X c o d e . a p p    N i m i t z   Developer/Applications/Xcode.app  / ��   $  p q p l  j j������  ��   q  r s r X   j6 t�� u t k   z1 v v  w x w l  z z�� y��   y &   Upgrade subproject if necessary    x  z { z Z   z/ | }���� | l  z � ~�� ~ H   z �   D   z  � � � n   z } � � � 1   { }��
�� 
ppth � o   z {���� 0 
subproject 
subProject � m   } ~ � �  
.xcodeproj   ��   } k   �+ � �  � � � O  � � � � � r   � � � � � n   � � � � � 1   � ���
�� 
abph � o   � ����� 0 
subproject 
subProject � o      ����  0 subprojectpath subProjectPath � m   � � o �  � � � r   � � � � � 4   � ��� �
�� 
psxf � o   � �����  0 subprojectpath subProjectPath � o      ���� "0 subprojectalias subProjectAlias �  � � � O   � � � � � k   � � � �  � � � l  � ��� ���   � 6 0 Rename the upgraded project's project reference    �  � � � r   � � � � � n  � � � � � 1   � ���
�� 
txdl � 1   � ���
�� 
ascr � o      ���� 0 revdel revDel �  � � � r   � � � � � J   � � � �  ��� � m   � � � �  /   ��   � n      � � � 1   � ���
�� 
txdl � 1   � ���
�� 
ascr �  � � � r   � � � � � n   � � � � � 4  � ��� �
�� 
citm � m   � ������� � l  � � ��� � c   � � � � � n   � � � � � 1   � ���
�� 
abph � o   � ����� 0 
subproject 
subProject � m   � ���
�� 
TEXT��   � o      ���� 0 fname fName �  � � � r   � � � � � o   � ����� 0 revdel revDel � n      � � � 1   � ���
�� 
txdl � 1   � ���
�� 
ascr �  � � � r   � � � � � b   � � � � � l  � � ��� � o   � ����� 0 fname fName��   � m   � � � � 
 proj    � o      ���� 0 upgradename upgradeName �  ��� � r   � � � � � o   � ����� 0 upgradename upgradeName � n       � � � 1   � ���
�� 
pnam � o   � ����� 0 
subproject 
subProject��   � m   � � o �  � � � l  � �������  ��   �  � � � Q   �) � � � � l  � � � � k   � � �  � � � O  � � � � � I  � ��� � �
�� .coreuppfnull���    alis � o   � ����� "0 subprojectalias subProjectAlias � �� ���
�� 
as   � o   � ����� 0 upgradename upgradeName��   � m   � � o �  � � � I  � ��� ���
�� .sysodelanull��� ��� nmbr � m   � ����� ��   �  � � � I   � ��������� 40 updatecurrentprojectrefs updateCurrentProjectRefs��  ��   �  � � � I  � ��� ���
�� .sysodelanull��� ��� nmbr � m   � ����� ��   �  � � � l  � ��� ���   � 0 * Move on to the next project in the queue.    �  ��� � O   � � � � k   � �  � � � I 
�� ���
�� .coreclos****      � **** � 1  ��
�� 
apdc��   �  ��� � r   � � � l  ��� � b   � � � l  ��� � e   � � n   � � � 1  �
� 
ppth � o  �~�~ 0 
subproject 
subProject��   � m   � � 
 proj   ��   � n       � � � 1  �}
�} 
ppth � o  �|�| 0 
subproject 
subProject��   � m   �  o��   � H B to upgrade target project... if already upgraded, this will error    � R      �{ ��z
�{ .ascrerr ****      � **** � o      �y�y 0 upgerr upgErr�z   � I  )�x ��w
�x .sysodlogaskr        TEXT � b   % � � � o   !�v�v 0 fname fName � m  !$ � �  could not be upgraded.   �w   �  ��u � l **�t�s�t  �s  �u  ��  ��   {  ��r � l 00�q�p�q  �p  �r  �� 0 
subproject 
subProject u o   m n�o�o 0 projreflist projRefList s  �n  l 77�m�l�m  �l  �n    �k l     �j�i�j  �i  �k       �h�h   �g�f
�g .aevtoappnull  �   � ****�f 40 updatecurrentprojectrefs updateCurrentProjectRefs �e �d�c�b
�e .aevtoappnull  �   � ****�d  �c     �a�a 40 updatecurrentprojectrefs updateCurrentProjectRefs�b *j+   �` �_�^�]�` 40 updatecurrentprojectrefs updateCurrentProjectRefs�_  �^   
�\�[�Z�Y�X�W�V�U�T�S�\ 0 projreflist projRefList�[ 0 
activeproj 
activeProj�Z 0 	grouplist 	groupList�Y 0 
subproject 
subProject�X  0 subprojectpath subProjectPath�W "0 subprojectalias subProjectAlias�V 0 revdel revDel�U 0 fname fName�T 0 upgradename upgradeName�S 0 upgerr upgErr ! o�R�Q�P�O�N�M�L	�K W�J�I ��H�G�F�E ��D�C ��B�A�@�?�>�= ��<�; ��:
�R 
apdc
�Q 
rgru
�P 
list
�O 
cobj
�N .corecnte****       ****
�M 
grup
�L 
filr	  
�K 
filT
�J 
kocl
�I 
ppth
�H 
abph
�G 
psxf
�F 
ascr
�E 
txdl
�D 
TEXT
�C 
citm
�B 
pnam
�A 
as  
�@ .coreuppfnull���    alis
�? .sysodelanull��� ��� nmbr�> 40 updatecurrentprojectrefs updateCurrentProjectRefs
�= .coreclos****      � ****�< 0 upgerr upgErr�;  
�: .sysodlogaskr        TEXT�]9jvE�O� a*�,E�O��,�&E�O Ph��-j j ���k/�-%E�O���k/�-�[�,\Z�81%E�O��-j k  Y hO�[�\[Zl\Zi2E�[OY��UO ˠ[��l kh ��,� �� ��,E�UO*�/E�O� B_ a ,E�Oa kv_ a ,FO��,a &a i/E�O�_ a ,FO�a %E�O��a ,FUO ?� �a �l UOkj O*j+ Okj O� *�,j O��,Ea %��,FUW X  �a %j  OPY hOP[OY�COPascr  ��ޭ