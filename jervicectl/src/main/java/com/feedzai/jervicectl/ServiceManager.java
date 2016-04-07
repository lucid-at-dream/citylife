package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

public class ServiceManager{

    private HashMap<String, ServiceWrapper> services;
    private ArrayList<ServiceWrapper> servicesList;
    private ExecutorService pool;

    public ServiceManager(){
        this.services = new HashMap();
        this.servicesList = new ArrayList();
        pool = Executors.newCachedThreadPool();
    }

    public void startService(ServiceWrapper wrapper){
        if( wrapper.hasBeenStarted() )
            return;
        wrapper.setServiceState(ServiceState.WAITING_DEPENDENCIES_START);

        if( !wrapper.canStart() )
            for( ServiceWrapper dependency : wrapper.dependencies )
                this.startService(dependency);

        pool.execute( wrapper.getStartServiceJob() );
    }

    public void stopService(ServiceWrapper wrapper){
        if( !wrapper.hasBeenStarted() )
            return;
        wrapper.setServiceState(ServiceState.WAITING_DEPENDENCIES_STOP);

        if( !wrapper.canStop() )
            for( ServiceWrapper dep : wrapper.dependents )
                this.stopService(dep);

        pool.execute( wrapper.getStopServiceJob() );
    }

    public void startAll(){
        for( ServiceWrapper sw : servicesList )
            this.startService(sw);
    }

    public void stopAll(){
        for( ServiceWrapper sw : servicesList )
            this.stopService(sw);
    }

    public ServiceWrapper registerService(String serviceName){
        ServiceWrapper s = null;
        if( services.containsKey(serviceName) ){
            s = services.get(serviceName);
        }else{
            s = new ServiceWrapper( serviceName, ServiceCfg.instantiateService(serviceName) );
            services.put(serviceName, s);
            servicesList.add(s);
        }
        return s;
    }

    public void registerDependency(String serviceName, String dependencyName){
        ServiceWrapper dependency = registerService(dependencyName);
        ServiceWrapper service = registerService(serviceName);
        service.addDependency(dependency);
    }

}
