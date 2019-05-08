import grpc

# needed for grpc imports to work
import sys
sys.path.append('./gen/exchange_rates')

import gen.exchange_rates.exchange_rates_pb2 as e_r
import gen.exchange_rates.exchange_rates_pb2_grpc as e_r_grpc


def getAllRates(stub):
    print('getting rates')
    c = e_r.Currencies()

    c.names.append(e_r.Currencies.EUR)
    print(c)
    rates = stub.GetRates(c)
    #print(type(c))
    print(rates)
    for rate in rates:
        print(rate)


def getTest(stub):

    print('getting test')

    res = stub.GetTest(e_r.NoParams())

    print(res)


def getEurUsd(stub):
    print("-------------- getEurUsd --------------")
    c = e_r.Currencies()
    c.names.exchange_rates.append(e_r.Currencies.EUR)
    c.names.exchange_rates.append(e_r.Currencies.USD)

    rates = stub.GetRates(c)

    for rate in rates:
        print(rate, type(rate))

# dobrze, na górze źle
def getUsdGbp(stub):
    print("-------------- getUsdGbp --------------")
    c = e_r.Currencies()
    c.names.append(e_r.Currencies.GBP)
    c.names.append(e_r.Currencies.USD)

    rates = stub.GetRates(c)

    print(rates)

    for rate in rates:
        for key in rate.rate:
            print( {key: rate.rate[key]})
        #print(rate.mapfield, type(rate))


def getRates(stub, currencies):
    c = e_r.Currencies()
    for currency_name in currencies:
        c.names.append(e_r.Currencies.CurrencyName.Value(currency_name))

    rates = stub.GetRates(c)
    try:
        for rate in rates:
            for key in rate.rate:
                yield (key, rate.rate[key])
    except grpc.RpcError:
        print('No exchange rates server! Rates will not be updated.')


def subscribe(rates):
    with grpc.insecure_channel('localhost:50066') as channel:
        stub = e_r_grpc.ExchangeRateStub(channel)
        yield from getRates(stub, rates)


def run():
    with grpc.insecure_channel('localhost:50066') as channel:
        stub = e_r_grpc.ExchangeRateStub(channel)
        #getTest(stub)
        #getEurUsd(stub)
        getUsdGbp(stub)
        #getRates(rates)


if __name__ == '__main__':
    run()
