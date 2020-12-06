import math
import wave
import struct
sample_rate = 44100.0
def generate_tone(file_name,
        freq=440.0, 
        duration_milliseconds=5000, 
        volume=1.0):
    
    audio = []
    num_samples = duration_milliseconds * (sample_rate / 1000.0)
    for x in range(int(num_samples)):
        audio.append(volume * math.sin(2 * math.pi * freq * ( x / sample_rate )))
    # Open up a wav file
    wav_file=wave.open(file_name,"w")
    # wav params
    nchannels = 1
    sampwidth = 2
    # 44100 is the industry standard sample rate - CD quality.  If you need to
    # save on file size you can adjust it downwards. The stanard for low quality
    # is 8000 or 8kHz.
    nframes = len(audio)
    comptype = "NONE"
    compname = "not compressed"
    wav_file.setparams((nchannels, sampwidth, sample_rate, nframes, comptype, compname))
    for sample in audio:
        wav_file.writeframes(struct.pack('h', int( sample * 32767.0 )))
    wav_file.close()
    return
generate_tone("/content/tones/f550.wav",550,5000,1.0)
