package com.feedzai.jervicectl;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ServiceWrapper implements Runnable{
    
    private String name;
    private Service service;
    
    private ArrayList<ServiceWrapper> dependencies;
    private ArrayList<ServiceWrapper> dependents;
    
    private volatile ServiceState serviceState;
    private volatile boolean stop, softstop;

    private Boolean targetMonitor;
    private volatile ServiceState targetState;

    public ServiceWrapper(String name, Service service){
        this.name = name;
        this.service = service;
        
        this.setServiceState(ServiceState.STOPPED);
        this.stop = false;
        this.softstop = false;

        this.dependencies = new ArrayList();
        this.dependents = new ArrayList();

        this.targetMonitor = new Boolean(true);
        this.targetState = ServiceState.STOPPED;
    }

    /**
    * @return the name of this service */
    public String getName(){
        return this.name;
    }

    /**
    * @return a list of services that this service depends on */
    public ArrayList<ServiceWrapper> getDependencies(){
        return this.dependencies;
    }

    /**
    * @return a list of services that depend on this one */
    public ArrayList<ServiceWrapper> getDependents(){
        return this.dependents;
    }

    /**
    * @return the current target state */
    public ServiceState getTargetState(){
        return this.targetState;
    }

    /**
    * waits for new jobs in the pendingJobs queue and processes them as soon as possible
    */
    public void run(){
        while(!stop){

            while( targetState != serviceState )
                getNextJob().run();

            if( softstop )
                break;

            synchronized(this.targetMonitor){
                while( targetState == serviceState && !stop && !softstop ){
                    try{
                        this.targetMonitor.wait();
                    }catch(InterruptedException e){
                        return; /*if interrupted finish*/
                    }
                }
            }
        }
    }

    /**
    * Changes the target state of this service and signals the controlling thread */
    public void setTargetState(ServiceState state){
        synchronized( this.targetMonitor ){
            this.targetState = state;
            this.targetMonitor.notify();
        }
    }

    /**
    * Notifies the thread waiting on target changes */
    public void notifyTarget(){
        synchronized(this.targetMonitor){
            this.targetMonitor.notify();
        }
    }

    /**
    * Signals the service control thread to stop */
    public void stopControlThread(){
        synchronized(this.targetMonitor){
            this.stop = true;
            targetMonitor.notify();
        }
    }

    /**
    * Stops the control thread whenever target state equals the current state
    */
    public void stopControlThreadWhenDone(){
        synchronized(this.targetMonitor){
            this.softstop = true;
            targetMonitor.notify();
        }
    }

    /**
    * @param the service to be testes as dependency
    * @return true if this service depends on the argument service*/
    public boolean dependsOn(ServiceWrapper other){
        return this.dependencies.contains( other );
    }

    /**
    * @param the service to be testes as dependent
    * @return true if this service is dependency of the argument service*/
    public boolean isDependencyOf(ServiceWrapper other){
        return this.dependents.contains( other );
    }

    /**
    * Returns true if the service is running, even if it wants to stop but is still waiting for dependencies.
    *
    * @return true if service is running, false otherwise. */
    public boolean isRunning(){
        return this.serviceState == ServiceState.RUNNING ||
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_STOP;
    }

    /**
    * Returns true if the service is stopped, even if it wants to start but is still waiting for dependencies.
    *
    * @return true if service is stopped, false otherwise. */
    public boolean isStopped(){
        return this.serviceState == ServiceState.STOPPED || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_START;
    }

    /**
    * @return true if the service wants to be in a running state, false otherwise */
    public boolean hasBeenStarted(){
        return this.serviceState == ServiceState.RUNNING || 
               this.serviceState == ServiceState.STARTING || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_START;
    }

    /**
    * @return true if the service wants to be in a stopped state, false otherwise */
    public boolean hasBeenStopped(){
        return this.serviceState == ServiceState.STOPPED || 
               this.serviceState == ServiceState.STOPPING || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_STOP;
    }

    /**
    * @param the wanted service state for this object */
    public void setServiceState(ServiceState state){
        this.serviceState = state;
    }

    /**
    * @return the current service state of this object */
    public ServiceState getServiceState(){
        return this.serviceState;
    }

    /**
    * adds param service as a dependency of this service.
    * adds this service as a dependent of param service.
    *
    * @param the service this service is dependent on.
    */
    public void addDependency(ServiceWrapper service){
        this.dependencies.add(service);
        service.dependents.add(this);
    }

    /**
    * @return true if all dependencies are running, false otherwise */
    public boolean canStart(){
        for( ServiceWrapper sw : this.dependencies )
            if( !sw.isRunning() )
                return false;
        return true;
    }

    /**
    * @return true if all dependents are stopped, false otherwise */
    public boolean canStop(){
        for( ServiceWrapper sw : this.dependents )
            if( !sw.isStopped() )
                return false;
        return true;
    }

    /**
    * @return the service associated with this wrapper */
    public Service getService(){
        return this.service;
    }

    /**
    * @return returns the next job on queue and removes it.*/
    public Runnable getNextJob(){
        if( targetState != serviceState && targetState == ServiceState.RUNNING )
            return getStartServiceJob();

        if( targetState != serviceState && targetState == ServiceState.STOPPED )
            return getStopServiceJob();
        return null;
    }

    /**
    * Adds a job for service starting to the job queue */
    public Runnable getStartServiceJob(){
        return new Runnable() {
            @Override
            public void run(){
                if( waitForDependenciesToStart() ){            
                    startServiceThread();
                    notifyDependents();
                }else{
                    setServiceState(ServiceState.STOPPED);
                }
            }
        };
    }

    /**
    * Adds a job for service stopping to the job queue */
    public Runnable getStopServiceJob(){
        return new Runnable() {
            @Override
            public void run(){
                if( waitForDependentsToStop() ){
                    stopServiceThread();
                    notifyDependencies();
                }else{
                    setServiceState(ServiceState.RUNNING);
                }
            }
        };
    }

    /**
    * returns when all dependencies have started 
    * @return returns true if all dependencies started, false if target changed */
    public boolean waitForDependenciesToStart(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_START);
        synchronized(this.targetMonitor){
            while( !this.canStart() && targetState == ServiceState.RUNNING ){
                try{
                    this.targetMonitor.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
        return this.canStart() && targetState == ServiceState.RUNNING;
    }

    /**
    * returns when all dependent services have stopped 
    * @return returns true if all dependents stopped, false if target changed */
    public boolean waitForDependentsToStop(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_STOP);
        synchronized( this.targetMonitor ){
            while( !this.canStop() && targetState == ServiceState.STOPPED ){
                try{
                    this.targetMonitor.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
        return this.canStop() && targetState == ServiceState.STOPPED;
    }

    /**
    * executes the start method of the service object 
    * @return true if no problems arise, false if an uncaught exception happens in start() */
    public boolean startServiceThread(){
        this.setServiceState(ServiceState.STARTING);
        try{
            service.start();
        }catch(Exception e){
            this.setServiceState(ServiceState.STOPPED);
            return false;
        }
        this.setServiceState(ServiceState.RUNNING);
        return true;
    }

    /**
    * executes the stop method of the service object
    * @return true if no problems arise, false if an uncaught exception happens in stop() */
    public boolean stopServiceThread(){
        this.setServiceState(ServiceState.STOPPING);
        try{
            service.stop();
        }catch(Exception e){
            this.setServiceState(ServiceState.RUNNING);
            return false;
        }
        this.setServiceState(ServiceState.STOPPED);
        return true;
    }

    /**
    * notifies all threads waiting on dependent services objects */
    public void notifyDependents(){
        for( ServiceWrapper dependent : dependents ){
            dependent.notifyTarget();
        }
    }

    /**
    * notifies all threads waiting on dependency services objects */
    public void notifyDependencies(){
        for( ServiceWrapper dependency : this.dependencies ){
            dependency.notifyTarget();
        }
    }

}
