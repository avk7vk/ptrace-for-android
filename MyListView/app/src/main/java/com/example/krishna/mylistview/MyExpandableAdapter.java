package com.example.krishna.mylistview;

        import java.util.ArrayList;

        import android.app.Activity;
        import android.view.LayoutInflater;
        import android.view.View;
        import android.view.View.OnClickListener;
        import android.view.ViewGroup;
        import android.widget.BaseExpandableListAdapter;
        import android.widget.CheckedTextView;
        import android.widget.TextView;
        import android.widget.Toast;

/**
 * Created by krishna on 11/16/14.
 */
public class MyExpandableAdapter extends BaseExpandableListAdapter {

    private Activity activity;
    public ArrayList<ArrayList<String>> childtems;
    private LayoutInflater inflater;
    public ArrayList<String> parentItems, child;
    public ArrayList<String> mainItems = new ArrayList<String>();
    public ArrayList<ArrayList<String>> mainchild = new ArrayList<ArrayList<String>>();
    private String currFilter = "all";
    private ArrayList<String> myFilters = new ArrayList<String>();
    private ArrayList<ArrayList<String>> myFitersList = new ArrayList<ArrayList<String>>();

    public MyExpandableAdapter(ArrayList<String> parents, ArrayList<ArrayList<String>> childern) {
        this.parentItems = parents;
        this.childtems = childern;
        initFilters();
    }

    public void setInflater(LayoutInflater inflater, Activity activity) {
        this.inflater = inflater;
        this.activity = activity;
    }

