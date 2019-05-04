# ONLY FOR TESTING, REAL CLIENTS IN JAVA

import grpc

import exchange_rates_pb2
import exchange_rates_pb2_grpc


def getAllRates(stub):
    print('getting rates')
    c = exchange_rates_pb2.Currencies()

    c.names.append(exchange_rates_pb2.Currencies.EUR)
    print(c)
    rates = stub.GetRates(c)
    #print(type(c))
    print(rates)
    for rate in rates:
        print(rate)


def getTest(stub):

    print('getting test')

    res = stub.GetTest(exchange_rates_pb2.NoParams())

    print(res)


def getEurUsd(stub):
    print("-------------- getEurUsd --------------")
    c = exchange_rates_pb2.Currencies()
    c.names.append(exchange_rates_pb2.Currencies.EUR)
    c.names.append(exchange_rates_pb2.Currencies.USD)

    rates = stub.GetRates(c)

    for rate in rates:
        print(rate)


def getUsdGbp(stub):
    print("-------------- getUsdGbp --------------")
    c = exchange_rates_pb2.Currencies()
    c.names.append(exchange_rates_pb2.Currencies.GBP)
    c.names.append(exchange_rates_pb2.Currencies.USD)

    rates = stub.GetRates(c)

    for rate in rates:
        print(rate)


def run():
    with grpc.insecure_channel('localhost:50066') as channel:
        stub = exchange_rates_pb2_grpc.ExchangeRateStub(channel)
        #getTest(stub)
        getEurUsd(stub)
        getUsdGbp(stub)



if __name__ == '__main__':
    run()
