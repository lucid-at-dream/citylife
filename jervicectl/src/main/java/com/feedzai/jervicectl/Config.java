package com.feedzai.jervicectl;

import java.util.ArrayList;
import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Config{

    private String filename;

    public Config(String filename){
        this.filename = filename;
    }

    /**
    * Attempts to parse the configuration file associated with the Config object instance
    * and returns an object describing the services configuration.
    *
    * @return A collection of ServiceCfg objects describing each service and dependencies*/
    public ArrayList<ServiceCfg> parseConfig(){
        String content = this.getFileContent();
        return this.parseServices(content);
    }

    /** 
    * Given a string describing a service, returns the service name
    *
    * @param service description as in the configuration file.
    * @return the service name (i.e. name of its class) */
    private String getServiceName(String serviceDescription){
        String serviceNameRegex = "[^\\{]*";
        return getFirstMatch(serviceDescription, serviceNameRegex).trim();
    }

    /** 
    * Given a string describing a service, returns the service's dependencies
    *
    * @param service description as in the configuration file.
    * @return the service's dependencies */
    private ArrayList<String> getServiceDepencies(String serviceDescription){
        String serviceDepsRegex = "\\{[^\\}]*\\}";
        String dependencies = getFirstMatch(serviceDescription, serviceDepsRegex).trim();

        return removeEmptyDeps( dependencies.split("[ \\{\\}\\t\\n]+") );
    }

    /**
    * Given a string describing a service, returns a ServiceCfg instance
    * with both the service's class and dependencies list.
    * 
    * @param Configuration file text describing the service
    * @return A ServiceCfg object describing the service configuration*/
    private ServiceCfg parseService(String service){
        String name = getServiceName(service);
        ArrayList<String> dependencies = getServiceDepencies(service);
        
        return new ServiceCfg(name, dependencies);
    }

    /**
    * @return a string containing all data contained in the file associated with the instance.
    */
    private String getFileContent(){
        String content = "";
        try{
            File cfgFile = new File(this.filename);
            Scanner cfgScanner = new Scanner(cfgFile);
            content = cfgScanner.useDelimiter("\\Z").next();
        }catch(FileNotFoundException e){
            System.err.println("Configuration file " + this.filename + " was not reachable.");
        }
        return content;
    }

    /**
    * Given the contents of the configuration file returns a collection of ServiceCfg objects
    * describing the services and dependencies.
    *
    * @return A collection of ServiceCfg objects describing each service and dependencies*/
    private ArrayList<ServiceCfg> parseServices(String content){
        String serviceEntryRegex = "[^\\{]*\\{[^\\}]*\\}";
        Pattern pattern = Pattern.compile(serviceEntryRegex);
        Matcher matcher = pattern.matcher(content);
        
        ArrayList<ServiceCfg> services = new ArrayList();
        while( matcher.find() )
            services.add( this.parseService( matcher.group() ) );
        return services;
    }

    /**
    * Function to avoid empty string in arrays resulting from string splits.
    *
    * @param an array of possible empty strings
    * @return an ArrayList with all string in deps except empty ones.
    */
    private ArrayList<String> removeEmptyDeps(String[] deps){
        ArrayList<String> clean = new ArrayList();
        for(String d : deps)
            if( !d.equals("") )
                clean.add(d);
        return clean;
    }

    /**
    * Given a string and a regex returns the first match of regex in str.
    * Useful for situations where we're just there's only one match.
    *
    * @param A string to parse
    * @param A regex to match with the parameter string
    * @return A string corresponding to the first match of regex in str.
    */
    private String getFirstMatch(String str, String regex){
        Matcher matcher = Pattern.compile(regex).matcher(str);
        matcher.find();
        return matcher.group();
    }

}
