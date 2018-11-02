//
public class StandAloneApp
{
    static {
       	System.loadLibrary("bes_dispatch");
	System.loadLibrary("bes_standalone");
    }

    private native void sayHello();

    public static void main(String[] args) {
	new StandAloneApp().sayHello();
    }
}