import subprocess as sp
import time

import graph
import data

def single_bench(clientName, clientNumber, functionID, iteration_count, benchsize):
    startarray = ["./benchmarks/" + clientName + "-client", str(clientNumber), str(functionID), str(iteration_count)]#, str(benchsize)]
    if functionID > 1:
        startarray.append(str(benchsize))
    print(startarray)
    client = sp.Popen(startarray, encoding="ascii", stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)

    print("clients started")
    time.sleep(1)

    startTime = time.time()

    print("clients run")
    client.wait()

    # todo: improve error handling here
    print(client.stderr.read())

    endTime = time.time()

    rtts_string = client.stdout.read().split()
    rtts = []
    for time_str in rtts_string:
        rtts.append(int(time_str))

    #results.write(str(resTime) + str(";"))
    #result_array.append(rtts)
    #result_array.append(endTime - startTime)
    print("restime: " + str(endTime - startTime))
    return data.DataSet(rtts, clientName, functionID, iteration_count, benchsize, clientNumber)

def bench(clientName, serverName, clientNumbers, iteration_count, benchsize):
    if serverName != "":
        print("start server: " + serverName)
        server = sp.Popen(["./benchmarks/" + serverName + "-server"], encoding="ascii", stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)

    results = data.Collection()

    for functionID in range(1, 5):
        # warmup
        single_bench(clientName, clientNumbers[0], functionID, iteration_count, benchsize)

        for clientNumber in clientNumbers:
            results.add(single_bench(clientName, clientNumber, functionID, iteration_count, benchsize))

    if serverName != "":
        server.kill()
    return results

def main():
    #clientNumber = [1, 2, 3, 5, 10, 20, 50, 75, 100]
    clientNumber = [1, 5, 20, 100]
    iteration_count = 100
    benchsize = 5000
    #benchsize = 262144

    result_ctx = bench("ctx-net", "ctx-net", clientNumber, iteration_count, benchsize)
    result_mqtt = bench("mqtt", "mqtt", clientNumber, iteration_count, benchsize)
    result_no_network = bench("no-network", "", clientNumber, iteration_count, benchsize)

    merged_bench = result_mqtt.merge(result_ctx).merge(result_no_network)
    #merged_bench.save("test.bench")

    #merged_bench = data.Collection()
    #merged_bench.load("test.bench")

    #graph.plot_single_implementation(result_no_network, "no-network")
    graph.plot_compare_implementations_scale_on_clients(merged_bench.filter_functionID(2))

if __name__ == "__main__":
    main()
