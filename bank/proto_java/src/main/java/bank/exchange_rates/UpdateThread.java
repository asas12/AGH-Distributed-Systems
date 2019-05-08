package bank.exchange_rates;

class UpdateThread extends Thread{

    RatesDict rd;

    UpdateThread(RatesDict rd){

        this.rd = rd;
    }


    public void run(){
        while(true) {
            try {
                Thread.sleep(5000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            //System.out.println("Updating");
            // TODO dodać zmienną warunkową
            rd.updateRates();
        }
    }
}