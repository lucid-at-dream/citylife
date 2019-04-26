package gateway.service.gateway.api;

import gateway.service.commands.Registry;
import gateway.service.commands.Service;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.*;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import java.io.IOException;

@Path("/api")
public class GatewayServer {

    private static final Logger logger = LoggerFactory.getLogger(GatewayServer.class);

    @Context private HttpServletRequest httpRequest;

    @POST
    @Path("/{serviceName}/{commandName}")
    @Consumes(MediaType.APPLICATION_JSON)
    @Produces(MediaType.APPLICATION_JSON)
    public String forwardPostRequest(@PathParam("serviceName") String serviceName, @PathParam("commandName") String commandName, final String request) throws IOException {

        Registry serviceRegistry = (Registry)httpRequest.getServletContext().getAttribute("serviceRegistry");

        logger.debug("POST Request received for command {} in service {}", commandName, serviceName);
        logger.info(request);

        Service service = serviceRegistry.getService(serviceName);
        return service.executeCommand(request);
    }
}
