package bank.exchange_rates;

import io.grpc.Server;
import io.grpc.ServerBuilder;

import java.io.IOException;

public class ExchangeRatesServer {

    public static void main(String[] args){

        RatesDict rd = new RatesDict();

        new UpdateThread(rd).start();

        Server server = ServerBuilder
                .forPort(50066)
                .addService(new ExchangeRateImpl(rd)).build();

        try {
            server.start();
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            server.awaitTermination();
        } catch (InterruptedException e) {
            System.out.println("Stop");
        }
    }

}
