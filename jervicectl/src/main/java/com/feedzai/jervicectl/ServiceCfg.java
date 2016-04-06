package com.feedzai.jervicectl;

import java.util.ArrayList;

public class ServiceCfg{
    public String name;
    public ArrayList<String> dependencies;

    public ServiceCfg(String name, ArrayList<String> dependencies){
        this.name = name;
        this.dependencies = dependencies;
    }

    public static Service instantiateService(String name){
        Service s = null;
        try{
            s = (Service)Class.forName(name).newInstance();
        }catch(ClassNotFoundException | InstantiationException | IllegalAccessException e){
            e.printStackTrace();
        }
        return s;
    }
}
