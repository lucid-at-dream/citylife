package com.feedzai.jervicectl;

import java.util.ArrayList;

public class ServiceWrapper implements Runnable{
    
    private Service service;
    
    private ArrayList<ServiceWrapper> dependencies;
    private ArrayList<ServiceWrapper> dependents;

    public int numDependencies;
    public int numRunningDependencies;
    
    private ServiceState state;
    private Boolean stopjob;
    public Thread thread;

    public ServiceWrapper(Service service){
        this.service = service;
        this.numDependencies = this.numRunningDependencies = 0;
        this.state = ServiceState.STOPPED;
        this.stopjob = false;
        this.dependents = new ArrayList();
    }

    public ServiceState getState(){
        return this.state;
    }

    public void addDependent(ServiceWrapper service){
        dependents.add(service);
        service.numDependencies++;
    }

    public void setNumDependencies(int n){
        this.numDependencies = n;
    }

    public boolean canStart(){
        return (numDependencies - numRunningDependencies) == 0;
    }

    public void run(){

        while( !this.canStart() ){
            synchronized( this ){
                try{
                    this.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }

        this.state = ServiceState.STARTING;
        boolean success = service.start();

        if( !success )
            return;

        this.state = ServiceState.RUNNING;
        for( ServiceWrapper dependent : dependents ){
            
            synchronized( dependent ){
                dependent.numRunningDependencies++;
                dependent.notify();
            }

        }

        while( !stopjob ){
            synchronized( stopjob ){
                try{
                    stopjob.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
        
        for( ServiceWrapper dependent : dependents ){
            //dependent.stop()
            dependent.numRunningDependencies--;
        }
        this.state = ServiceState.STOPPED;
        service.stop();

    }
}
