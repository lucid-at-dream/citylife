package gateway.service.commands;

import java.util.Map;

public class ServiceEndpoint {

    private String service;

    private String address;

    private int port;

    private Map<String, Command> commands;

    public ServiceEndpoint() {}

    public ServiceEndpoint(final String service, final String address, final int port, final Map<String, Command> commands) {
        this.service = service;
        this.address = address;
        this.port = port;
        this.commands = commands;
    }

    public String getService() {
        return service;
    }

    public String getAddress() {
        return address;
    }

    public int getPort() {
        return this.port;
    }

    public Map<String, Command> getCommands() {
        return this.commands;
    }

    public void setService(String service) {
        this.service = service;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public void setCommands(Map<String, Command> commands) {
        this.commands = commands;
    }
}
