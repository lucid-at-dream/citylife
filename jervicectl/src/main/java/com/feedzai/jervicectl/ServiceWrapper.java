package com.feedzai.jervicectl;

import java.util.ArrayList;

public class ServiceWrapper implements Runnable{
    
    private Service service;
    private ArrayList<String> dependents;
    private int nDeps, nRunningDeps;
    private ServiceState state;
    public Boolean stopjob;

    public ServiceWrapper(Service service){
        this.service = service;
        this.nDeps = this.nRunningDeps = 0;
        this.state = ServiceState.STOPPED;
        this.stopjob = false;
        this.dependents = new ArrayList();
    }

    public void addDependent(String service){
        dependents.add(service);
    }

    public void setNumDependencies(int n){
        this.nDeps = n;
    }

    public boolean canStart(){
        return nDeps - nRunningDeps == 0;
    }

    public void incRunningDeps(int inc){
        this.nRunningDeps -= 1;
        //if(==0) notify someone
    }

    private void tellDependentsStopped(){
        for(String sw : dependents){
            //sw.incRunningDeps(-1);
        }
    }

    private void tellDependentsStarted(){
        for(String sw : dependents){
            //sw.incRunningDeps(1);
        }
    }

    public void run(){
        boolean success = service.start();

        if( !success )
            return;

        this.tellDependentsStarted();
        this.state = ServiceState.RUNNING;

        while( !stopjob ){
            synchronized( stopjob ){
                try{
                    stopjob.wait();
                }catch(InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
        
        this.tellDependentsStopped();
        service.stop();
    }
}
