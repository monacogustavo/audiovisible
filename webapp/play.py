from pydub import AudioSegment
from pydub.playback import play

song = AudioSegment.from_mp3("Seven_Lions_-_Only_Now_feat._Tyler_Graves_Quannum_Logic__KALCYFR_remix.wav")
play(song)