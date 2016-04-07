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

    public void loadServicesFromConf(String cfgfile){
        Config config = new Config(cfgfile);
        ArrayList<ServiceCfg> servicesConfig = config.parseConfig();

        for( ServiceCfg scfg: servicesConfig ){
            registerService(scfg.name);
            for( String dependency : scfg.dependencies )
                registerDependency(scfg.name, dependency);
        }
    }

    public void logAllStatus(){
        for(ServiceWrapper sw : servicesList){
            String state = getServiceStateStr( sw.getServiceState() );
            System.out.println(sw.isRunning() + ":" + sw.isStopped() + " | ["+state+"]  " + sw.name);
        }
    }

    public void logServiceStatus(String name){
        if( services.containsKey( name ) ){
            ServiceWrapper sw = services.get(name);
            String state = getServiceStateStr( sw.getServiceState() );
            System.out.println("["+state+"]  " + sw.name);
        }
    }    

    public String getServiceStateStr(ServiceState state){
        switch(state){
            case WAITING_DEPENDENCIES_START:
                return "WAITING_DEPENDENCIES_START";
            case STARTING:
                return "STARTING";
            case RUNNING:
                return "RUNNING";
            case WAITING_DEPENDENCIES_STOP:
                return "WAITING_DEPENDENCIES_STOP";
            case STOPPING:
                return "STOPPING";
            case STOPPED:
                return "STOPPED";
        }
        return "omg - IT'S THE BIT FLIP!! stop messing around stars";
    }

    public void startService(String serviceName){
        startService( services.get(serviceName) );
    }

    public void startService(ServiceWrapper wrapper){
        for( ServiceWrapper dependency : wrapper.dependencies )
            this.startService(dependency);

        wrapper.addStartServiceJob();
        synchronized( wrapper.pendingJobs ){
            if( wrapper.isProcessingJobs )
                return;
            else
                wrapper.isProcessingJobs = true;
        }
        pool.execute(wrapper);
    }

    public void stopService(String serviceName){
        stopService( services.get(serviceName) );
    }

    public void stopService(ServiceWrapper wrapper){

        for( ServiceWrapper dep : wrapper.dependents )
            this.stopService(dep);

        wrapper.addStopServiceJob();
        synchronized( wrapper.pendingJobs ){
            if( wrapper.isProcessingJobs )
                return;
            else
                wrapper.isProcessingJobs = true;
        }
        pool.execute( wrapper );
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
