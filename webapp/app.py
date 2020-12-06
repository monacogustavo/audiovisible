from flask import Flask, render_template, request, redirect, url_for, abort, \
    send_from_directory
import imghdr
import os
from pydub import AudioSegment
from pydub.playback import play
from os import path
from pydub import AudioSegment
import wave
import sys
import pyaudio
import pitch
import numpy as np
import matplotlib.pyplot as plt
import struct
import pygame
from wavtorgb import *
from math import *
from scipy import signal

CHUNK = 1024
INTERVAL = 0.32

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/', methods=['POST'])
def upload_file():
    for uploaded_file in request.files.getlist('file'):
        if uploaded_file.filename != '':
            uploaded_file.save(uploaded_file.filename)
        # files                                                                         
        src = uploaded_file.filename
        if uploaded_file.filename.endswith('.mp3'):
            uploaded_file.filename2 = uploaded_file.filename[:-4]
            dst = uploaded_file.filename2 + ".wav"
        else:
            dst = uploaded_file.filename

        # convert mp3 to wav                                                            
        sound = AudioSegment.from_mp3(src)
        sound.export(dst, format="wav")
        #os.remove(uploaded_file.filename)

        wf = wave.open(dst, 'rb')
        p = pyaudio.PyAudio()

        stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                        channels=wf.getnchannels(),
                        rate=wf.getframerate(),
                        output=True)

        data = wf.readframes(CHUNK)
 
        pygame.init()
        screen = pygame.display.set_mode((1000, 800))
        
        chunk = 2048
        swidth = wf.getsampwidth()
        RATE = wf.getframerate()
        # use a Blackman window
        window = np.blackman(CHUNK)
        # open the stream
        p = pyaudio.PyAudio()
        background = pygame.Surface(screen.get_size())
        background = background.convert()

        global thefreq
        thefreq = 1.0
        stream = p.open(format =
                        p.get_format_from_width(wf.getsampwidth()),
                        channels = wf.getnchannels(),
                        rate = RATE,
                        output = True)

        # read the incoming data
        data = wf.readframes(CHUNK)
        # play stream and find the frequency of each chunk
        while len(data) == CHUNK*swidth:
        # write data out to the audio stream
            stream.write(data)
        # unpack the data and times by the hamming window
            indata = np.array(wave.struct.unpack("%dh"%(len(data)/swidth),\
                                                data))*window
        # Take the fft and square each value
            fftData=abs(np.fft.rfft(indata))**2
        # find the maximum
            which = fftData[1:].argmax() + 1
        # use quadratic interpolation around the max
            if which != len(fftData)-1:
                y0,y1,y2 = np.log(fftData[which-1:which+2:])
                x1 = (y2 - y0) * .5 / (2 * y1 - y2 - y0)
                # find the frequency and output it
                thefreq = (which+x1)*RATE/CHUNK
                thefreq = which*RATE/CHUNK
                #print "the previous freq is "+str(thefreq)
            while thefreq < 350 and thefreq > 15:
                #global thefreq
                thefreq = thefreq*2 
            while thefreq > 700:
                #global thefreq
                thefreq = thefreq/2 
            c = 3*10**8
            THz = thefreq*2**40
            pre = float(c)/float(THz)
            nm = int(pre*10**(-floor(log10(pre)))*100)	
            rgb = wavelen2rgb(nm, MaxIntensity=255)
            #Fills the background with the appropriate colot, does this so fast, it creates a "fading effect" in between colors
            background.fill((rgb[0],rgb[1],rgb[2]))
            #"blits" (renders) the color to the background
            screen.blit(background, (0, 0))
            #and finally displays the background
            pygame.display.flip()
    
            
    #     # read some more data
            data = wf.readframes(CHUNK)
        if data:
            stream.write(data)

        stream.stop_stream()
        stream.close()

        p.terminate()
    return redirect(url_for('index'))



if __name__ == '__main__':
    app.run(host='0.0.0.0',port=4462,debug=True) 

# https://blog.miguelgrinberg.com/post/handling-file-uploads-with-flask