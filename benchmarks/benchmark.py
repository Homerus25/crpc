import subprocess as sp
import time

import graph
import data

def single_bench(clientName, clientNumber, functionID, iteration_count, benchsize, serverThreads):
    startarray = ["./benchmarks/" + clientName + "-client", str(clientNumber), str(functionID), str(iteration_count)]#, str(benchsize)]
    if functionID > 1:
        startarray.append(str(benchsize))
    if clientName == "no-network":
        startarray.append(str(serverThreads))
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
    return data.DataSet(rtts, clientName, functionID, iteration_count, benchsize, clientNumber, serverThreads)

def bench(clientName, serverName, serverThreads, clientNumbers, iteration_count, benchsize):
    results = data.Collection()

    for st in serverThreads:
        if serverName != "":
            print("start server: " + serverName)
            server = sp.Popen(["./benchmarks/" + serverName + "-server", str(st)], encoding="ascii", stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)

        for functionID in range(1, 5):
            # warmup
            single_bench(clientName, clientNumbers[0], functionID, iteration_count, benchsize, st)

            for clientNumber in clientNumbers:
                ben = single_bench(clientName, clientNumber, functionID, iteration_count, benchsize, st)
                if len(ben.rtts) == 0:
                    print("fail!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                    exit()
                results.add(ben)

        if serverName != "":
            server.kill()
    return results

def do_bench():
    clientNumber = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
    iteration_count = 1000
    benchsize = 1024
    serverThreads = [6]

    #result_ctx = bench("ctx-net", "ctx-net", serverThreads, clientNumber, iteration_count, benchsize)
    #result_ctx.save("benchdata/ctx-6.bench")

    #result_no_network = bench("no-network", "", serverThreads, clientNumber, iteration_count, benchsize)
    #result_no_network.save("benchdata/no-network6.bench")

    #result_mqtt = bench("mqtt", "mqtt", serverThreads, clientNumber, iteration_count, benchsize)
    #result_mqtt.save("benchdata/mqtt-6.bench")


    result_no_network = data.Collection()
    result_no_network.load("benchdata/no-network6.bench")

    result_ctx = data.Collection()
    result_ctx.load("benchdata/ctx-6.bench")

    result_mqtt = data.Collection()
    result_mqtt.load("benchdata/mqtt-6.bench")

    merged_bench = result_mqtt.merge(result_ctx).merge(result_no_network)
    graph.plot_compare_implementations_scale_on_clients_all_functions(merged_bench.filter_serverThreads(6))

def main():
    do_bench()
    return []
    #clientNumber = [1, 2, 3, 5, 10, 20, 50, 75, 100]
    clientNumber = [1, 5, 20, 100]
    iteration_count = 100
    benchsize = 5000
    #benchsize = 262144

    #result_ctx = bench("ctx-net", "ctx-net", clientNumber, iteration_count, benchsize)
    #result_mqtt = bench("mqtt", "mqtt", clientNumber, iteration_count, benchsize)
    #result_no_network = bench("no-network", "", clientNumber, iteration_count, benchsize)

    #result_ctx.save("ctx.bench")
    result_ctx = data.Collection()
    result_ctx.load("ctx.bench")

    #result_mqtt.save("mqtt.bench")
    result_mqtt = data.Collection()
    result_mqtt.load("mqtt.bench")

    #result_no_network.save("no-network3.bench")
    result_no_network = data.Collection()
    result_no_network.load("no-network3.bench")

    merged_bench = result_mqtt.merge(result_ctx).merge(result_no_network)
    #merged_bench.save("merged.bench")

    #merged_bench = data.Collection()
    #merged_bench.load("merged.bench")

    #graph.plot_single_implementation(result_ctx, "ctx-net")
    #graph.plot_single_implementation(result_mqtt, "mqtt")
    #graph.plot_single_implementation(result_no_network, "no-network")
    #graph.plot_compare_implementations_scale_on_clients(merged_bench.filter_functionID(4))
    graph.plot_compare_implementations_scale_on_clients_all_functions(merged_bench)

if __name__ == "__main__":
    main()
