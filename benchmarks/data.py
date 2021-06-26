import numpy as np
import pickle

class Collection:
    def __init__(self):
        self.data = []

    def add(self, dataset):
        self.data.append(dataset)

    def filter_implementation(self, impl):
        newCollection = Collection()
        newCollection.data.extend([x for x in self.data if x.implementation == impl])
        return newCollection

    def filter_functionID(self, fID):
        newCollection = Collection()
        newCollection.data.extend([x for x in self.data if x.functionID == fID])
        return newCollection

    def get_data(self):
        return self.data

    def list_implementations(self):
        return list(set([x.implementation for x in self.data]))

    def merge(self, other):
        newCollection = Collection()
        newCollection.data.extend(self.data)
        newCollection.data.extend(other.data)
        return newCollection

    def save(self, filename):
        with open(filename, 'wb') as file:
            pickle.dump(self.data, file, pickle.HIGHEST_PROTOCOL)

    def load(self, filename):
        with open(filename, 'rb') as file:
            self.data = pickle.load(file)

class DataSet:
    rtts = []
    implementation = ""
    functionID = -1
    iterations = -1
    datasize = -1

    def __init__(self, times, implementation, fID, iterations, datasize, clients):
        self.rtts = times
        self.implementation = implementation
        self.functionID = fID
        self.iterations = iterations
        self.datasize = datasize
        self.clients = clients

    def get_times_counted(self):
        buckets = {}
        for time in self.rtts:
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

    def get_median_time(self):
        return np.median(self.rtts)

    def get_mean_time(self):
        return np.mean(self.rtts)

    def get_longest_time(self):
        return np.max(self.rtts)

    def get_shortest_time(self):
        return np.min(self.rtts)