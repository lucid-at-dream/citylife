package gateway.service.testservices;

import com.fasterxml.jackson.databind.ObjectMapper;
import gateway.service.SocketIOUtils;
import gateway.service.commands.ServiceEndpoint;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class EndpointStub {

    private Thread serverThread;
    private volatile boolean shouldStop;
    private Queue<String> requests;
    private ServiceEndpoint endpoint;

    private static final Logger logger = LoggerFactory.getLogger(EndpointStub.class);

    public EndpointStub(ServiceEndpoint endpoint) {
        this.endpoint = endpoint;
    }

    void start() {
        // Reset everything
        this.shouldStop = false;
        this.requests = new ConcurrentLinkedQueue<>();

        // Start the service (this endpoint)
        serverThread = new Thread(
                this::runService
        );

        serverThread.start();
    }

    synchronized void stop() {
        logger.info("Stopping endpoint for service {}", endpoint.getService());
        this.shouldStop = true;
        try {
            Socket s = new Socket(this.endpoint.getAddress(), this.endpoint.getPort());
            SocketIOUtils.writeString(s, "shutdown");
        } catch (IOException ex) {
            logger.error("Error writing shutdown request to test service {}", this.endpoint.getService());
        }
    }

    void join() throws InterruptedException {
        this.serverThread.join();
    }

    public Queue<String> getRequests() {
        return this.requests;
    }

    void registerEndpoint(final String gatewayAddress, final int gatewayPort) throws Exception {

        // Build the request
        ObjectMapper mapper = new ObjectMapper();
        final String request = mapper.writeValueAsString(endpoint);

        Socket socket = new Socket(gatewayAddress, gatewayPort);

        // Write endpoint registration request
        SocketIOUtils.writeString(socket, request);

        // Read the response
        final String response = SocketIOUtils.readString(socket);

        // Log the response
        logger.info("Response from server after endpoint registration (" + endpoint.getService() + "):");
        logger.info(response);
    }

    private void runService() {

        ServerSocket serverSocket;
        try {
            serverSocket = new ServerSocket(endpoint.getPort());
        } catch (IOException ex){
            logger.error("Error starting service server for test purposes.", ex);
            return;
        }

        logger.info("Test service listening on port {}:{}", endpoint.getAddress(), endpoint.getPort());


        try {

            while (!shouldStop) {

                try {
                    Socket socket = serverSocket.accept();

                    logger.info("Test service {} listening for commands", this.endpoint.getService());

                    // Read the response
                    final String input = SocketIOUtils.readString(socket);

                    if (input.equals("shutdown")) {
                        shouldStop = true;
                        continue;
                    }

                    logger.info("Command input: {}", input);

                    // Register request for test assertions
                    this.requests.add(input);

                    // Build response
                    ResponseStub response = new ResponseStub();
                    response.setPort(endpoint.getPort());
                    response.setService(endpoint.getService());
                    response.setRequest(input);

                    // Reply
                    ObjectMapper mapper = new ObjectMapper();
                    SocketIOUtils.writeString(socket, mapper.writeValueAsString(response));

                } catch (Exception ex) {
                    logger.error("Error handling request. Ignoring.", ex);
                }
            }

        } finally {
            try {
                serverSocket.close();
            } catch (IOException ex) {
                logger.error("Errror closing server", ex);
            }
        }

    }
}
