from concurrent import futures

import grpc
import time

import exchange_rates_pb2
import exchange_rates_pb2_grpc

_ONE_HOUR_IN_SECONDS = 60 * 60


class ExchangeRatesServicer(exchange_rates_pb2_grpc.ExchangeRateServicer):

    def __init__(self):
        print('init')

    def GetRates(self, request, context):
        # TODO unMock code
        print('getting rates')
        print(request)

        for name in request.names:
            time.sleep(4)
            print(name)
            m = exchange_rates_pb2.Rate()

            mon = exchange_rates_pb2.MonetaryType(units=100, cents=1)
            #mon.units = 100
            #mon.cents = 1

            m.rate['EUR'].units = 10
            m.rate['EUR'].cents = 3

            yield m

    def GetTest(self, request, context):
        print('test')
        return exchange_rates_pb2.MonetaryType(units=100, cents=2)


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    exchange_rates_pb2_grpc.add_ExchangeRateServicer_to_server(
        ExchangeRatesServicer(), server)
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
