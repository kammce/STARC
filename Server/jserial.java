import java.util.*;
import java.text.*;

public class jserial {
	static final char us = 0x1F;
	public static void main(String args[]) {
		if(args.length != 1) {
			System.out.println("INVALID");
			return;
		}
		String incomming[] = args[0].split(us+"",2);
		if(incomming[0].equals("GetTime")) {
			DateFormat dateFormat = new SimpleDateFormat("HH:mm");
			Date date = new Date();
			String time = dateFormat.format(date);
			System.out.println("CurrentTime"+us+time);
		} if(incomming[0].equals("FoundPerson")) {
			System.out.println("Cool Story Bro!");
		} else {
			System.out.println("INVALID");
		}
	}
}
