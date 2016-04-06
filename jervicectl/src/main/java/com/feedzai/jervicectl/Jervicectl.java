package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;

public class Jervicectl{
    private HashMap<String, ServiceWrapper> services;
    private ArrayList<ServiceWrapper> servicesList;

    public static void main( String[] args )
    {
        Jervicectl mgr = new Jervicectl();
        mgr.init();
        mgr.startAll();
    }

    public Jervicectl(){
        this.services = new HashMap();
        this.servicesList = new ArrayList();
    }

    private void loadService(ServiceCfg cfg){
        ServiceWrapper s = null;
        if( services.containsKey(cfg.name) ){
            s = services.get(cfg.name);
        }else{
            s = new ServiceWrapper( ServiceCfg.instantiateService(cfg.name) );
            services.put(cfg.name, s);
            servicesList.add(s);
        }
        s.setNumDependencies(cfg.dependencies.size());

        for( String dependency : cfg.dependencies ){
            ServiceWrapper dep = null;
            if( services.containsKey(dependency) ){
                dep = services.get(dependency);
            }else{
                dep = new ServiceWrapper( ServiceCfg.instantiateService(dependency) );
                services.put(dependency, dep);
                servicesList.add(dep);
            }
            dep.addDependent(cfg.name);
        }
    }

    public void init(){
        Config config = new Config("jervicectl.cfg");
        ArrayList<ServiceCfg> servicesConfig = config.parseConfig();

        for( ServiceCfg scfg: servicesConfig ){
            this.loadService(scfg);
        }
    }

    public void loadConfig(){

    }

    public void startService(){

    }

    public void stopService(){

    }

    public void getServiceStatus(){

    }

    public void startAll(){
        for(ServiceWrapper sw : servicesList){
            Thread t = new Thread(sw);
            t.start();
        }
    }

    public void stopAll(){

    }
}
