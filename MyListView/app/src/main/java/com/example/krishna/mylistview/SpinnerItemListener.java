package com.example.krishna.mylistview;

import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Toast;

import android.os.Handler;

/**
 * Created by krishna on 11/30/14.
 */
class SpinnerItemListener implements AdapterView.OnItemSelectedListener {

    private Handler mySpinHandler;

    public SpinnerItemListener(Handler mySpinHandler) {
        this.mySpinHandler = mySpinHandler;
    }
    public void onItemSelected(AdapterView<?> parent, View view, int pos,long id) {

        Message m = new Message();
        m.setTarget(this.mySpinHandler);
        switch (pos) {
            case 0: //All
                m.obj = "all";
                break;
            case 1: //File I/O
                m.obj = "file_io";
                break;
            case 2: //Network
                m.obj = "network";
                break;
            case 3: //permissions
                m.obj = "memory";
                break;
            default :
                m.obj = "all";
        }
        m.sendToTarget();
        Toast.makeText(parent.getContext(),
                "OnItemSelectedListener : " + " "+(String)m.obj+" " +parent.getItemAtPosition(pos).toString(),
                Toast.LENGTH_SHORT).show();

    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {
        // TODO Auto-generated method stub
    }

}
