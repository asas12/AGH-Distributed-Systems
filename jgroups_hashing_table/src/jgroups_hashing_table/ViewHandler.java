package jgroups_hashing_table;

import org.jgroups.Address;
import org.jgroups.JChannel;
import org.jgroups.MergeView;
import org.jgroups.View;

import java.util.List;

public class ViewHandler extends Thread{

    private JChannel ch;
    private MergeView view;

    public ViewHandler(JChannel ch, MergeView view){
        this.ch = ch;
        this.view = view;
    }

    public void run(){

        List<View> subgroups = view.getSubgroups();
        View chosen_view = subgroups.get(0);
        Address local_addr = ch.getAddress();
        if(!chosen_view.getMembers().contains(local_addr)){
            System.out.println("I have to update state.");
            try {
                ch.getState(chosen_view.getMembers().get(0), 0);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }else{
            System.out.println("I don't have to update state.");
        }
    }

}
