package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;

public class Jervicectl{
    private HashMap<String, ServiceWrapper> services;

    public static void main( String[] args )
    {
        Jervicectl mgr = new Jervicectl();
        mgr.init();

    }

    public Jervicectl(){
        this.services = new HashMap();
    }

    private void loadService(ServiceCfg cfg){
        
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

    }

    public void stopAll(){

    }
}
