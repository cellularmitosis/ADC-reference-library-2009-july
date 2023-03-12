-- GetiTunesInfo.applescript

set savedTextItemDelimiters to AppleScript's text item delimiters
set AppleScript's text item delimiters to {"**"}

tell application "iTunes"
	set playerState to player state
	if playerState is playing then
		set theStreamTitle to current stream title
		set theTrack to current track
		set theTrackTitle to name of theTrack
		set theTrackArtist to artist of theTrack
		set theTrackAlbum to album of theTrack
		set theInfo to {playerState, theStreamTitle, theTrackTitle, theTrackArtist, theTrackAlbum} as string
	else
		set the info to playerState
	end if
end tell