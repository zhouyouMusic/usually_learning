import os

for i, j ,k in os.walk("openssl-1.0.2s"):
    for file in k:
        if file=="Makefile":
            dirph=os.getcwd()
            absph=dirph+os.sep+i+os.sep+file
            print(absph)

            fs=open(absph,"r")
            newLine=""
            data=""
            for line in fs.readlines():
                if "CFLAGS=" in line:

                    newLine=line.replace("\n","") + " -fembed-bitcode"+"\n"

                    data = data + newLine
                    print(data)
                else:
                    data = data + line
            fs.seek(0,0)
            fs.close()
            fs=open(absph,"w")
            fs.write(data)
            fs.close()
