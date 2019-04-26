package gateway.service.testservices;

public class ResponseStub {
    private String service;
    private int port;
    private String request;

    public int getPort() {
        return port;
    }

    public String getService() {
        return service;
    }

    public String getRequest() {
        return request;
    }

    public void setService(String service) {
        this.service = service;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public void setRequest(String request) {
        this.request = request;
    }
}
