(*	Copyright © 2007 Apple Inc. All Rights Reserved.
	
	Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
			Apple Inc. ("Apple") in consideration of your agreement to the
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
			Neither the name, trademarks, service marks or logos of Apple Inc. 
			may be used to endorse or promote products derived from the Apple
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
*)

on run {input, parameters}
	
	set method_indicator to (|conversionMethod| of the parameters) as integer
	
	(* PROCESS IMAGE FILES *)
	set input to input as list
	repeat with i from 1 to the count of input
		set this_file to item i of input
		-- PROCESSING STATEMENTS GO HERE
		if method_indicator is 0 then
			pad_image(this_file)
		else
			crop_image(this_file)
		end if
	end repeat
	
	return input
end run

on pad_image(this_file)
	-- indicate the proportions for the pad area
	set H_proportion to 16
	set V_proportion to 9
	try
		tell application "Image Events"
			-- start the Image Events application
			launch
			-- open the image file
			set this_image to open this_file
			-- get dimensions of the image
			copy dimensions of this_image to {W, H}
			-- calculate pad dimensions
			if H_proportion is greater than V_proportion then
				set the new_W to (H * H_proportion) / V_proportion
				set pad_dimensions to {new_W, H}
			else
				set the new_H to (W * V_proportion) / H_proportion
				set pad_dimensions to {W, new_H}
			end if
			-- perform action
			pad this_image to dimensions pad_dimensions
			-- save the changes
			save this_image with icon
			-- purge the open image data
			close this_image
		end tell
	on error error_message
		display dialog error_message
	end try
end pad_image

on crop_image(this_file)
	try
		tell application "Image Events"
			-- start the Image Events application
			launch
			-- open the image file
			set this_image to open this_file
			-- get dimensions of the image
			copy dimensions of this_image to {W, H}
			-- determine the letterbox area
			set crop_W to W
			-- calcluate the 16:9 proportions
			set crop_H to (W * 9) / 16
			-- perform action
			crop this_image to dimensions {crop_W, crop_H}
			-- save the changes
			save this_image with icon
			-- purge the open image data
			close this_image
		end tell
	on error error_message
		display dialog error_message
	end try
end crop_image