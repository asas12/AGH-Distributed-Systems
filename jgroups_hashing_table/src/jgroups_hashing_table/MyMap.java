package jgroups_hashing_table;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.util.Collections;
import java.util.HashMap;

import org.jgroups.*;
import org.jgroups.protocols.*;
import org.jgroups.protocols.pbcast.*;
import org.jgroups.stack.ProtocolStack;
import org.jgroups.util.Util;

import static java.lang.Thread.sleep;

public class MyMap implements SimpleStringMap {

    private JChannel channel;
    private final HashMap<String, Integer> mapInstance;

    private DISCARD discard;

    public MyMap() throws Exception {
        mapInstance = new HashMap<String, Integer>();

        channel = new JChannel(false);

        ProtocolStack stack=new ProtocolStack();
        channel.setProtocolStack(stack);
        stack.addProtocol(new UDP().setValue("mcast_group_addr",InetAddress.getByName("230.100.200.53")))
                .addProtocol(new PING())
                .addProtocol(new MERGE3())
                .addProtocol(new FD_SOCK())
                .addProtocol(new FD_ALL().setValue("timeout", 12000).setValue("interval", 3000))
                .addProtocol(new VERIFY_SUSPECT())
                .addProtocol(new BARRIER())
                .addProtocol(new NAKACK2())
                .addProtocol(new UNICAST3())
                .addProtocol(new STABLE())
                .addProtocol(new GMS())
                .addProtocol(new UFC())
                .addProtocol(new MFC())
                .addProtocol(new FRAG2())
                .addProtocol(new STATE_TRANSFER())
                .addProtocol(new SEQUENCER());
                //.addProtocol(new FLUSH());

        stack.init();

        setReceiver(channel);

        channel.connect("hashMapCluster");

        channel.getState(null, 0);

        System.out.println("Finished setting up.\nCommands: put <key> <val>; get <key>; contains <key>; remove <key>; close.");
    }

    public boolean containsKey(String key){
        return mapInstance.containsKey(key);
    }

    public Integer get(String key){
        return mapInstance.get(key);
    }

    public void put(String key, Integer value) {
        synchronized (mapInstance) {
            Integer was = mapInstance.put(key, value);
            String m = "p " + key + " " + value;
            Message msg = new Message(null, null, m);
            try {
                channel.send(msg);
            } catch (Exception e) {
                e.printStackTrace();
            }
            System.out.println("Was: " + was + ", is: "+ value);
        }
    }

    public Integer remove(String key){
        Integer returnedValue;
        synchronized (mapInstance){
            returnedValue = mapInstance.remove(key);
            String m = "r " + key + " " + returnedValue;
            Message msg = new Message(null, null, m);
            try {
                channel.send(msg);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return returnedValue;
    }

    public void close(){
        try {
            sleep(60000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        channel.close();
    }

    public String print(){
        return mapInstance.toString();
    }

    public void drop(){
        this.discard = new DISCARD().setDiscardAll(true);
        channel.getProtocolStack().addProtocol(discard);
        System.out.println(channel.getProtocolStack().getProtocols());
    }

    public void undrop(){
        channel.getProtocolStack().removeProtocol(DISCARD.class);
        System.out.println(channel.getProtocolStack().getProtocols());
    }

    private void setReceiver(JChannel channel){
        channel.setReceiver(new ReceiverAdapter(){
            @Override
            public void viewAccepted(View view) {
                // wyświetla listę instancji w klastrze przy wchodzeniu/ wychodzeniu
                super.viewAccepted(view);
                System.out.println("Instances: " + view.toString());
                if(view instanceof MergeView){
                    System.out.println("View - instanceof mergeview.");
                    MergeView tmp = (MergeView) view;
                    ViewHandler vh = new ViewHandler(channel, tmp);
                    vh.start();
                }
            }
            public void receive(Message msg) {
                // odpala się przy otrzymaniu wiadomości
                if(msg.getSrc().equals(channel.getAddress())){
                    System.out.println("Discarding message from myself...");
                }else {
                    System.out.println("Received msg from " + msg.getSrc() + ": " + msg.getObject());
                    synchronized (mapInstance) {
                        String info = msg.getObject().toString();
                        String[] tokens = info.split(" ");
                        if (tokens[0].equals("r")) {
                            mapInstance.remove(tokens[1]);
                            System.out.println("Removing " + tokens[1]);
                        } else {
                            mapInstance.put(tokens[1], Integer.parseInt(tokens[2]));
                            System.out.println("Updating " + tokens[1]);
                            //this.;
                        }
                    }
                }
            }
            public void getState(OutputStream output) throws Exception {
                synchronized (mapInstance){
                    Util.objectToStream(mapInstance, new DataOutputStream(output));
                }
                System.out.println("Get state.");

            }

            public void setState(InputStream input) throws Exception {
                HashMap<String, Integer> map;
                map = (HashMap<String, Integer>) Util.objectFromStream(new DataInputStream(input));
                synchronized (mapInstance){
                    mapInstance.clear();
                    mapInstance.putAll(map);
                }
                System.out.println("Setting state: " + Collections.singletonList(map));
            }
        });
    }
}