    @Override
    public View getChildView(int groupPosition, final int childPosition, boolean isLastChild, View convertView, ViewGroup parent) {

        child = (ArrayList<String>) childtems.get(groupPosition);

        TextView textView = null;

        if (convertView == null) {
            convertView = inflater.inflate(R.layout.group, null);
        }

        textView = (TextView) convertView.findViewById(R.id.textView1);
        textView.setText(child.get(childPosition));

        convertView.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                Toast.makeText(activity, child.get(childPosition),
                        Toast.LENGTH_SHORT).show();
            }
        });

        return convertView;
    }

    @Override
    public View getGroupView(int groupPosition, boolean isExpanded, View convertView, ViewGroup parent) {

        if (convertView == null) {
            convertView = inflater.inflate(R.layout.row, null);
        }

        ((CheckedTextView) convertView).setText(parentItems.get(groupPosition)+" ("+ childtems.get(groupPosition).size()+")");
        ((CheckedTextView) convertView).setChecked(isExpanded);

        return convertView;
    }

    @Override
    public Object getChild(int groupPosition, int childPosition) {
        return null;
    }

    @Override
    public long getChildId(int groupPosition, int childPosition) {
        return 0;
    }

    @Override
    public int getChildrenCount(int groupPosition) {
        return ((ArrayList<String>) childtems.get(groupPosition)).size();
    }

    @Override
    public Object getGroup(int groupPosition) {
        return null;
    }

    @Override
    public int getGroupCount() {
        return parentItems.size();
    }

    @Override
    public void onGroupCollapsed(int groupPosition) {
        super.onGroupCollapsed(groupPosition);
    }

    @Override
    public void onGroupExpanded(int groupPosition) {
        super.onGroupExpanded(groupPosition);
    }

    @Override
    public long getGroupId(int groupPosition) {
        return 0;
    }

    @Override
    public boolean hasStableIds() {
        return false;
    }

    @Override
    public boolean isChildSelectable(int groupPosition, int childPosition) {
        return false;
    }

    public void addtoUI(SysCallHolder mySys) {
        Parser.getSysCallSet(mySys, this.mainItems, this.mainchild);
        if(checkFilter(mySys))
            notifyDataSetChanged();
        //applyFilters();
    }
    public void changeFilter(String filter) {
        System.out.println("ChangeFilter : "+filter);
        if (!myFilters.contains(filter.toLowerCase())) {
            this.currFilter = "all";
            System.out.println("\n\n\n\n\n\nDoesnt Contain Filter: "+filter);
        }
        else
            this.currFilter = filter;
        System.out.println("CurrFilter : "+filter);
        applyFilters();
        notifyDataSetChanged();
    }

    public void applyFilters() {
        this.parentItems.clear();
        this.childtems.clear();
        System.out.println("In ApplyFilter currFIlter : "+currFilter);
        if (this.currFilter.equals("all")) {
            this.parentItems.addAll(this.mainItems);
            this.childtems.addAll(this.mainchild);
        }
        else
        {
            int pos = this.myFilters.indexOf(currFilter);
            if (pos == -1) {
                this.parentItems.addAll(this.mainItems);
                this.childtems.addAll(this.mainchild);
                return;
            }
            ArrayList <String> syscalls = this.myFitersList.get(pos);
            for (String syscall : this.mainItems)
            {
                if (syscalls.contains(syscall.toLowerCase()))
                {
                    int tpos = this.mainItems.indexOf(syscall);
                    if(tpos == -1) continue;
                    this.parentItems.add(syscall);
                    this.childtems.add(this.mainchild.get(tpos));
                }
            }
        }
    }
    public boolean checkFilter(SysCallHolder holder) {
        //System.out.println("In CheckFilter currFIlter : "+currFilter);
       int pos = this.myFilters.indexOf(currFilter);
       boolean ret = false;

                ArrayList <String> syscalls = this.myFitersList.get(pos);
                String sysName = holder.getSysCall();
                String sysArgs = holder.getArgsAsString();
                if (syscalls.contains(sysName.toLowerCase()) || currFilter.equals("all"))
                {
                    if(!parentItems.contains(sysName)) {
                        parentItems.add(sysName);
                        childtems.add(new ArrayList<String>());
                    }
                    int tpos = parentItems.indexOf(sysName);
                    childtems.get(tpos).add(sysArgs);
                    ret = true;
                }
        return ret;
    }
    public void initFilters() {
        ArrayList<String> tmp = new ArrayList();
        this.myFilters.add("all");
            tmp = new ArrayList();
            this.myFitersList.add(tmp);
        this.myFilters.add("file_io");
            tmp = new ArrayList();
            tmp.add("read");
            tmp.add("write");
            tmp.add("open");
            tmp.add("stat");
            tmp.add("lstat");
            tmp.add("fstat");
            tmp.add("lseek");
            tmp.add("llseek");
            tmp.add("readv");
            tmp.add("writev");
            tmp.add("pread");
            tmp.add("pwrite");
            tmp.add("sync");
            tmp.add("fsync");
            tmp.add("utime");
            tmp.add("utimes");
            tmp.add("access");
            tmp.add("chmod");
            tmp.add("creat");
            tmp.add("close");
            tmp.add("mkdir");
            tmp.add("rmdir");
            tmp.add("mknod");
            tmp.add("unlink");
            tmp.add("symlink");
            tmp.add("link");
            tmp.add("rename");
            tmp.add("ioctl");
            this.myFitersList.add(tmp);
        this.myFilters.add("network");
            tmp = new ArrayList();
            tmp.add("recvfrom");
            tmp.add("sendto");
            tmp.add("socketcall");
            tmp.add("socketpair");
            tmp.add("bind");
            tmp.add("listen");
            tmp.add("accept");
            tmp.add("connect");
            tmp.add("getsockname");
            tmp.add("getpeername");
            tmp.add("send");
            tmp.add("recvfrom");
            tmp.add("recv");
            tmp.add("setsockopt");
            tmp.add("getsockopt");
            tmp.add("shutdown");
            tmp.add("sendmsg");
            tmp.add("recvmsg");
            tmp.add("socket");
            this.myFitersList.add(tmp);
        this.myFilters.add("memory");
            tmp = new ArrayList();
            tmp.add("brk");
            tmp.add("munmap");
            tmp.add("mprotect");
            tmp.add("sendfile");
            tmp.add("msync");
            tmp.add("mlock");
            tmp.add("munlock");
            tmp.add("mlockall");
            tmp.add("munlockall");
            tmp.add("mremap");
            tmp.add("swapoff");
            tmp.add("swapon");
            this.myFitersList.add(tmp);
    }
}
