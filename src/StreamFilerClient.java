import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;

public class StreamFilerClient {

    public static void main(String[] args) {

        final String HOST = args[0];
        final int PORT = Integer.parseInt(args[1]);
        final String DIR = args[2];

        try (Socket s = new Socket(HOST, PORT);
             Scanner input = new Scanner(new BufferedInputStream(s.getInputStream()));
             DataOutputStream output = new DataOutputStream(new BufferedOutputStream(s.getOutputStream()))
        ) {
            File folder = new File(DIR);
            File[] files = folder.listFiles();
            assert files != null;
            for (File file : files) {
                if (file.isFile()) {

                    String fileFormat = file.getName().split("\\.")[1];
                    output.writeBytes(fileFormat);
                    output.flush();
                    System.out.println(input.nextLine() + ": sending file format");

                    String fileSize = String.valueOf(file.length());
                    output.writeBytes(fileSize);
                    output.flush();
                    System.out.println(input.nextLine() + ": sending file size");

                    System.out.println("Sending " + file.getName() + " (" + fileSize + " B)" + " to server");
                    FileInputStream fis = new FileInputStream(DIR + file.getName());
                    byte[] buffer = new byte[8192];
                    int length;
                    while ((length = fis.read(buffer)) > 0) {
                        output.write(buffer, 0, length);
                        output.flush();
                    }
                    fis.close();
                    System.out.println(input.nextLine() + ": sent file\n");

                }
            }
        } catch (UnknownHostException e) {
            System.err.println("Error: Cannot find Host");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
