package com.feedzai.jervicectl;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ServiceWrapper implements Runnable{
    
    public String name;
    private Service service;
    
    public ArrayList<ServiceWrapper> dependencies;
    public ArrayList<ServiceWrapper> dependents;
    
    private volatile ServiceState serviceState;

    public volatile boolean isProcessingJobs;
    public ConcurrentLinkedQueue<Runnable> pendingJobs;

    public ServiceWrapper(String name, Service service){
        this.name = name;
        this.service = service;
        
        this.setServiceState(ServiceState.STOPPED);

        this.dependencies = new ArrayList();
        this.dependents = new ArrayList();

        this.isProcessingJobs = false;
        this.pendingJobs = new ConcurrentLinkedQueue();
    }

    public void run(){
        for(;;){
            getNextJob().run();

            synchronized(this.pendingJobs){
                if( !hasPendingJobs() ){
                    isProcessingJobs = false;
                    break;
                }
            }
        }
    }

    public boolean isRunning(){
        return this.serviceState == ServiceState.RUNNING ||
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_STOP;
    }

    public boolean isStopped(){
        return this.serviceState == ServiceState.STOPPED || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_START;
    }

    public boolean hasBeenStarted(){
        return this.serviceState == ServiceState.RUNNING || 
               this.serviceState == ServiceState.STARTING || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_START;
    }

    public boolean hasBeenStopped(){
        return this.serviceState == ServiceState.STOPPED || 
               this.serviceState == ServiceState.STOPPING || 
               this.serviceState == ServiceState.WAITING_DEPENDENCIES_STOP;
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
        for( ServiceWrapper sw : this.dependencies )
            if( !sw.isRunning() )
                return false;
        return true;
    }

    public boolean canStop(){
        for( ServiceWrapper sw : this.dependents )
            if( !sw.isStopped() )
                return false;
        return true;
    }

    public boolean hasPendingJobs(){
        return !pendingJobs.isEmpty();
    }

    public Runnable getNextJob(){
        return pendingJobs.poll();
    }

    public void addStartServiceJob(){
        pendingJobs.add( new Runnable() {
            @Override
            public void run(){
                if( hasBeenStarted() ){
                    return;
                }
                waitForDependenciesToStart();
                startServiceThread();
                notifyDependents();
            }
        });
    }

    public void addStopServiceJob(){
        pendingJobs.add( new Runnable() {
            @Override
            public void run(){
                if( hasBeenStopped() ){
                    return;
                }
                waitForDependentsToStop();
                stopServiceThread();
                notifyDependencies();
            }
        });
    }

    public void waitForDependenciesToStart(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_START);
        synchronized(this){
            while( !this.canStart() ){
                System.out.println(name + " waiting for deps to start");
                try{
                    System.out.println("waiting on this: " + this);
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
                dependent.notify();
            }
        }
    }

    public void waitForDependentsToStop(){
        this.setServiceState(ServiceState.WAITING_DEPENDENCIES_STOP);
        synchronized( this ){
            while( !this.canStop() ){
                System.out.println(name + " waiting for deps to stop");
                try{
                    System.out.println("waiting on this: " + this);
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
