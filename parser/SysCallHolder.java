package ptrace.parser;



public class SysCallHolder {
	private String sysCall;
	private String[] args = null;
	private int returnVal;
	public SysCallHolder(String sysCall, int returnVal, String[] args){
		setArgs(args);
		setReturnVal(returnVal);
		setSysCall(sysCall);
	}
	public String getSysCall() {
		return sysCall;
	}
	public void setSysCall(String sysCall) {
		this.sysCall = sysCall;
	}
	public int getReturnVal() {
		return returnVal;
	}
	public void setReturnVal(int returnVal) {
		this.returnVal = returnVal;
	}
	public int getNumArgs() {
		return args.length;
	}
	public String[] getArgs() {
		return args;
	}
	public String getArg(int i) {
		return args[i];
	}
	public void setArgs(String[] args) {
		this.args = args;
	}
}
