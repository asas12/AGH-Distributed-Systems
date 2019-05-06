import grpc

# needed for grpc imports to work
import sys
sys.path.append('./gen/exchange_rates')

import gen.exchange_rates.exchange_rates_pb2
import gen.exchange_rates.exchange_rates_pb2_grpc


def getAllRates(stub):
    print('getting rates')
    c = gen.exchange_rates.exchange_rates_pb2.Currencies()

    c.names.append(gen.exchange_rates.exchange_rates_pb2.Currencies.EUR)
    print(c)
    rates = stub.GetRates(c)
    #print(type(c))
    print(rates)
    for rate in rates:
        print(rate)


def getTest(stub):

    print('getting test')

    res = stub.GetTest(gen.exchange_rates.exchange_rates_pb2.NoParams())

    print(res)


def getEurUsd(stub):
    print("-------------- getEurUsd --------------")
    c = gen.exchange_rates.exchange_rates_pb2.Currencies()
    c.names.exchange_rates.append(gen.exchange_rates.exchange_rates_pb2.Currencies.EUR)
    c.names.exchange_rates.append(gen.exchange_rates.exchange_rates_pb2.Currencies.USD)

    rates = stub.GetRates(c)

    for rate in rates:
        print(rate)


def getUsdGbp(stub):
    print("-------------- getUsdGbp --------------")
    c = gen.exchange_rates.exchange_rates_pb2.Currencies()
    c.names.append(gen.exchange_rates.exchange_rates_pb2.Currencies.GBP)
    c.names.append(gen.exchange_rates.exchange_rates_pb2.Currencies.USD)

    rates = stub.GetRates(c)

    print(rates)

    for rate in rates:
        print(rate)


def run():
    with grpc.insecure_channel('localhost:50066') as channel:
        stub = gen.exchange_rates.exchange_rates_pb2_grpc.ExchangeRateStub(channel)
        #getTest(stub)
        #getEurUsd(stub)
        getUsdGbp(stub)


if __name__ == '__main__':




    run()
