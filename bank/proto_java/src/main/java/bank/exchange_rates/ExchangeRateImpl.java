package bank.exchange_rates;

import io.grpc.stub.StreamObserver;

public class ExchangeRateImpl extends ExchangeRateGrpc.ExchangeRateImplBase {

    RatesDict rd;

    ExchangeRateImpl(RatesDict rd){
        this.rd = rd;
    }

    @Override
    public void getRates(Currencies request, StreamObserver<Rate> responseObserver) {

        while(true){

            for(Currencies.CurrencyName name: request.getNamesList()){

                Rate rate = Rate.newBuilder().putRate(name.name() , this.rd.getRate(name.name())).build();
                responseObserver.onNext(rate);

            }
            try {
                Thread.sleep(5000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

    }

    @Override
    public void getTest(NoParams request, StreamObserver<MonetaryType> responseObserver) {
        super.getTest(request, responseObserver);
    }
}
