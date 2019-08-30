import os

fw = open("array.txt","w")
fw.write("{\n")

count = 0
for wavFile in os.listdir("audioData"):
    if wavFile.endswith(".wav"):
        recId = wavFile.split(".")[0]
        txtName = "audioData" + os.sep + recId + ".txt"
        audioPath = "\"" + "audioData" + "/" + wavFile + "\""
        with open(txtName,"r") as fr:
            txtData=fr.read()
            txtStr = "\"" + txtData + "\"" 
            
            fw.write("{")
            fw.write(audioPath)
            fw.write(",")
            fw.write(txtStr)
            fw.write("}")
            fw.write(",\n")
            count = count + 1

fw.write("}\n")
fw.close()
print (count)
                    
