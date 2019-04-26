package gateway.service.commands;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class Registry {

    private static final Logger logger = LoggerFactory.getLogger(Registry.class);

    private ConcurrentMap<String, Service> registry;

    public Registry() {
        registry = new ConcurrentHashMap<>();
    }

    synchronized public void registerEndpoint(final ServiceEndpoint endpoint) {

        logger.info("Registering endpoint for service {} at {}:{}", endpoint.getService(), endpoint.getAddress(), endpoint.getPort());

        Service service;
        if (!this.registry.containsKey(endpoint.getService())) {
            service = new Service(endpoint.getService());
            this.registry.put(service.getName(), service);
        } else {
            service = this.registry.get(endpoint.getService());
        }
        service.addEndpoint(endpoint);
    }

    public Service getService(final String name) {
        return this.registry.get(name);
    }
}
