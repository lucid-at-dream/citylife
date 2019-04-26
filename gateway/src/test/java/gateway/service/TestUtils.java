package gateway.service;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import gateway.service.commands.Command;
import gateway.service.commands.CommandParameter;
import gateway.service.testservices.ServiceStub;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.ws.rs.core.MediaType;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

public class TestUtils {

    private static final Logger logger = LoggerFactory.getLogger(TestUtils.class);

    public static String executeCommand(String commandURL, String commandBody) {
        HttpURLConnection connection = null;

        try {
            //Create connection
            URL url = new URL(commandURL);
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", MediaType.APPLICATION_JSON);
            connection.setDoOutput(true);

            //Send request
            DataOutputStream writer = new DataOutputStream(connection.getOutputStream());
            writer.write(commandBody.getBytes(StandardCharsets.UTF_8));
            writer.close();

            //Get Response
            DataInputStream reader = new DataInputStream(connection.getInputStream());
            byte[] bytes = new byte[65536];
            while (reader.read(bytes) < 0) {
                Thread.sleep(50);
            }
            reader.close();

            return new String(bytes, StandardCharsets.UTF_8);

        } catch (Exception e) {
            e.printStackTrace();
            return null;
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    public static ServiceStub newTestService(final String serviceName, final int numEndpoints, final int regPort) throws Exception {
        ServiceStub testService = new ServiceStub(
                serviceName,
                numEndpoints,
                ImmutableMap.of(
                        "cmd1",
                        new Command(
                                "cmd1", "Command for service tests",
                                ImmutableList.of(
                                        new CommandParameter("arg1", "str", "The first argument of the command"),
                                        new CommandParameter("arg2", "str", "The second argument of the command")
                                )
                        ),
                        "cmd2",
                        new Command(
                                "cmd2", "Command for service tests",
                                ImmutableList.of(
                                        new CommandParameter("arg1", "str", "The first argument of the command"),
                                        new CommandParameter("arg2", "str", "The second argument of the command")
                                )
                        )
                )
        );
        testService.startService();
        Thread.sleep(100); // TODO: Remove this sleep
        logger.info("Test MainService {} started", serviceName);

        // Register the test service endpoints
        logger.info("Registering endpoints for service {}", serviceName);
        testService.registerEndpoints("127.0.0.1", regPort);

        return testService;
    }
}
