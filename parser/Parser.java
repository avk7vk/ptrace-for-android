package ptrace.parser;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Parser {

	public static List<SysCallHolder> parse(String filename) throws IOException {
		// TODO Auto-generated method stub
		BufferedReader reader = new BufferedReader(new FileReader(filename));
	    String line1;
	    String line2;
	    List<SysCallHolder> holders = new ArrayList<SysCallHolder>();
	    while (true) {
	    	line1 = reader.readLine();
	    	if (line1 == null) {
	    		reader.close();
	    		return holders;
	    	}
	    	line1.trim();
	    	
	    	while(countMatches(line1,"\"")%2 != 0) {
	    		if((line2 = reader.readLine()) ==null){
	    			reader.close();
	    			return holders;
	    		}
	    		if(line2.startsWith("\"")) {
	    			line2.trim();
	    			line1 = line1.concat(line2);
	    			//System.out.println(line1);
	    			break;
	    		}
	    	}
	    	line1 = line1.replaceAll(" ", "");
	    	String sysCall = line1.substring(0, line1.indexOf("("));
	    	String argString = line1.substring(line1.indexOf("(")+1, line1.lastIndexOf(")"));
	    	int retValue = Integer.parseInt(line1.substring(line1.lastIndexOf("=")+1));
	    	int index = 0;
	    	int i = 0;
	    	String[] strArgs = new String[countMatches(line1,"\"")/2];
	    	while((index = argString.indexOf("\"", index)) != -1) {
	    		int nextIndex = argString.indexOf("\"", index+1);
	    		String strArg = argString.substring(index, nextIndex+1);
	    		//System.out.println(strArg);
	    		argString = argString.replace(strArg, "\"\"");
	    		strArgs[i] = strArg;
	    		i++;
	    		index = nextIndex +1;
	    	}
	    	String[] argArray = argString.split(",");
	    	i = 0;
	    	for (int j = 0;j < argArray.length; j++) {
	    		if (argArray[j].equals("\"\"")) {
	    			argArray[j] = strArgs[i];
	    			i++;
	    		}
	    			
	    	}
	    	SysCallHolder holder = new SysCallHolder(sysCall, retValue, argArray);
	    	holders.add(holder);
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
