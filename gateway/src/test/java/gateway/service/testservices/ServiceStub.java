package gateway.service.testservices;

import com.fasterxml.jackson.databind.ObjectMapper;
import gateway.service.commands.Command;
import gateway.service.commands.ServiceEndpoint;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class ServiceStub {

    private final String name;

    private final List<EndpointStub> endpoints;

    private static final Logger logger = LoggerFactory.getLogger(ServiceStub.class);

    public ServiceStub(final String name, int numEndpoints, final Map<String, Command> commands) {
        this.name = name;
        this.endpoints = new ArrayList<>();

        // Configure the service endpoints
        Random rand = new Random();
        for (int i = 0; i < numEndpoints; i++) {

            ServiceEndpoint endpointConfig = new ServiceEndpoint();
            endpointConfig.setService(this.name);
            endpointConfig.setAddress("127.0.0.1");
            endpointConfig.setPort(rand.nextInt(40000) + 10000 );
            endpointConfig.setCommands(commands);

            EndpointStub endpoint = new EndpointStub(endpointConfig);
            endpoints.add(endpoint);
        }
    }

    public List<EndpointStub> getEndpoints() {
        return this.endpoints;
    }

    public void startService() {
        for (EndpointStub endpoint : endpoints) {
            endpoint.start();
        }
    }

    public void registerEndpoints(final String gatewayAddress, final int gatewayPort) throws Exception {
        for (EndpointStub endpoint : endpoints) {
            endpoint.registerEndpoint(gatewayAddress, gatewayPort);
        }
    }

    public void stopService() throws InterruptedException {
        logger.info("Stopping test service stub '{}' threads", this.name);

        for (EndpointStub endpoint : endpoints) {
            endpoint.stop();
            endpoint.join();
        }

        logger.info("Test service stub {} stopped", this.name);
    }
}
