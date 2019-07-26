package com.feedzai.jervicectl;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

public class ServiceCfgTest extends TestCase {

    public ServiceCfgTest( String testName )
    {
        super( testName );
    }

    public static Test suite()
    {
        return new TestSuite( ServiceCfgTest.class );
    }

    public void setUp(){
    }

    public void testServiceInstantiation() {
        String sname = "com.feedzai.testServices.ServiceA";

        ServiceCfg cfg = new ServiceCfg( sname, null );
        
        Service s = null;
        try{
            s = cfg.instantiateService();
        }catch(NoSuchServiceException e){
            System.err.println("Unable to Setup the Test Environment. You probably need to create the mock services.");
            assert(false);
        }

        assert( s != null );
        assert( s.getClass().getName().equals( sname ) );
    }

    public void testInstantiationOfNonExistingService() {
        ServiceCfg cfg = new ServiceCfg( "com.feedzai.testServices.IHopeThisServiceDoesNotExist", null );
        try{
            cfg.instantiateService();
        }catch(NoSuchServiceException e){
            assert(true);
        }
    }

}
