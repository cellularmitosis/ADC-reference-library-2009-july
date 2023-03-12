tell application "Sketch"
	with timeout of 60 * 60 seconds
		tell document 1
			
			align every graphic to vertical centers
			
			delay 3
			
			set x to every graphic
			align x to horizontal centers
			
		end tell
	end timeout
end tell
