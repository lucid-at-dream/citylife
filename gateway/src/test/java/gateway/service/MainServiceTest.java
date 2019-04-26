package gateway.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import gateway.service.commands.*;
import gateway.service.testservices.EndpointStub;
import gateway.service.testservices.ResponseStub;
import gateway.service.testservices.ServiceStub;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.ws.rs.core.MediaType;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Random;

public class MainServiceTest {

    private static final Logger logger = LoggerFactory.getLogger(MainServiceTest.class);

    private int gwPort;
    private int regPort;

    private MainService mainService;

    @Before
    public void setup() throws Exception {

        Random rand = new Random();
        gwPort = rand.nextInt(40000) + 10000;
        regPort = rand.nextInt(40000) + 10000;

        mainService = new MainService(gwPort, regPort);
        mainService.run();

        // Sleep for a while to make sure the service is started
        Thread.sleep(100); // TODO: Remove this sleep
        logger.info("GatewayServer Server started");
    }

    @After
    public void teardown() throws Exception {
        logger.info("Stopping the gateway server altogether");
        mainService.stop();
        mainService.join();
        logger.info("GatewayServer service stopped");
    }

    @Test
    public void test_SingleServiceRegistered_CommandExecutionCorrectlyForwarded() throws Exception {
        ServiceStub testService = TestUtils.newTestService("test-service", 1, this.regPort);

        // Base test configuration
        final String baseUrl = "http://127.0.0.1:" + gwPort;
        final String request = "{\"arg1\": \"value1\", \"arg2\": \"value2\"}";

        // Execute a command
        final String output = TestUtils.executeCommand( baseUrl + "/api/test-service/cmd1", request);
        ObjectMapper mapper = new ObjectMapper();
        ResponseStub response = mapper.readValue(output, ResponseStub.class);

        // Assert command correctly forwarded
        assert testService.getEndpoints().get(0).getRequests().poll().equals(request) : "Request was correctly forwarded to the service";
        assert response.getRequest().equals(request) : "The response from command execution is correct";

        // Stop the running services
        testService.stopService();
    }

    @Test
    public void test_SingleServiceRegistered_MultipleEndpoints_RequestsAreLoadBalanced() throws Exception {
        ServiceStub testService = TestUtils.newTestService("test-service", 2, this.regPort);

        // Base test configuration
        final String baseUrl = "http://127.0.0.1:" + gwPort;
        final String request = "{\"arg1\": \"value1\", \"arg2\": \"value2\"}";

        for (int i = 0; i < 100; i++) {
            // Execute a command
            final String output = TestUtils.executeCommand( baseUrl + "/api/test-service/cmd1", request);
            ObjectMapper mapper = new ObjectMapper();
            ResponseStub response = mapper.readValue(output, ResponseStub.class);

            // Assert the response is correct
            assert response.getRequest().equals(request) : "The response from command execution is correct";
        }

        // Confirm that the load balancing worked
        final double firstEndpointRequestCount = testService.getEndpoints().get(0).getRequests().size();
        final double secondEndpointRequestCount = testService.getEndpoints().get(1).getRequests().size();

        logger.info("Request counts: e1 - {}, e2 - {}", firstEndpointRequestCount, secondEndpointRequestCount);
        double ratio = firstEndpointRequestCount / secondEndpointRequestCount;

        assert 0.25 < ratio : "An endpoint did not receive 1/4 of the requests than the other";
        assert 1.75 > ratio : "An endpoint did not receive 1/4 of the requests than the other";

        // Stop the running services
        testService.stopService();
    }
}
