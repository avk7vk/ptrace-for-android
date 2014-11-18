package com.example.krishna.mylistview;

import android.os.Message;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import android.os.Handler;

public class Parser {

    public static void getSysCallSet(SysCallHolder call, ArrayList<String> sysCalls,
                                     ArrayList<ArrayList<String>> sysCallArgs) {
        //sysCalls = new ArrayList<String>();
        //sysCallArgs = new ArrayList<ArrayList<String>>();
            String sysCall = call.getSysCall();
            if (!sysCalls.contains(sysCall)) {
                sysCalls.add(sysCall);
                sysCallArgs.add(new ArrayList<String>());
            }
            int index = sysCalls.indexOf(sysCall);
            sysCallArgs.get(index).add(call.getArgsAsString());
    }
    public static ArrayList<ArrayList<String>>
    getSysCallDetailsAsString(List<SysCallHolder> holders) {

        return null;
    }
    public static void parse(BufferedReader br, Handler myHandler) throws IOException {
        // TODO Auto-generated method stub
        BufferedReader reader = br;
        String line1;
        String line2;
        List<SysCallHolder> holders = new ArrayList<SysCallHolder>();

            try {
                while ((line1 = reader.readLine()) != null) {
                    if (line1.startsWith("START_TRACE"))
                        break;
                    Log.v("Parser1", line1 + "\n");
                }
            }catch (Exception e) {
                Log.v("Parser1c", "Exception :" +e);
            }
            while (true) {
                Log.v("test2","asdf");
                try {
                    line1 = reader.readLine();
                    //Log.v("Parser21", line1 + "\n");
                    if (line1 == null ) {
                        Log.v("test1", "ret");
                        //reader.close();
                        break;
                    } else if (line1.startsWith("END_STRACE")) {
                        Log.v("test1", "ret1");
                        break;
                    }
                    if (!line1.contains("(")) {
                        Log.v("test", line1);
                        continue;
                    }
                    line1.trim();

                    while (countMatches(line1, "\"") % 2 != 0) {
                        if (!reader.ready()) {
                            Log.v("testasdfg1", "ret");
                            throw new Exception("MY Exception");
                        }
                        if ((line2 = reader.readLine()) == null) {
                            //reader.close();
                            Log.v("test1w3rert", "ret");
                            return;
                        }
                        if (line2.startsWith("\"")) {
                            line2.trim();
                            line1 = line1.concat(line2);
                            //System.out.println(line1);
                            break;
                        }
                    }
                    line1 = line1.replaceAll(" ", "");
                    String sysCall = line1.substring(0, line1.indexOf("("));
                    String argString = line1.substring(line1.indexOf("(") + 1, line1.lastIndexOf(")"));
                    int retValue = Integer.parseInt(line1.substring(line1.lastIndexOf("=") + 1));
                    int index = 0;
                    int i = 0;
                    String[] strArgs = new String[countMatches(line1, "\"") / 2];
                    while ((index = argString.indexOf("\"", index)) != -1) {
                        int nextIndex = argString.indexOf("\"", index + 1);
                        String strArg = argString.substring(index, nextIndex + 1);
                        //System.out.println(strArg);
                        argString = argString.replace(strArg, "\"\"");
                        strArgs[i] = strArg;
                        i++;
                        index = nextIndex + 1;
                    }
                    String[] argArray = argString.split(",");
                    i = 0;
                    for (int j = 0; j < argArray.length; j++) {
                        if (argArray[j].equals("\"\"")) {
                            argArray[j] = strArgs[i];
                            i++;
                        }

                    }
                    SysCallHolder holder = new SysCallHolder(sysCall, retValue, argArray);
                    Log.v("Parser23", sysCall + " " + retValue + "" + argArray + "\n");
                    //holders.add(holder);
                    Message m = new Message();
                    m.setTarget(myHandler);
                    m.obj = holder;
                    m.sendToTarget();


                } catch (Exception e) {
                    Log.v("Parser24", "Execption :" + e);
                    continue;
                }
            }
    }

    private static int countMatches(String line1, String string) {
        // TODO Auto-generated method stub
        int count = 0;
        int index = 0;
        while ((index = line1.indexOf(string, index)) != -1) {
            //System.out.println(line1+" "+index);
            count++;
            index++;
        }
        return count;
    }

}
