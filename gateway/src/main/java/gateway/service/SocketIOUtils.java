package gateway.service;

import gateway.service.registry.api.RegistrationServer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class SocketIOUtils {

    private static final Logger logger = LoggerFactory.getLogger(RegistrationServer.class);

    private SocketIOUtils() {}

    public static String readString(Socket socket) throws IOException, InterruptedException {
        DataInputStream reader = new DataInputStream(socket.getInputStream());
        byte[] input = new byte[65536];

        int readBytes = -1;
        while ((readBytes = reader.read(input)) < 0) {
            Thread.sleep(50);
        }
        socket.shutdownInput();

        logger.debug("Read {} Bytes", readBytes);

        return new String(input, 0, readBytes, StandardCharsets.UTF_8);
    }

    public static void writeBytes(Socket socket, byte[] data) throws IOException {
        DataOutputStream writer = new DataOutputStream(socket.getOutputStream());
        writer.write(data);
        socket.shutdownOutput();
    }

    public static void writeString(Socket socket, String data) throws IOException {
        writeBytes(socket, data.getBytes(StandardCharsets.UTF_8));
    }
}
