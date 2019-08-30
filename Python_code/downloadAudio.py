import os
import pdb
import urllib.request
import time
 
 
def get_url(fileName):
    line1 = []
    with open(fileName) as f:
        lines = (line for line in f)
        for line in lines:
            #pdb.set_trace()
            line = line.split("|")[1].strip()
            line1.append(line)
 
        return line1
 
def download_url(url,writeFileName):
    try:
        downloadFile = urllib.request.urlopen(url)
        with open(writeFileName,'wb') as output:
            output.write(downloadFile.read())
    except urllib.error.HTTPError:
        print(url + "    error")



with open("recodId.txt","r+",encoding='UTF-8') as fp:
    dataSav=""
    flag = 0
    count=0
    flagEnd = 0
   
    for lineData in fp:
        if "}" in lineData:
            flagEnd = 1
        if flag==1 and flagEnd == 0:
            if "params.request.refText" in lineData:
                count=count+1
                flag = 0

                txtF = dataSav.split(":",1)[1].replace('\n',"").replace(",","").replace('\"',"") 

                txtPath = "audioData" + os.sep + txtF + ".txt"

                audioPath = "audioData" + os.sep + txtF + ".mp3"

                lineDat = lineData.split(":",1)[1].replace('\n',"").replace(",","").replace('\"',"")

                url = "http://records.17kouyu.com/" + txtF + ".mp3"


                download_url(url,audioPath)
                
                
                with open(txtPath,"w") as fw:
                    fw.write(lineDat)
                fw.close()
               
                

 #  print (dataSav.split(":",1)[1] + "_" + lineData.split(":",1)[1])
                dataSav = ""
        if "recordId" in lineData:
            dataSav = lineData
#            print(dataSav.split(":",1)[1])            
            flag=1
            flagEnd = 0


    print (count)
            

              
              
