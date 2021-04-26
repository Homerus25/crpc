import subprocess as sp
import time

def bench(clientNames, serverNames, clientNumber, iteration_count, resultName):
    with open(resultName, "w") as results:
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
                    clients.append(sp.Popen(["./benchmarks/" + clientName + "-client", "1", str(iteration_count)], encoding="ascii", stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE))
                    #client = sp.Popen(["./benchmarks/" + clientName + "-client"], stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)
                    #client.stdin.write(bytes('s', "ascii"))
                
                outs = []
                errors = []
                print("clients startet")
                time.sleep(1)

                startTime = time.time()

                for client in clients:
                    out, err = client.communicate('s')
                    outs.append(out)
                    errors.append(err)

                print("clients run")

                time.sleep(5)
                resTime = 0


                for client in clients:
                    client.wait()

                print("waited for all")

                for error in errors:
                    print(error)

                endTime = time.time()

                for idx in range(len(outs)):
                    out = outs[idx]
                    if out.isnumeric():
                        print(out)
                        resTime += int(out)
                    else:
                        print("failed")
                        print(out)
                        print(errors[idx])
                        if clients[idx].returncode == None:
                            print("not terminated")
                        else:
                            print(clients[idx].returncode)

                results.write(str(resTime) + str(";"))
                print("restime: " + str(endTime - startTime))
                print("restime: " + str(resTime))
            
            server.kill()
            results.write("\n")


clientNumber = 20
iteration_count = 100

serverNames = ["ctx-net"]
clientNames = ["ctx-net"]

bench(clientNames, serverNames, clientNumber, iteration_count, "ctx.csv")
bench(["mqtt"], ["mqtt"], clientNumber, iteration_count, "mqtt.csv")