package jgroups_hashing_table;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Main {
    public static void main(String[] args) throws Exception {

        System.setProperty("java.net.preferIPv4Stack","true");

        MyMap myMap = new MyMap();

        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String in;
        boolean close = false;

        while(!close){
            in = br.readLine();
            String[] tokens = in.split(" ");
            try {
                switch (tokens[0]) {
                    case "get":
                        System.out.println("Getting: " + tokens[1]);
                        System.out.println(myMap.get(tokens[1]));
                        break;
                    case "contains":
                        System.out.println("Checking if contains: " + tokens[1]);
                        System.out.println(myMap.containsKey(tokens[1]));
                        break;
                    case "put":
                        System.out.println("Putting number: " + tokens[2] + " under: " + tokens[1]);
                        myMap.put(tokens[1], Integer.parseInt(tokens[2]));
                        break;
                    case "remove":
                        System.out.println("Removing: " + tokens[1]);
                        System.out.println(myMap.remove(tokens[1]));
                        break;
                    case "close":
                        System.out.println("Closing channel.");
                        close = true;
                        break;
                    case "print":
                        System.out.println(myMap.print());
                        break;
                    case "drop":
                        myMap.drop();
                        break;
                    case "undrop":
                        myMap.undrop();
                        break;
                    default:
                        System.out.println("Unknown command.");

                }
            } catch (ArrayIndexOutOfBoundsException e){
                System.out.println("Too few arguments!");
            }
        }
        myMap.close();

    }
}
