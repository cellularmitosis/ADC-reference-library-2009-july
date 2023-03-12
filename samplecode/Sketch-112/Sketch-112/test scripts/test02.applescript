tell application "Sketch"
	with timeout of 60 * 60 seconds
		tell document 1
			
			get orientation of every rectangle
			
			set x to every rectangle
			repeat with y in x
				rotate y by 90
			end repeat
			
			try
				rotate rectangle 1 by 80
			on error eMsg number eNum
				log {eNum, eMsg}
			end try
			
			get orientation of every rectangle
			
		end tell
	end timeout
end tell
