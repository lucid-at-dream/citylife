package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;

public class Jervicectl{
    private ServiceManager serviceMgr;
    private ArrayList<ServiceWrapper> servicesList;

    public static void main( String[] args )
    {
        String cfgname = args.length > 0 ? args[0] : "jervicectl.cfg";

        Jervicectl jervicectl = new Jervicectl();
        jervicectl.init(cfgname);
        jervicectl.startAll();
    }

    public Jervicectl(){
        this.serviceMgr = new ServiceManager();
    }

    public void init(String cfgname){
        Config config = new Config(cfgname);
        ArrayList<ServiceCfg> servicesConfig = config.parseConfig();

        for( ServiceCfg scfg: servicesConfig ){
            this.serviceMgr.registerService(scfg.name);
            for( String dependency : scfg.dependencies )
                this.serviceMgr.registerDependent(dependency, scfg.name);
        }
    }

    public boolean startService(String name){
        //serviceMgr.startService(name);
        return true;
    }

    public void stopService(String name){
        //serviceMgr.stopService(name);
    }

    public ServiceState getServiceStatus(String name){
        //return serviceMgr.getService(name).getState();
        return ServiceState.STOPPED;
    }

    public void startAll(){
        serviceMgr.startAll();
    }

    public void stopAll(){
        serviceMgr.stopAll();
    }
}
