package com.feedzai.jervicectl;

import java.util.ArrayList;

public class ServiceCfg{
    public String name;
    public ArrayList<String> dependencies;

    public ServiceCfg(String name, ArrayList<String> dependencies){
        this.name = name;
        this.dependencies = dependencies;
    }
}
