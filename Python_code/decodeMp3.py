import os

for mp3File in os.listdir("audioData"):
    if mp3File.endswith(".mp3"):
        cmdId = mp3File.split(".")[0]
        mp3Path = "audioData" + os.sep + mp3File
        wavPath = "audioData" + os.sep + cmdId + ".wav"
        cmdStr = "D:"+ os.sep + "BinaryInstall"+os.sep+"ffmpeg"+os.sep+"bin"+os.sep+"ffmpeg -i " + mp3Path + " -f wav " + wavPath
        os.system(cmdStr)

