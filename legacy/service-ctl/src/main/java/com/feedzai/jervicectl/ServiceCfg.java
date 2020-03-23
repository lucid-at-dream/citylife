package com.feedzai.jervicectl;

import java.util.ArrayList;

public class ServiceCfg{
    public String name;
    public ArrayList<String> dependencies;

    public ServiceCfg(String name, ArrayList<String> dependencies){
        this.name = name;
        this.dependencies = dependencies;
    }

    /**
    * instantiates the service associated with this service configuration and returns it.
    *
    * @return an instance of the service configured by the current object. */
    public Service instantiateService() throws NoSuchServiceException{
        try{
            return (Service)Class.forName(name).newInstance();
        }catch(ClassNotFoundException | InstantiationException | IllegalAccessException e){
            throw new NoSuchServiceException("Unable to load service " + name);
        }
    }
}
