package com.example.krishna.mylistview;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Message;
import android.view.Menu;
import android.view.MenuItem;



import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import android.os.Handler;

import android.app.ExpandableListActivity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ExpandableListView;
import android.widget.Spinner;

import com.example.krishna.mylistview.SysCallHolder;

//import org.xml.sax.Parser;


public class TrackSysCalls extends ExpandableListActivity {

    private ArrayList<String> parentItems = new ArrayList<String>();
    private ArrayList<ArrayList<String>> childItems = new ArrayList<ArrayList<String>>();
    private MyExpandableAdapter adapter;
    private DropDown dropDown;
    public Button button;
    private Spinner spinner;
    private Handler myHandler = new Handler() {
        @Override
        public void handleMessage(Message message){
            SysCallHolder sysCallObj = (SysCallHolder)message.obj;
            adapter.addtoUI(sysCallObj);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String spid = intent.getStringExtra(MainActivity.EXTRA_PID);
        String pname = intent.getStringExtra(MainActivity.EXTRA_PNAME);
        Log.v("TRACK", "SPID :"+spid);
        Log.v("TRACK", "SPID :"+pname);
        setTitle(pname);
        //int pid = Integer.getInteger(spid);
        // this is not really  necessary as ExpandableListActivity contains an ExpandableList
       // setContentView(R.layout.activity_track_sys_calls);
        /*LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        spinner = (Spinner) inflater.inflate(R.layout.activity_track_sys_calls, null).findViewById(R.id.spinner);;
        Log.v("TRACK",""+spinner);

        ArrayList list1 = new ArrayList<String>();
        list1.add("write");
        list1.add("open");
        list1.add("close");



        ArrayAdapter<String> spinAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, list1);

        spinAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        spinner.setAdapter(spinAdapter);
        */
        ExpandableListView expandableList = getExpandableListView(); // you can use (ExpandableListView) findViewById(R.id.list)

        expandableList.setDividerHeight(2);
        expandableList.setGroupIndicator(null);
        expandableList.setClickable(true);
        BufferedReader reader = null;

        callTest(spid, pname);
        /*
        if (sysCallList != null)
            Parser.getSysCallSet(sysCallList, parentItems, childItems);
        */
        adapter = new MyExpandableAdapter(parentItems, childItems);

        adapter.setInflater((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE), this);
        expandableList.setAdapter(adapter);
        expandableList.setOnChildClickListener(this);
       // dropDown = new DropDown((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE), this);

    }

    public void callTest(String pid, String pname) {
        final String Tag = "MYAPP";
        List<SysCallHolder> syscalls = null;
        final String proname = pname;
        final String propid = pid;
        //StringBuffer outbuf = new StringBuffer("Before\n");
        new Thread(new Runnable() {
            @Override
            public void run() {


            Process process= null;
            Log.v(Tag, "Before SU Command");
            try {
                // process = Runtime.getRuntime().exec("su");
                process = Runtime.getRuntime().exec(new String[]{"/system/bin/sh", "-c", "su -c 'andro -p "+propid+" & sleep 5'"});
              /* process = new ProcessBuilder()
                        .command("/system/bin/sh")
                        .redirectErrorStream(true)
                        .start();
                */
                // process.waitFor();
                Log.v(Tag, "after su");

                launchApp(proname);
                //DataOutputStream out = new DataOutputStream(process.getOutputStream());
                OutputStream out = process.getOutputStream();
                //out.write("strace ls\n".getBytes());
                //out.flush();
                BufferedReader in = new BufferedReader(new InputStreamReader(process.getInputStream()));
                BufferedReader err = new BufferedReader(new InputStreamReader(process.getErrorStream()));
                String line = "";
                //out.writeBytes("andro\n");
                process.waitFor();

                Log.v(Tag, "before In");
                Parser.parse(in, myHandler);
                in.close();
                /*
                while ((line = in.readLine()) != null) {
                    Log.v(Tag, line+"\n");
                    //outbuf.append(line+"\n");
                }

                Log.v(Tag, line+"\n");*/
                //process.waitFor();
                //Log.v(Tag, "before err");
                //Parser.parse(err);
                /*
                while( (line = err.readLine()) != null) {
                    Log.v(Tag, line+"\n");
                    //outbuf.append(line+"\n");
                }
                */

                //out.flush();
                //out.writeBytes("strace -p 826\n");
                //process.waitFor();

                Log.v(Tag, "after write");
               /* while ((line = in.readLine()) != null) {
                    outbuf.append(line+"\n");
                }*/
               /*while( (line = err.readLine()) != null) {
                    outbuf.append(line+"\n");
                }*/
                process.waitFor();

            } catch (Exception e) {
                Log.v(Tag, "Execption Occured " +e);
            }
            finally {

                process.destroy();
            }
            Log.v(Tag, "END");
            }
        }).start();
    }
    public void launchApp(String pname) {
        Intent intent = getPackageManager().getLaunchIntentForPackage(pname);
        if (intent != null) {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);

        }
        else {
            /* bring user to the market or let them choose an app? */
            intent = new Intent(Intent.ACTION_VIEW);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    }
}
