package com.feedzai.jervicectl;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ServiceWrapper implements Runnable{
    
    public String name;
    private Service service;
    
    public ArrayList<ServiceWrapper> dependencies;
    public ArrayList<ServiceWrapper> dependents;
    
    private volatile ServiceState serviceState;
    private volatile boolean stop;

    private Boolean jobsMonitor;
    public ConcurrentLinkedQueue<Runnable> pendingJobs;

    public ServiceWrapper(String name, Service service){
        this.name = name;
        this.service = service;
        
        this.setServiceState(ServiceState.STOPPED);
        this.stop = false;

        this.dependencies = new ArrayList();
        this.dependents = new ArrayList();

        this.jobsMonitor = new Boolean(true);
        this.pendingJobs = new ConcurrentLinkedQueue();
    }

    /**
    * waits for new jobs in the pendingJobs queue and processes them as soon as possible
    */
    public void run(){
        for(;!stop;){
            if( hasPendingJobs() )
                getNextJob().run();

            synchronized(this.jobsMonitor){
                while( !hasPendingJobs() && !stop ){
                    try{
                        this.jobsMonitor.wait();
                    }catch(InterruptedException e){
                        return; /*if interrupted finish*/
                    }
                }
            }
        }
    }

    /**
    * Signals the service control thread to stop */
    public void stopControlThread(){
        synchronized(this.jobsMonitor){
            this.stop = true;
            jobsMonitor.notify();
        }
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
    * @return returns true if the service has pending jobs on queue.*/
    public boolean hasPendingJobs(){
        return !pendingJobs.isEmpty();
    }

    /**
    * @return returns the next job on queue and removes it.*/
    public Runnable getNextJob(){
        return pendingJobs.poll();
    }

    /**
    * Adds a job for service starting to the job queue */
    public void addStartServiceJob(){
        pendingJobs.add( new Runnable() {
            @Override
            public void run(){
                if( hasBeenStarted() ){
                    return;
                }
                askDependenciesToStart();
                waitForDependenciesToStart();
                startServiceThread();
                notifyDependents();
            }
        });
        synchronized(this.jobsMonitor){
            jobsMonitor.notify();
        }
    }

    /**
    * Adds a job for service stopping to the job queue */
    public void addStopServiceJob(){
        pendingJobs.add( new Runnable() {
            @Override
            public void run(){
                if( hasBeenStopped() ){
                    return;
                }
                askDependentsToStop();
                waitForDependentsToStop();
                stopServiceThread();
                notifyDependencies();
            }
        });
        synchronized(this.jobsMonitor){
            jobsMonitor.notify();
        }
    }

    /**
    * Adds start service jobs to all dependencies */
    public void askDependenciesToStart(){
        for( ServiceWrapper dependency : dependencies )
            dependency.addStartServiceJob();
    }

    /**
    * Adds stop service jobs to all dependent services */
    public void askDependentsToStop(){
        for( ServiceWrapper dependency : dependents )
            dependency.addStopServiceJob();
    }

    /**
    * returns when all dependencies have started */
    public void waitForDependenciesToStart(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_START);
        synchronized(this){
            while( !this.canStart() ){
                try{
                    this.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }

    /**
    * returns when all dependent services have stopped */
    public void waitForDependentsToStop(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_STOP);
        synchronized( this ){
            while( !this.canStop() ){
                try{
                    this.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }

    /**
    * executes the start method of the service object */
    public void startServiceThread(){
        this.setServiceState(ServiceState.STARTING);
        service.start();
        this.setServiceState(ServiceState.RUNNING);
    }

    /**
    * executes the stop method of the service object */
    public void stopServiceThread(){
        this.setServiceState(ServiceState.STOPPING);
        service.stop();
        this.setServiceState(ServiceState.STOPPED);
    }

    /**
    * notifies all threads waiting on dependent services objects */
    public void notifyDependents(){
        for( ServiceWrapper dependent : dependents ){
            synchronized( dependent ){
                dependent.notify();
            }
        }
    }

    /**
    * notifies all threads waiting on dependency services objects */
    public void notifyDependencies(){
        for( ServiceWrapper dependency : this.dependencies ){
            synchronized(dependency){
                dependency.notify();
            }
        }
    }

}
