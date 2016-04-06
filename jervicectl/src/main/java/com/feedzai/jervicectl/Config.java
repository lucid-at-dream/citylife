package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Config{

    private String filename;
    private HashMap<String, HashMap> services;

    public Config(String filename){
        this.filename = filename;
    }

    private String getFirstMatch(String str, String regex){
        Matcher matcher = Pattern.compile(regex).matcher(str);
        matcher.find();
        return matcher.group();
    }

    private String getServiceName(String serviceDescription){
        String serviceNameRegex = "[^\\{]*";
        return getFirstMatch(serviceDescription, serviceNameRegex).trim();
    }

    private ArrayList<String> removeEmptyDeps(String[] deps){
        ArrayList<String> clean = new ArrayList();
        for(String d : deps)
            if( !d.equals("") )
                clean.add(d);
        return clean;
    }

    private ArrayList<String> getServiceDepencies(String serviceDescription){
        String serviceDepsRegex = "\\{[^\\}]*\\}";
        String dependencies = getFirstMatch(serviceDescription, serviceDepsRegex).trim();

        return removeEmptyDeps( dependencies.split("[ \\{\\}\\t\\n]+") );
    }

    private ServiceWrapper parseService(String service){

        String name = getServiceName(service);
        ArrayList<String> dependencies = getServiceDepencies(service);

        try{
            Service s = (Service)Class.forName(name).newInstance();
            return new ServiceWrapper(s);
        }catch(ClassNotFoundException | InstantiationException | IllegalAccessException e){
            e.printStackTrace();
        }

        return null;
    }

    public boolean loadConfig(){
        String content = "";
        
        try{
            File cfgFile = new File(this.filename);
            Scanner cfgScanner = new Scanner(cfgFile);
            content = cfgScanner.useDelimiter("\\Z").next();
        }catch(FileNotFoundException e){
            e.printStackTrace();
            return false;
        }

        String serviceRegex = "[^\\{]*\\{[^\\}]*\\}";
        Pattern pattern = Pattern.compile(serviceRegex);
        Matcher matcher = pattern.matcher(content);
        
        while(matcher.find()){
            this.parseService( matcher.group() );
        }

        return true;
    }

}
