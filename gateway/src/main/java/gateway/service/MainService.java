package gateway.service;

import gateway.service.commands.Registry;
import gateway.service.gateway.api.GatewayServer;
import gateway.service.registry.api.RegistrationServer;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;
import org.glassfish.jersey.servlet.ServletContainer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import static org.eclipse.jetty.servlet.ServletContextHandler.NO_SESSIONS;

public class MainService {

    private static final Logger logger = LoggerFactory.getLogger(MainService.class);

    private int gatewayPort;
    private int registryPort;

    private Registry serviceRegistry;

    private Server gatewayServer = null;

    private RegistrationServer registrationServer = null;
    private Thread registryThread = null;

    MainService(int gwPort, int regPort) {
        this.gatewayPort = gwPort;
        this.registryPort = regPort;
        this.serviceRegistry = new Registry();
    }

    public void run() throws Exception {
        startGatewayServer();
        startRegistrationServer();
    }

    public void stop() throws Exception {
        this.gatewayServer.stop();
        this.registrationServer.stopServer();
    }

    public void join() throws InterruptedException {
        // Wait for clean termination
        this.gatewayServer.join();
        this.registryThread.join();
    }

    private void startRegistrationServer() {
        this.registrationServer = new RegistrationServer(registryPort, serviceRegistry);
        registryThread = new Thread(this.registrationServer);

        logger.info("Starting the Endpoint Registration service");
        registryThread.start();
    }

    private void startGatewayServer() throws Exception {
        this.gatewayServer = new Server(this.gatewayPort);

        ServletContextHandler servletContextHandler = new ServletContextHandler();
        servletContextHandler.setContextPath("/");
        servletContextHandler.setAttribute("serviceRegistry", this.serviceRegistry);

        this.gatewayServer.setHandler(servletContextHandler);

        ServletHolder servletHolder = servletContextHandler.addServlet(ServletContainer.class, "/*");
        servletHolder.setInitOrder(0);
        servletHolder.setInitParameter(
                "jersey.config.server.provider.classnames",
                GatewayServer.class.getCanonicalName()
        );

        logger.info("Starting the REST API HTTP server");

        this.gatewayServer.start();
    }
}
