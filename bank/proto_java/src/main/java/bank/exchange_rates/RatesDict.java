package bank.exchange_rates;

import java.util.HashMap;
import java.util.Random;

class RatesDict {

    private HashMap<String, Double> rates;

    RatesDict(){
        this.rates = new HashMap<String, Double>();
        this.rates.put("EUR", 4.2795);
        this.rates.put("USD", 3.8177);
        this.rates.put("GBP", 3.7480);
        this.rates.put("CHF", 4.9840);
    }


    synchronized double getRate(String currency){
        System.out.print(currency+'\t');
        System.out.println(this.rates.get(currency));
        return this.rates.get(currency);
    }

    synchronized void updateRates(){
        Random rand = new Random();

        for(String currency : this.rates.keySet()){
            double r = this.rates.get(currency);
            this.rates.put(currency, r+(rand.nextDouble()-0.5)/25);
        }
    }



}
