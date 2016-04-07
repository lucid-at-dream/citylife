package com.feedzai.jervicectl;

import java.util.ArrayList;

public class ServiceWrapper{
    
    private Service service;
    
    public ArrayList<ServiceWrapper> dependencies;
    public ArrayList<ServiceWrapper> dependents;

    public int numDependencies;
    public int numRunningDependencies;
    
    public String name;

    private volatile ServiceState serviceState;
    private Boolean stopjob;

    public ServiceWrapper(String name, Service service){
        this.name = name;
        this.service = service;
        
        this.numRunningDependencies = 0;
        this.setServiceState(ServiceState.STOPPED);
        this.stopjob = false;
        
        this.dependencies = new ArrayList();
        this.dependents = new ArrayList();
    }

    public boolean hasBeenStarted(){
        return this.serviceState == ServiceState.RUNNING || 
               this.serviceState == ServiceState.STARTING || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_START;
    }

    public void setServiceState(ServiceState state){
        this.serviceState = state;
    }

    public ServiceState getServiceState(){
        return this.serviceState;
    }

    public void addDependency(ServiceWrapper service){
        this.dependencies.add(service);
        service.dependents.add(this);
    }

    public boolean canStart(){
        return (this.dependencies.size() - numRunningDependencies) == 0;
    }

    public boolean canStop(){
        for( ServiceWrapper sw : this.dependents )
            if( sw.getServiceState() != ServiceState.STOPPED )
                return false;
        return true;
    }





    public Runnable getStartServiceJob(){
        return new Runnable() {
            @Override
            public void run(){
                waitForDependenciesToStart();
                startServiceThread();
                notifyDependents();
            }
        };
    }

    public Runnable getStopServiceJob(){
        return new Runnable() {
            @Override
            public void run(){
                waitForDependentsToStop();
                stopServiceThread();
                notifyDependencies();
            }
        };
    }

    public void waitForDependenciesToStart(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_START);
        synchronized(this){
            while( !this.canStart() && !this.stopjob ){
                try{
                    this.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }

    public void startServiceThread(){
        this.setServiceState(ServiceState.STARTING);
        service.start();
        this.setServiceState(ServiceState.RUNNING);
    }

    public void notifyDependents(){
        for( ServiceWrapper dependent : dependents ){
            synchronized( dependent ){
                dependent.numRunningDependencies++;
                dependent.notify();
            }
        }
    }

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

    public void stopServiceThread(){
        this.setServiceState(ServiceState.STOPPING);
        service.stop();
        this.setServiceState(ServiceState.STOPPED);
    }

    public void notifyDependencies(){
        for( ServiceWrapper dependency : this.dependencies ){
            synchronized(dependency){
                dependency.notify();
            }
        }
    }

}
