# needed for grpc imports to work
import sys
sys.path.append('./gen/exchange_rates')

from concurrent import futures
import threading
import random

import grpc
import time

import gen.exchange_rates.exchange_rates_pb2  # "as e_r" BREAKS importing!!! why???
import gen.exchange_rates.exchange_rates_pb2_grpc

_ONE_HOUR_IN_SECONDS = 60 * 60

class RatesConcurrentDict(object):

    def __init__(self, cv):
        print('Dict initing')
        self.rates = {'EUR': 4.2795, 'USD': 3.8177, 'GBP': 3.7480, 'CHF': 4.9840}
        self.cv = cv

        thread = threading.Thread(target=self.update_rates_thread, args=(), daemon=True)
        thread.start()

    def get_rate(self, currency_name):
        """Gets only one name, can lead to taking rates from different concurrent epochs"""
        with self.cv:
            rate = self.rates[currency_name]
            return rate

    def get_rates(self, currency_names):
        """ Gets all names at once"""
        with self.cv:
            return [self.rates[currency_name] for currency_name in currency_names]

    def update_rates_thread(self):
        """ Updates rates every 5 seconds"""
        with self.cv:
            self.cv.notify_all()
            print('... Initial rates ...')
            print(self.rates)
        while True:
            time.sleep(5)
            with self.cv:
                for currency in self.rates:
                    self.rates[currency] += (random.random()-0.5)/25
                print('... Updated rates ...')
                print(self.rates)
                self.cv.notify_all()


class ExchangeRatesServicer(gen.exchange_rates.exchange_rates_pb2_grpc.ExchangeRateServicer):

    def __init__(self, rates_dict, cv):
        self.rates = rates_dict
        self.cv = cv
        print('init')

    def GetRates(self, request, context):
        """ This method returns infinite stream of requested currency reates"""
        print(request)

        real_names = [gen.exchange_rates.exchange_rates_pb2.Currencies.CurrencyName.Name(name) for name in request.names]
        print('Getting rates: ', real_names)

        with self.cv:
            self.cv.wait(timeout=0.0001)
            rates = self.rates.get_rates(real_names)

            for real_name, rate in zip(real_names, rates):
                m = gen.exchange_rates.exchange_rates_pb2.Rate()
                m.rate[real_name] = rate
                print(real_name, m.rate[real_name])

                yield m

        while True:
            with self.cv:
                # changing rates wakes up all threads
                self.cv.wait()
                rates = self.rates.get_rates(real_names)

                for real_name, rate in zip(real_names, rates):
                    m = gen.exchange_rates.exchange_rates_pb2.Rate()
                    m.rate[real_name] = rate
                    print(real_name, m.rate[real_name])

                    yield m

    def GetTest(self, request, context):
        print('test')
        #return exchange_rates_pb2.MonetaryType(units=100, cents=2)
        return 42


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))

    cv = threading.Condition()
    rates = RatesConcurrentDict(cv)

    gen.exchange_rates.exchange_rates_pb2_grpc.add_ExchangeRateServicer_to_server(
        ExchangeRatesServicer(rates, cv), server)
    server.add_insecure_port('[::]:50066')
    print('Serving')
    server.start()
    try:
        while True:
            time.sleep(_ONE_HOUR_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == "__main__":
    serve()
