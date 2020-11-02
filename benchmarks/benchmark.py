import subprocess as sp
import time

#clientNames = ["ctx", "single-thread"]
clientNames = ["single-thread", "ctx"]
serverNames = clientNames
clientNumber = 100

#global client, server

with open("benmarkResults.csv", "w") as results:
    results.write("times;")
    for clientName in clientNames:
        results.write(clientName + ";")
    results.write("\n")

    for serverName in serverNames:
        print("start server: " + serverName)
        server = sp.Popen(["./benchmarks/" + serverName + "-server"], encoding="ascii", stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)
        results.write(serverName + ";")

        for clientName in clientNames:
            clients = []
            print("start client: " + clientName)
            for i in range(clientNumber):
                clients.append(sp.Popen(["./benchmarks/" + clientName + "-client"], encoding="ascii", stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE))
                #client = sp.Popen(["./benchmarks/" + clientName + "-client"], stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)
                #client.stdin.write(bytes('s', "ascii"))
            
            outs = []
            print("clients startet")

            for client in clients:
                out, err = client.communicate('s')
                outs.append(out)

            print("clients run")

            #for i in range(clientNumber):
                #clients[i].stdin.write('s')
                #clients[i].stdin.flush()


            time.sleep(5)

            resTime = 0

            for out in outs:
                while not out.isnumeric():
                    time.sleep(5)
                    print("sleep")
                    print(out)
                resTime += int(out)

            #for i in range(clientNumber):
                #time = clients[i].stdout.readline()[0:-1]
                #results.write(str(time) + str(";"))
                #tim = clients[i].stdout.readline()[0:-1]
                #print(tim)
                #resTime += int(tim)

            results.write(str(resTime) + str(";"))
        
        server.kill()
        results.write("\n")
