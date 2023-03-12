#!/usr/bin/perl -w


# Description:
# 
# TextNameTool
# a tool for generating rasterized images of text using nice looking fonts.
# this file contains a number of utilities for the graphical titles
# 
# Copyright: 	(c) Copyright 2003 Apple Computer, Inc. All rights reserved.
# 
# Disclaimer:	IMPORTANT:  This Apple software is supplied to you by
# Apple Computer, Inc.  ("Apple") in consideration of your agreement to the
# following terms, and your use, installation, modification or
# redistribution of this Apple software constitutes acceptance of these
# terms.  If you do not agree with these terms, please do not use, install,
# modify or redistribute this Apple software.
# 
# In consideration of your agreement to abide by the following terms, and
# subject to these terms, Apple grants you a personal, non-exclusive
# license, under Apple's copyrights in this original Apple software (the
# "Apple Software"), to use, reproduce, modify and redistribute the
# Apple Software, with or without modifications, in source and/or binary
# forms; provided that if you redistribute the Apple Software in its
# entirety and without modifications, you must retain this notice and
# the following text and disclaimers in all such redistributions of
# the Apple Software.  Neither the name, trademarks, service marks or
# logos of Apple Computer, Inc. may be used to endorse or promote
# products derived from the Apple Software without specific prior
# written permission from Apple.  Except as expressly stated in this notice,
# no other rights or licenses, express or implied, are granted by Apple
# herein, including but not limited to any patent rights that may be
# infringed by your derivative works or by other works in which the Apple
# Software may be incorporated.
# 
# The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
# COMBINATION WITH YOUR PRODUCTS.
# 
# IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
# DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
# OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE,
# EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



use strict;
use FileHandle;


########## I M P O R T A N T ##########
# set this variable to the path to the
# text name tool, where ever you happen to put it
# on your system.

my $tnt_path_to_tool = "build/TextNameTool";



my %tnt_ascii_to_xml_table = (
	">" => "gt",
	"<" => "lt",
	"&" => "amp",
	"\"" => "quot",
	"\'" => "apos"
);

my %tnt_xml_to_ascii_table = reverse %tnt_ascii_to_xml_table;


sub tnt_xml_entity_char ($) {
	my ($entity) = @_;
	my $thechar = $tnt_xml_to_ascii_table{$entity};
	unless ($thechar) {
		die "unrecognized xml entity $entity";
	}
	return $thechar;
}

