package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;

public class ServiceManager{

    private HashMap<String, ServiceWrapper> services;
    private ArrayList<ServiceWrapper> servicesList;
    private ExecutorService pool;

    public ServiceManager(){
        this.services = new HashMap();
        this.servicesList = new ArrayList();
        pool = Executors.newCachedThreadPool();
    }

    /**
    *@return the number of registered services*/
    public int getNumRegisteredServices(){
        return servicesList.size();
    }

    /**
    * loads the services described in the configuration file passed as parameter
    *
    * @param cfgfile path to configuration file  */
    public void loadServicesFromConf(String cfgfile) throws NoSuchServiceException, NoSuchConfigFileException{
        Config config = new Config(cfgfile);
        ArrayList<ServiceCfg> servicesConfig = config.parseConfig();

        for( ServiceCfg scfg: servicesConfig ){
            registerService(scfg.name);
            for( String dependency : scfg.dependencies )
                registerDependency(scfg.name, dependency);
        }
    }

    /**
    * Given a service name attempts to start it.
    *
    * @param serviceName the name of the service to start 
    * @throws NoSuchServiceException when the service name hasn't been registered. */
    public void startService(String serviceName) throws NoSuchServiceException{
        if( !services.containsKey(serviceName) )
            throw new NoSuchServiceException("Service " + serviceName + " is unknown");
        startService( services.get(serviceName) );
    }

    /**
    * Given a service attempts to start it.
    *
    * @param wrapper the name of the service to start
    * @throws NoSuchServiceException when the service name hasn't been registered. */
    public void startService(ServiceWrapper wrapper){
        for( ServiceWrapper dependency : wrapper.getDependencies() )
            startService(dependency);
        wrapper.setTargetState(ServiceState.RUNNING);
    }

    /**
    * Given a service name attempts to stop it.
    *
    * @param serviceName the name of the service to stop
    * @throws NoSuchServiceException when the service name hasn't been registered. */
    public void stopService(String serviceName) throws NoSuchServiceException{
        if( !services.containsKey(serviceName) )
            throw new NoSuchServiceException("Service " + serviceName + " is unknown");
        stopService( services.get(serviceName) );
    }

    /**
    * Given a service attempts to stop it.
    *
    * @param wrapper the name of the service to stop */
    public void stopService(ServiceWrapper wrapper){
        for( ServiceWrapper dependent : wrapper.getDependents() )
            stopService(dependent);
        wrapper.setTargetState(ServiceState.STOPPED);
    }

    /**
    * outputs the service status of the parameter service to the stdout 
    *
    * @param name the name of the service */
    public void logServiceStatus(String name) throws NoSuchServiceException{
        if( services.containsKey( name ) ){
            ServiceWrapper sw = services.get(name);
            logServiceStatus(sw);
        }else{
            throw new NoSuchServiceException("Service " + name + " is unknown");
        }
    }    

    /**
    * outputs the service status of the parameter service to the stdout 
    *
    * @param service the service */
    public void logServiceStatus(ServiceWrapper service){
        String state = getServiceStateStr( service.getServiceState() );
        String target = getServiceStateStr( service.getTargetState() );
        System.out.println("["+state+" -> " + target + "]  " + service.getName());
    }

    /**
    * starts all known services */
    public void startAll(){
        for( ServiceWrapper sw : servicesList )
            this.startService(sw);
    }

    /**
    * stops all known services */
    public void stopAll(){
        for( ServiceWrapper sw : servicesList )
            this.stopService(sw);
    }

    /**
    * outputs the service status of all known services to the stdout */
    public void logAllStatus(){
        for(ServiceWrapper sw : servicesList)
            logServiceStatus(sw);
    }

    /**
    * returns the service corresponding to the argument service name in case it is known.
    * @param serviceName the service name
    * @return the corresponding service wrapper (possibly null) */
    public ServiceWrapper getService(String serviceName) throws NoSuchServiceException{
        if( services.containsKey(serviceName) )
            return services.get(serviceName);
        throw new NoSuchServiceException("Service " + serviceName + " is unknown");
    }

    /**
    * if the reported service is unknown creates a new service.
    * 
    * @param serviceName the name of the service to register
    * @return the newly created or existing service */
    public ServiceWrapper registerService(String serviceName) throws NoSuchServiceException{
        ServiceWrapper wrapper = null;
        if( services.containsKey(serviceName) ){
            wrapper = services.get(serviceName);
        }else{
            Service service = new ServiceCfg(serviceName, null).instantiateService();
            wrapper = new ServiceWrapper( serviceName, service );
            services.put(serviceName, wrapper);
            servicesList.add(wrapper);
            pool.execute(wrapper);
        }
        return wrapper;
    }

    /**
    * *warning* testing only
    * waits for all jobs to finish and returns then.
    *
    * @return true if successful clean termination, false if timed out */
    public boolean waitForJobsAndStopAllThreads(int timeout){
        
        for( ServiceWrapper sw : this.servicesList ){
            sw.stopControlThreadWhenDone();
        }

        pool.shutdown();
        try{
            pool.awaitTermination(timeout, TimeUnit.MILLISECONDS);
        }catch(InterruptedException e){
            return false;
        }
        return true;
    }

    /**
    * adds the service corresponding to dependencyName as a dependency of serviceName. 
    * Note: if either service is not previously known, it is created and loaded. 
    *
    * @param serviceName the name of the service in which to add the dependency
    * @param dependencyName the name of the service to be added as a dependency */
    public void registerDependency(String serviceName, String dependencyName) throws NoSuchServiceException{
        ServiceWrapper dependency = registerService(dependencyName);
        ServiceWrapper service = registerService(serviceName);

        if( dependency != null && service != null )
            service.addDependency(dependency);
    }



    /**
    * Given a service state returns a descriptive string
    *
    * @param state the state that is going to be translated to text
    * @return a descriptive of the argument state */
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
        return "Warning :: Fatal :: BIT FLIP!! - - - - TONIGHT WE PARTY WITH THE STARS. BAAAAM!";
    }
}
