package gateway.service.registry.api;

import com.fasterxml.jackson.databind.ObjectMapper;
import gateway.service.SocketIOUtils;
import gateway.service.commands.Registry;
import gateway.service.commands.ServiceEndpoint;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class RegistrationServer implements Runnable {

    private static final Logger logger = LoggerFactory.getLogger(RegistrationServer.class);
    private final int port;
    private final Registry serviceRegistry;
    private volatile boolean shouldStop = false;
    private final ExecutorService executor = Executors.newCachedThreadPool();

    public RegistrationServer(int port, Registry serviceRegistry) {
        this.port = port;
        this.serviceRegistry = serviceRegistry;
    }

    @Override
    public void run() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {

            logger.info("Registration server is listening on port {}", port);
            while (!shouldStop) {
                final Socket socket = serverSocket.accept();
                executor.execute(
                        () -> this.handleRequest(socket)
                );
            }

        } catch (IOException ex) {
            logger.error("Registration server exception: {}", ex.getMessage(), ex);

        } finally {
            executor.shutdown();
            try {
                if (!executor.awaitTermination(10, TimeUnit.SECONDS)) {
                    logger.info("Executor service for endpoint registration did not stop within the timeout.");
                    int droppedTaskCount = executor.shutdownNow().size();
                    logger.info("Registration service executor shutdown abruptly. {} tasks did not terminate.", droppedTaskCount);
                }
            } catch (InterruptedException ex) {
                logger.error("Registration server exception during executor shutdown", ex);
                Thread.currentThread().interrupt();
            }
        }
    }

    private void handleRequest(Socket socket) {

        logger.info("Request received to register an endpoint");

        try {
            // Read input
            final String input = SocketIOUtils.readString(socket);

            // Tell the server to shutdown.
            if (input.equals("shutdown")) {
                this.shouldStop = true;
                return;
            }

            // Parse JSON
            ObjectMapper mapper = new ObjectMapper();
            ServiceEndpoint endpoint = mapper.readValue(input, ServiceEndpoint.class);

            logger.info("Registering endpoint");
            this.serviceRegistry.registerEndpoint(endpoint);

            // Reply
            SocketIOUtils.writeString(socket, "{\"status\": \"Success\"}");

        } catch (Throwable ex) {

            logger.error("Exception registering endpoint", ex);
            try {
                SocketIOUtils.writeString(socket, "{\"status\": \"Error\"}");
            } catch (IOException ex2) {
                logger.info("Exception writing error registration response {}", ex2.getMessage());
            }

        } finally {
            try {
                logger.info("Closing registration connection");
                socket.close();
            } catch (Exception ex) {
                logger.info("Error", ex);
            }
        }
    }

    public synchronized void stopServer() {
        this.shouldStop = true;
        try {
            Socket s = new Socket("localhost", this.port);
            SocketIOUtils.writeString(s, "shutdown");
        } catch (IOException ex) {
            logger.error("Error writing shutdown request to registration endpoint");
        }
    }
}
