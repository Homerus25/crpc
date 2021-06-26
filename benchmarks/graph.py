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

    (x, counts) = bd.filter_functionID(1).get_data()[0].get_times_counted()
    (x2, counts2) = bd.filter_functionID(2).get_data()[0].get_times_counted()
    (x3, counts3) = bd.filter_functionID(3).get_data()[0].get_times_counted()
    (x4, counts4) = bd.filter_functionID(4).get_data()[0].get_times_counted()

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


# y -> median/mean
# x -> clientNumber
# impl by color
def plot_compare_implementations_scale_on_clients(bench_data: data.Collection):
    implementations = bench_data.list_implementations()
    print(implementations)
    colors = ["ob", "or"]
    colors_b = ["blue", "red"]
    for impl in implementations:
        bench = bench_data.filter_implementation(impl)
        #ya = [x.get_median_time() for x in bench.get_data()]
        ya = [x.get_mean_time() for x in bench.get_data()]
        xa = [x.clients for x in bench.get_data()]
        """
        #err = [(x.get_longest_time() - x.get_mean_time()) * 3 for x in bench.get_data()]
        err = []
        for dt in bench.get_data():
            mean = dt.get_mean_time()
            err.append([dt.get_longest_time() - mean, mean - dt.get_shortest_time()])
        """
        err_up = [(x.get_longest_time() - x.get_median_time()) for x in bench.get_data()]
        err_down = [(x.get_median_time() - x.get_shortest_time()) for x in bench.get_data()]

        color = colors.pop()
        #plt.plot(xa, ya, color, label=impl)
        plt.errorbar(xa, ya, yerr=[err_up, err_down])#, color=colors_b.pop())
        #plt.bar(xa, ya)
        #plt.legend(impl)


    #plt.yaxis.set_major_locator(MaxNLocator(integer=True))
    #plt.xaxis.set_major_locator(MaxNLocator(integer=True))

    plt.legend(implementations)
    plt.show()