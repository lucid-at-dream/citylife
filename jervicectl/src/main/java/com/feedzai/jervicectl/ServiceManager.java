package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;

public class ServiceManager{

    private HashMap<String, ServiceWrapper> services;
    private ArrayList<ServiceWrapper> servicesList;

    public ServiceManager(){
        this.services = new HashMap();
        this.servicesList = new ArrayList();
    }

    public void startService(ServiceWrapper wrapper){
        Thread t = new Thread(wrapper);
        wrapper.thread = t;
        t.start();
    }

    public void startAll(){
        for( ServiceWrapper sw : servicesList ){
            this.startService(sw);
        }
    }

    public void stopAll(){
    }

    public ServiceWrapper registerService(String serviceName){
        ServiceWrapper s = null;
        if( services.containsKey(serviceName) ){
            s = services.get(serviceName);
        }else{
            s = new ServiceWrapper( ServiceCfg.instantiateService(serviceName) );
            services.put(serviceName, s);
            servicesList.add(s);
        }
        return s;
    }

    public void registerDependent(String serviceName, String dependentName){
        ServiceWrapper dependent = registerService(dependentName);
        ServiceWrapper service = registerService(serviceName);
        service.addDependent(dependent);
    }

}