sub tnt_decode_xml_string ($) {
	my ($encodedstring) = @_;
	$encodedstring =~ s/\&([a-zA-Z]+|\#[0-9]+)\;/tnt_xml_entity_char($1)/ge;
	return $encodedstring;
}



sub tnt_xml_char_entity ($) {
	my ($thechar) = @_;
	my $entity = $tnt_ascii_to_xml_table{$thechar};
	if ($entity) {
		return "\&$entity;";
	} else {
		return $thechar;
	}
}

sub tnt_encode_xml_string ($) {
	my ($encodedstring) = @_;
	$encodedstring =~ s/(.)/tnt_xml_char_entity($1)/ge;
	$encodedstring =~ s/\t/    /g;
	return $encodedstring;
}


sub tnt_get_xml_section ($$) {
	my ($textdata, $tagname) = @_;
	$tagname = quotemeta($tagname);
	if ($textdata =~ m/<$tagname>(.*?)<\/$tagname>/is) {
		my $data = $1;
		return $data ;
	} else {
		return "";
	}
}

sub tnt_get_all_xml_sections ($$) {
	my ($textdata, $tagname) = @_;
	my @section_list;
	$tagname = quotemeta($tagname);
	@section_list = ($textdata =~ m/<$tagname>(.*?)<\/$tagname>/gis);
	return @section_list;
}




sub tnt_do_unix_command($) {
	my ($unix_command) = @_;
	my ($command_data, $inbuf);
	open(COMMANDFILEHANDLE, '-|', $unix_command) or die "error piping from '$unix_command': $!";
	$command_data = "";
	while (read(COMMANDFILEHANDLE, $inbuf, (1024*16)) != 0) {
		$command_data .= $inbuf;
	}
	close(COMMANDFILEHANDLE);
	return $command_data;
}




# returns the help text
sub tnt_help_text () {
		# query the tool for the dimensions
	my $xml = tnt_do_unix_command("$tnt_path_to_tool -h");
	if (tnt_get_xml_section($xml, 'status') eq 'ok') {
		return tnt_decode_xml_string(tnt_get_xml_section($xml, 'help'));
	} else {
		die(tnt_decode_xml_string(tnt_get_xml_section($xml, 'message')));
	}
}


# returns a list of the names of all of the fonts available
sub tnt_list_fonts () {
		# query the tool for the dimensions
	my $xml = tnt_do_unix_command("$tnt_path_to_tool -L");
	if (tnt_get_xml_section($xml, 'status') eq 'ok') {
		my $fontsxml = tnt_get_xml_section($xml, 'fonts');
		my @fontlist = tnt_get_all_xml_sections ($fontsxml, 'font');
		my ($nth, @fontnames);
		foreach $nth (@fontlist) {
			@fontnames = (@fontnames, tnt_decode_xml_string(tnt_get_xml_section($nth, 'name')));
		}
		return @fontnames;
	} else {
		die(tnt_decode_xml_string(tnt_get_xml_section($xml, 'message')));
	}
}


# returns the width and height of a graphics file
sub tnt_graphics_file_bounds ($) {
	my ($filename) = @_;
		# encode the file name
	my $qfilename = quotemeta($filename);
		# query the tool for the dimensions
	my $xml = tnt_do_unix_command("$tnt_path_to_tool -i $qfilename");
	if (tnt_get_xml_section($xml, 'status') eq 'ok') {
		my $imagexml = tnt_get_xml_section($xml, 'investigated-image');
		my $width = tnt_get_xml_section($imagexml, 'width');
		my $height = tnt_get_xml_section($imagexml, 'height');
		return ($width, $height);
	} else {
		die(tnt_decode_xml_string(tnt_get_xml_section($xml, 'message')));
	}
}


# creates an image file, returning the width and height as a list
# options are provided as a hash reference in the third parameter
# comments below explain what they are
sub tnt_make_image ($$$) {
	my ($filename, $titlestring, $options) = @_;
		# encode the file nameand message
	my $qfilename = quotemeta($filename);
	my $qtitlestring = quotemeta($titlestring);
	my $commandline = "$tnt_path_to_tool -o $qfilename -m $qtitlestring";
		# add other options
	if ($$options{FONT}) { $commandline .= (" -f " . quotemeta($$options{FONT})); } # use one of the names returned by tnt_list_fonts
	if ($$options{SIZE}) { $commandline .= (" -s " . quotemeta($$options{SIZE})); }
	if ($$options{WIDTH}) { $commandline .= (" -w " . quotemeta($$options{WIDTH})); }
	if ($$options{BACKGROUND}) { $commandline .= (" -r " . quotemeta($$options{BACKGROUND})); } # path to file
	if ($$options{MEASURE}) { $commandline .= " -t"; } # don't make the file, just measure it
	if ($$options{LINESPACING}) { $commandline .= (" -p " . quotemeta($$options{LINESPACING})); }
	if ($$options{QUALITY}) { $commandline .= (" -q " . quotemeta($$options{QUALITY})); } # high, min, low, normal, high, max
	if ($$options{BORDER}) { $commandline .= (" -b " . quotemeta($$options{BORDER})); } # see help
	if ($$options{BOLD}) { $commandline .= " -a b"; }
	if ($$options{ITALIC}) { $commandline .= " -a i"; }
	if ($$options{UNDERLINE}) { $commandline .= " -a u"; }
	if ($$options{CONDENSE}) { $commandline .= " -a c"; }
	if ($$options{EXTEND}) { $commandline .= " -a e"; }
	if ($$options{ATTRIBUTES}) { $commandline .= (" -a " . quotemeta($$options{ATTRIBUTES})); } # font attributes
	if ($$options{FORMAT}) { $commandline .= (" -k " . quotemeta($$options{FORMAT})); } # jpg or png
	if ($$options{QUICKDRAW}) { $commandline .= " -d"; } # 4 bit quickdraw
	if ($$options{COLOR}) { $commandline .= (" -c " . quotemeta($$options{COLOR})); } # see help
	if ($$options{LINEBREAKS}) { $commandline .= " -n "; }
	if ($$options{HTML}) { $commandline .= " -e "; } # message is encoded as html text &amp;, &gt; and so forth...
		# query the tool for the dimensions
	my $xml = tnt_do_unix_command($commandline);
	if (tnt_get_xml_section($xml, 'status') eq 'ok') {
		my $imagexml = tnt_get_xml_section($xml, 'generated-image');
		my $width = tnt_get_xml_section($imagexml, 'width');
		my $height = tnt_get_xml_section($imagexml, 'height');
		return ($width, $height);
	} else {
		die(tnt_decode_xml_string(tnt_get_xml_section($xml, 'message')));
	}
}



# example, generating images for all fonts:
# 
# eval { # use eval to trap errors
# 
#     my @fonts = tnt_list_fonts();
#     my $nth;
#     foreach $nth (@fonts) {
#         tnt_make_image(
#             "/Users/johnmont/document_index_tools/textnametool/fonts/$nth.jpg",
#             "A quick brown fox jumps over the lazy dog.",
#             {FONT => $nth, SIZE => 46, BORDER => 'auto'});
#     }
# };
# if ($@) { # display any errors we trapped....
#     my $errtext = $@;
#     print "error: $errtext\n";
# }
# 



1;

