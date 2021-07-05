"""PyAudio Example: Play a WAVE file."""

import pyaudio
import wave
import sys
import serial

#ser = serial.Serial(port='COM2', timeout=0, write_timeout=0)
ser = serial.Serial(port='COM2')

CHUNK = 1024

sync_period = 5

if len(sys.argv) < 2:
    print("Plays a wave file.\n\nUsage: %s filename.wav" % sys.argv[0])
    sys.exit(-1)

wf = wave.open(sys.argv[1], 'rb')

p = pyaudio.PyAudio()

stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                channels=wf.getnchannels(),
                rate=wf.getframerate(),
                output=True)

frames_read = 0;
if len(sys.argv) == 3:
    if float(sys.argv[2]) > 0:
        # throw away a bunch of frames
        frames_read = int(float(sys.argv[2]) * wf.getframerate())
        wf.readframes(frames_read)

data = wf.readframes(CHUNK);

start_time = stream.get_time()
next_sync_time = 0;
while len(data) > 0:
    stream.write(data)
    data = wf.readframes(CHUNK)
    stream_time = stream.get_time() - start_time;
    if stream_time > next_sync_time:
        next_sync_time += sync_period
        # the time into the song is the number of frames read divided by the number
        # of frames per second
        song_time = frames_read / wf.getframerate();
        song_time_ms = int(song_time * 1000)
        song_time_string = ('t' + str(song_time_ms) + '\r')
        ser.write(song_time_string.encode('utf-8'))
        print(song_time_string)
    frames_read += CHUNK

ser.close()
stream.stop_stream()
stream.close()

p.terminate()
