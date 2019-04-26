package gateway.service.commands;

import gateway.service.SocketIOUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.net.Socket;
import java.util.*;

public class Service {

    private String name;
    private List<ServiceEndpoint> endpoints;
    private static final Logger logger = LoggerFactory.getLogger(Service.class);
    private Random random = new Random();

    Service(String name) {
        this.name = name;
        this.endpoints = new ArrayList<>();
    }

    public String getName() {
        return this.name;
    }

    public Service addEndpoint(final ServiceEndpoint endpoint) {
        logger.info("New endpoint at {}:{} added for service {}", endpoint.getAddress(), endpoint.getPort(), this.name);
        endpoints.add(endpoint);
        return this;
    }

    public ServiceEndpoint getRandomEndpoint() {
        int index = random.nextInt(endpoints.size());
        return endpoints.get(index);
    }

    public String executeCommand(final String input) throws IOException {

        logger.debug("Trying to execute command for service {}", this.name);
        logger.debug("Command input: {}", input);


        String response;
        Socket socket = null;

        try {
            ServiceEndpoint endpoint = getRandomEndpoint();
            socket = new Socket(endpoint.getAddress(), endpoint.getPort());

            // Write the command (call the command)
            SocketIOUtils.writeString(socket, input);

            logger.debug("Executed command: {}", input);

            response = SocketIOUtils.readString(socket);

            logger.debug("Command output: {}", response);

        } catch (Exception ex) {
            logger.error("Error handling command", ex);
            response = "{\"status\":\"error\"}";
        } finally {
            if (socket != null) {
                socket.close();
            }
        }
        return response;
    }
}
