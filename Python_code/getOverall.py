import os
import json


listDataOld = []
listDatatmp = []
listDataNew = []

fileCount = 0                           #统计使用第几个list,new 还是old
for opusFile in os.listdir():    
#    print (opusFile)
                             
    if opusFile.startswith("opus_win32_"):
        print (opusFile)
        fileCount = fileCount + 1
        flagErr = 0                      
        fopu = open(opusFile)
        lineData = fopu.readline()
        if "[[[" in lineData and "]]]" in lineData:
            count = 0
                
            for chr in lineData:
                count = count +1     
                if chr == "[":             
                    chrFlag = lineData[count-1:count+2]
                    if chrFlag == "[[[":
                        break
                    else:
                        flagErr = 1 
                        break
            tmpData = lineData[count+2:]
            lastData = tmpData.replace("]]]","")
            json_str = json.loads(lastData)
#           print (json_str)
                
#           print (json_str.keys())

            try:
                listDatatmp = []
            
                recordId = json_str["recordId"]
                refText = json_str["refText"]
            
                overall = json_str["result"]["overall"]
                listDatatmp.append(recordId)
                listDatatmp.append(refText)
                listDatatmp.append(overall)
                if fileCount== 1:
                    listDataOld.append(listDatatmp)
                elif fileCount==2:
                    listDataNew.append(listDatatmp)
                else:
                    pass
                
            except KeyError:
                pass             
       
        while lineData:
            lineData = fopu.readline()
            if flagErr == 1:
                flagErr = 0
                continue   
            if "[[[" in lineData and "]]]" in lineData:
                count = 0
                
                for chr in lineData:
                    count = count +1     
                    if chr == "[":             
                        chrFlag = lineData[count-1:count+2]
                        if chrFlag == "[[[":
                            break
                        else:
                            flagErr = 1 
                            break
                tmpData = lineData[count+2:]
                lastData = tmpData.replace("]]]","")
                json_str = json.loads(lastData)
#                print (json_str)

                try:
                    listDatatmp = []  
                    recordId = json_str["recordId"]
                    refText = json_str["refText"]
                    overall = json_str["result"]["overall"]

                    listDatatmp.append(recordId)
                    listDatatmp.append(refText)
                    listDatatmp.append(overall)

        
                    if fileCount== 1:
                        listDataOld.append(listDatatmp)
                    elif fileCount==2:
                        listDataNew.append(listDatatmp)
                    else:
                        pass
                    
                except KeyError:
                    pass
                    

fl = open("outWin32.txt","w+")

for oldTmp in listDataOld:
    for newTmp in listDataNew:
        if oldTmp[1] == newTmp[1]:
            savCount = oldTmp[0] + "\t" + str(oldTmp[2]) + "\t" + newTmp[0] + "\t" + str(newTmp[2]) + "\t" + oldTmp[1] + "\n"
            fl.write(savCount) 
            break
            
            
fl.close()
#print (listDataOld)
#print (listDataNew)
                            
                        
 
                        
            
