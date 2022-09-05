import numpy as np
from matplotlib import pyplot as plt
from matplotlib.ticker import MaxNLocator
import data

def count_times(times):
    buckets = {}
    for time in times:
        if time in buckets:
            buckets[time] += 1
        else:
            buckets[time] = 1

    x = []
    counts = []

    for k, v in sorted(buckets.items()):
        #print(k,v)
        x.append(int(k))
        counts.append(int(v))

    return (x, counts)


def plot_single_implementation(bench_data: data.Collection, implName):

    bd = bench_data.filter_implementation(implName)

    (x, counts) = bd.filter_functionID(1).get_data()[1].get_times_counted()
    (x2, counts2) = bd.filter_functionID(2).get_data()[1].get_times_counted()
    (x3, counts3) = bd.filter_functionID(3).get_data()[1].get_times_counted()
    (x4, counts4) = bd.filter_functionID(4).get_data()[1].get_times_counted()

    #(x, counts) = count_times(bench_data[0])
    #(x2, counts2) = count_times(bench_data[2])
    #(x3, counts3) = count_times(bench_data[4])
    #(x4, counts4) = count_times(bench_data[6])
    #print(counted)

    #plt.title("test titel")

    figure, plots = plt.subplots(2, 2)

    #figure.title("super plots")

    ylabel = "appearance"
    xlabel = "round-trip-time (ms)"

    def config_plot(plot):
        plot.set_ylabel(ylabel)
        plot.set_xlabel(xlabel)
        plot.yaxis.set_major_locator(MaxNLocator(integer=True))
        plot.xaxis.set_major_locator(MaxNLocator(integer=True))


    plots[0, 0].title.set_text("say hello")
    plots[0, 0].plot(x, counts, "ob")
    config_plot(plots[0, 0])

    plots[1, 0].title.set_text("calculate average")
    plots[1, 0].plot(x2, counts2, "ob")
    config_plot(plots[1, 0])

    plots[0, 1].title.set_text("get random numbers")
    plots[0, 1].plot(x3, counts3, "ob")
    config_plot(plots[0, 1])

    plots[1, 1].title.set_text("send back")
    plots[1, 1].plot(x4, counts4, "ob")
    config_plot(plots[1, 1])

    #fig, ax = plt.subplots()
    #n, bins, patches = ax.hist(x, counts, density=True)
    #n, bins, patches = ax.hist(x2, len(counts2), density=True)

    plt.subplots_adjust(wspace=0.3, hspace=0.3)

    plt.show()


# y -> mean
# x -> clientNumber
# impl by color
def plot_compare_implementations_scale_on_clients(bench_data: data.Collection):
    implementations = bench_data.list_implementations()
    print(implementations)
    colors = ["--ob", "--or", "--og"]
    for impl in implementations:
        bench = bench_data.filter_implementation(impl)

        ya = [x.get_mean_time() for x in bench.get_data()]
        xa = [x.clients for x in bench.get_data()]

        err_up = [(x.get_longest_time() - x.get_mean_time()) for x in bench.get_data()]
        err_down = [(x.get_mean_time() - x.get_shortest_time()) for x in bench.get_data()]

        color = colors.pop()
        plt.errorbar(xa, ya, yerr=[err_down, err_up], fmt=color, capsize=5.0)

    plt.legend(implementations)
    plt.show()

# y -> mean
# x -> clientNumber
# impl by color
def plot_compare_implementations_scale_on_clients_all_functions(bench_data: data.Collection):
    implementations = bench_data.list_implementations()
    print(implementations)

    figure, plots = plt.subplots(2, 2)

    for fID in range(1,5):
        if fID == 1:
            plot = plots[0, 0]
            fName = "say hello"
        elif fID == 2:
            plot = plots[0, 1]
            fName = "calculate average"
        elif fID == 3:
            plot = plots[1, 0]
            fName = "get random numbers"
        elif fID == 4:
            plot = plots[1, 1]
            fName = "send back"

        colors = ["--ob", "--or", "--og"]

        #for impl in implementations:
        for impl in ["mqtt", "no-network", "ctx-net"]:
            bench = bench_data.filter_functionID(fID)
            bench = bench.filter_implementation(impl)

            ya = [x.get_mean_time() for x in bench.get_data()]
            xa = [x.clients for x in bench.get_data()]

            err_up = [(x.get_longest_time() - x.get_mean_time()) for x in bench.get_data()]
            err_down = [(x.get_mean_time() - x.get_shortest_time()) for x in bench.get_data()]
            print(err_down)

            color = colors.pop()
            plot.errorbar(xa, ya, yerr=[err_down, err_up], fmt=color, capsize=5.0)
            plot.legend(implementations)
            plot.title.set_text(fName)
            plot.set_ylabel("round-trip-time (ms)")
            plot.set_xlabel("number clients")
            plot.yaxis.set_major_locator(MaxNLocator(integer=True))
            plot.xaxis.set_major_locator(MaxNLocator(integer=True))

    figure.suptitle("6 serverthreads")
    plt.subplots_adjust(wspace=0.3, hspace=0.3)
    plt.show()