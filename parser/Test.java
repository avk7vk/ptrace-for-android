package ptrace.parser;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

public class Test {

	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub
		if (args.length != 1)
			throw new ExceptionInInitializerError("Provide log file");
		List<SysCallHolder> holders = Parser.parse(args[0]);
		for (Iterator<SysCallHolder> iter = holders.iterator();iter.hasNext();){
			SysCallHolder item = iter.next();
			System.out.println(item.getSysCall()+"\t"+item.getReturnVal());
			for (int i = 0; i < item.getNumArgs(); i++) {
				System.out.print(item.getArg(i)+"\t");
			}
			System.out.println();
		}
	}

}
