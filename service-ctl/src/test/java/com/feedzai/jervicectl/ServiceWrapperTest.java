package com.feedzai.jervicectl;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

public class ServiceWrapperTest extends TestCase {

    ServiceWrapper wrapper;

    public ServiceWrapperTest( String testName )
    {
        super( testName );
    }

    public static Test suite()
    {
        return new TestSuite( ServiceWrapperTest.class );
    }

    public void setUp(){
        ServiceCfg cfg = new ServiceCfg( "com.feedzai.testServices.ServiceA", null );
        wrapper = null;
        try{
            wrapper = new ServiceWrapper("com.feedzai.testServices.ServiceA", cfg.instantiateService());
        }catch(NoSuchServiceException e){
            System.err.println("Unable to Setup the Test Environment. You probably need to create the mock services.");
        }
    }

    public void testStopControlThread()
    {
        Thread test = new Thread(wrapper);
        test.start();
        wrapper.stopControlThread();

        try{
            test.join(50);
        }catch(InterruptedException e){}

        assert(!test.isAlive());
    }

    public void testStopControlThreadWhenDone(){
        Thread test = new Thread(wrapper);
        test.start();
        wrapper.setTargetState(ServiceState.RUNNING);
        wrapper.setTargetState(ServiceState.STOPPED);
        wrapper.stopControlThreadWhenDone();

        try{
            test.join(500);
        }catch(InterruptedException e){}

        assert(!test.isAlive());
        assert(wrapper.getServiceState() == ServiceState.STOPPED);
    }

    public void testGetNextJobWhenRunningAndTargetIsRunning(){
        wrapper.setServiceState(ServiceState.RUNNING);
        wrapper.setTargetState(ServiceState.RUNNING);
        assert( wrapper.getNextJob() == null );
    }

    public void testGetNextJobWhenStoppedAndTargetIsStopped(){
        wrapper.setServiceState(ServiceState.STOPPED);
        wrapper.setTargetState(ServiceState.STOPPED);
        assert( wrapper.getNextJob() == null );
    }

    public void testGetNextJobWhenStoppedAndTargetIsRunning(){
        wrapper.setServiceState(ServiceState.STOPPED);
        wrapper.setTargetState(ServiceState.RUNNING);
        
        assert( wrapper.getNextJob() != null );
    }

    public void testGetNextJobWhenRunningAndTargetIsStopped(){
        wrapper.setServiceState(ServiceState.RUNNING);
        wrapper.setTargetState(ServiceState.STOPPED);

        assert( wrapper.getNextJob() != null );
    }

    public void testStartServiceJobStartsService(){
        wrapper.setServiceState(ServiceState.STOPPED);
        wrapper.setTargetState(ServiceState.RUNNING);

        wrapper.getNextJob().run();

        assert( wrapper.getServiceState() == ServiceState.RUNNING );
    }

    public void testStopServiceJobStopsService(){
        wrapper.setServiceState(ServiceState.RUNNING);
        wrapper.setTargetState(ServiceState.STOPPED);

        wrapper.getNextJob().run();

        assert( wrapper.getServiceState() == ServiceState.STOPPED );
    }

}
