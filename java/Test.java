import java.util.List;
import javax.smartcardio.*;

public class Test {
 public static void main(String[] args) {
  try {
   // Display the list of terminals
   TerminalFactory factory = TerminalFactory.getDefault();
   List<CardTerminal> terminals = factory.terminals().list();
   System.out.println("Terminals: " + terminals);

   // Use the first terminal
   CardTerminal terminal = terminals.get(0);

   // Connect wit hthe card
   Card card = terminal.connect("*");
   System.out.println("card: " + card);
   CardChannel channel = card.getBasicChannel();

   // Send Select Applet command
   byte[] aid = {(byte) 0xF0, 0x00, 0x00, 0x02, 0x47, 0x10, 0x01, 0x09, 0x75, 0x00, 0x00, 0x00, 0x34, 0x74, 0x20};
   ResponseAPDU answer = channel.transmit(new CommandAPDU(0x00, 0xA4, 0x04, 0x00, aid));
   System.out.println("answer: " + answer.toString());

   // Send test command
   byte[] command = {0x00, 0x00};
   answer = channel.transmit(new CommandAPDU(0x00, 0xCB, 0x00, 0x00, command));
   System.out.println("answer: " + answer.toString());
   byte r[] = answer.getData();
   for (int i=0; i<r.length; i++)
    System.out.print((char)r[i]);
   System.out.println();

   // Disconnect the card
   card.disconnect(false);
  } catch(Exception e) {
   System.out.println("Ouch: " + e.toString());
  }
 }
}
