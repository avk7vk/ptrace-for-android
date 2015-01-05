package com.example.krishna.mylistview;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


public class MainActivity extends Activity {

    String mytag = "MainActivity";
    public final static String EXTRA_PID = "com.example.myfirstapp.PID";
    public final static String EXTRA_PNAME = "com.example.myfirstapp.PNAME";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final ListView listview = (ListView)findViewById(R.id.appsListView);
        ArrayList<PInfo> appsList = getInstalledApps(false);//getRunningApps();
        final AppsAdapter adapter = new AppsAdapter(this, appsList);

        listview.setAdapter(adapter);
        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                int itemPosition =i;
                PInfo item = (PInfo)listview.getItemAtPosition(itemPosition);
                Log.v("MainActivity","App :"+item.appname);
                //launchApp(item);
                trackSyscalls(item);
                Log.v("MainActivity","Package :"+item.pname);


            }
        });

    }
    public String exeCommand(int pid) {
        String testString = "";
        try {
            Log.v(mytag, "Before");
            Process p = Runtime.getRuntime().exec(new String[] { "strace -p "+pid+" -o /storage/sdcard/dummy.txt" });

            //p.waitFor();
            /*DataOutputStream outs=new DataOutputStream(p.getOutputStream());
            BufferedReader inreader;
            outs.writeBytes("andro\n");

            inreader = new BufferedReader(new InputStreamReader(p.getInputStream()));
            String line = "";
            int i = 0;
            while ((line = inreader.readLine())!= null && i++ < 10) {
                output.append(line + "\n");
            }*/
            Log.v(mytag, "Got SU Perms & Executed");
            testString = "Got SU Perms & Executed";
        } catch (Exception e) {
            Log.v(mytag, "Got Exeception " + e);
            testString = "Got Exeception " + e;
        }
        return testString;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public boolean launchApp(PInfo info) {
        boolean launched = false;
        try {
            final Intent intent = getPackageManager().getLaunchIntentForPackage(info.pname);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    startActivity(intent);
                }
            }).start();
            launched = true;
        }catch(Exception e) {
            System.out.println("Exception : "+e);
        }
        return launched;
    }

    public void trackSyscalls(PInfo item) {

        if(launchApp(item)) {
            getPidOfApp(item);
            final Intent intent = new Intent(this, TrackSysCalls.class);
            intent.putExtra(EXTRA_PID, item.pid + "");
            intent.putExtra(EXTRA_PNAME, item.pname);
            //intent.putExtra(EXTRA_PID, item);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    startActivity(intent);
                }
            }).start();
        }
    }
    private void getPidOfApp(PInfo item) {
        int i = 0;
        while (true){
            i++;
            System.out.println("In Iter : "+i);
            ActivityManager manager = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
            List<ActivityManager.RunningAppProcessInfo> services = manager.getRunningAppProcesses();
            for(ActivityManager.RunningAppProcessInfo service : services ) {
                try {
                    if(item.pname.equals(service.processName)) {
                        item.pid = service.pid;
                        return;
                    }
                } catch (Exception e) {
                    Log.v("MAINACT", "Exception" + e);
                }
            }
        }
    }
    private ArrayList<PInfo> getInstalledApps(boolean getSysPackages) {
        ArrayList<PInfo> res = new ArrayList<PInfo>();
        List<PackageInfo> packs = getPackageManager().getInstalledPackages(0);
        for(int i=0;i<packs.size();i++) {

            try {
                PackageInfo p = packs.get(i);

                ApplicationInfo ai = p.applicationInfo;
                PInfo newInfo = new PInfo();
                newInfo.appname = p.applicationInfo.loadLabel(getPackageManager()).toString();
                newInfo.pname = p.packageName;
                newInfo.icon = p.applicationInfo.loadIcon(getPackageManager());
                if (getPackageManager().getLaunchIntentForPackage(p.packageName) == null) {
                    continue;
                }
                res.add(newInfo);
            }catch (Exception e){
                System.out.println("Exception @getInstalledApps : "+e);
            }
        }
        return res;
    }
    private ArrayList<PInfo> getRunningApps() {
        ArrayList<PInfo> res = new ArrayList<PInfo>();
        ActivityManager manager = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> services = manager.getRunningAppProcesses();
        for(ActivityManager.RunningAppProcessInfo service : services ) {
            try {
                ApplicationInfo app = getPackageManager().getApplicationInfo(service.processName, 0);
                PInfo newInfo = new PInfo();
                newInfo.appname = app.loadLabel(getPackageManager()).toString();
                newInfo.pname = service.processName;
                newInfo.icon = app.loadIcon(getPackageManager());
                newInfo.pid = service.pid;
                res.add(newInfo);

            } catch (PackageManager.NameNotFoundException e) {

                Log.v("MAINACT", "Exception"+e);
            }
            finally{
                continue;
            }

        }
        return res;
    }

    public class AppsAdapter extends BaseAdapter {
        private LayoutInflater inflater;
        private ArrayList<PInfo> mApps;

        public AppsAdapter(Context context, ArrayList<PInfo> mApps) {
            this.inflater = LayoutInflater.from(context);
            this.mApps = mApps;
        }

        public View getView(int position, View convertView, ViewGroup parent) {

            ViewHendler hendler;
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.simple_list_row, null);
                hendler = new ViewHendler();
                hendler.textLable = (TextView)convertView.findViewById(R.id.app_label);
                hendler.iconImage = (ImageView)convertView.findViewById(R.id.app_icon);
                convertView.setTag(hendler);
            } else {
                hendler = (ViewHendler)convertView.getTag();
            }
            PInfo info = this.mApps.get(position);
            hendler.iconImage.setImageDrawable(info.icon);
            hendler.textLable.setText(info.appname);
            hendler.textLable.setPadding(100, 40, 0, 10);

            return convertView;

        }
        class ViewHendler{
            TextView textLable;
            ImageView iconImage;
        }


        public final int getCount() {
            return mApps.size();
        }

        public final Object getItem(int position) {
            return mApps.get(position);
        }

        public final long getItemId(int position) {
            return position;
        }
    }
}

