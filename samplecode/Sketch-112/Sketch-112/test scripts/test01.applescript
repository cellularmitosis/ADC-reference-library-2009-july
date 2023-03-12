tell application "Sketch"
	with timeout of 60 * 60 seconds
		tell document 1
			
			get orientation of every rectangle
			
			set x to every rectangle
			repeat with y in x
				if orientation of y is landscape then
					set orientation of y to portrait
				else
					set orientation of y to landscape
				end if
			end repeat
			
			try
				set orientation of rectangle 1 to 5
			on error eMsg number eNum
				log {eNum, eMsg}
			end try
			
			get orientation of every rectangle
			
		end tell
	end timeout
end tell
