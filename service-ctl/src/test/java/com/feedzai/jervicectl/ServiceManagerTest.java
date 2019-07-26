package com.feedzai.jervicectl;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import com.feedzai.testServices.AbstractTestService;

public class ServiceManagerTest extends TestCase {

    String prefix = "com.feedzai.testServices.Service";
    private ServiceManager serviceMgr;
    private String[] serviceSuffixes;

    public ServiceManagerTest( String testName )
    {
        super( testName );
    }

    public static Test suite()
    {
        return new TestSuite( ServiceManagerTest.class );
    }

    public void setUp(){
        this.serviceMgr = new ServiceManager();
        this. serviceSuffixes = new String[] {"A","B","C","D","E","F",
                                              "G","H","I","J","K","L",
                                              "M","N","O","P","Q","R",
                                              "S","T","U","V","W","X",
                                              "Y","Z"};
        try{
            this.serviceMgr.loadServicesFromConf("testConfigs/big.cfg");
        }catch(NoSuchServiceException | NoSuchConfigFileException e){
            System.err.println("Unable to Setup the Test Environment. You either don't" + 
                               " have the mock up services or the test config file.");
        }
    }

    public AbstractTestService getAbstractTestServiceFromSuffix(String suffix){
        return (AbstractTestService)getServiceFromSuffix(suffix).getService();
    }

    public void startServiceFromSuffix(String suffix){
        try{
            this.serviceMgr.startService(prefix+suffix);
        }catch(NoSuchServiceException e){
            System.err.println("Don't trust the tests, the mock up services haven't been created. Undefined behavior ahead.");
        }
    }

    public void stopServiceFromSuffix(String suffix){
        try{
            this.serviceMgr.stopService(prefix+suffix);
        }catch(NoSuchServiceException e){
            System.err.println("Don't trust the tests, the mock up services haven't been created. Undefined behavior ahead.");
        }
    }

    public ServiceWrapper getServiceFromSuffix(String suffix){
        try{
            ServiceWrapper sw = this.serviceMgr.getService(prefix+suffix);
            return sw;
        }catch(NoSuchServiceException e){
            System.err.println("Don't trust the tests, the mock up services haven't been created. Undefined behavior ahead.");
            return null;
        }
    }

    public void testServicesLoadingFromConfFile(){
        for(String service : serviceSuffixes){
            assert( getServiceFromSuffix( service ) != null );
        }
    }

    public void testDependenciesLoadingFromConfFile(){
        ServiceWrapper serviceD = getServiceFromSuffix("D"),
                       serviceG = getServiceFromSuffix("G"),
                       serviceK = getServiceFromSuffix("K"),
                       serviceL = getServiceFromSuffix("L"),
                       serviceO = getServiceFromSuffix("O"),
                       serviceP = getServiceFromSuffix("P");

        assert( serviceG.dependsOn(serviceD) );
        assert( serviceD.isDependencyOf(serviceG) );

        assert( serviceL.dependsOn(serviceK) );
        assert( serviceK.isDependencyOf(serviceL) );

        assert( serviceP.dependsOn(serviceO) );
        assert( serviceO.isDependencyOf(serviceP) );
    }

    public void testGetNonExistentService(){
        try{
            serviceMgr.getService("IWishIDidNotExist");
            assert(false);
        }catch(NoSuchServiceException e){
            assert(true);
        }
    }

    public void testStartNonExistentService(){
        try{
            serviceMgr.startService(prefix + "IWishIDidNotExist");
            assert(false);
        }catch(NoSuchServiceException e){
            assert(true);
        }
    }
    public void testStopNonExistentService(){
        try{
            serviceMgr.stopService(prefix + "IWishIDidNotExist");
            assert(false);
        }catch(NoSuchServiceException e){
            assert(true);
        }
    }

    public void testServiceStartingWithoutDependencies(){
        startServiceFromSuffix("A");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("A").getServiceState() == ServiceState.RUNNING );
    }

    public void testServiceStartingWithOneDependency(){
        startServiceFromSuffix("D");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("A").getServiceState() == ServiceState.RUNNING );
        assert( getServiceFromSuffix("D").getServiceState() == ServiceState.RUNNING );
    }

    public void testOneDependencyStartsBeforeService(){
        AbstractTestService srvD = getAbstractTestServiceFromSuffix("D");
        AbstractTestService srvA = getAbstractTestServiceFromSuffix("A");

        startServiceFromSuffix("D");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( srvD.startedAfter(srvA) );
    }

    public void testServiceStartingWithManyDependencies(){
        startServiceFromSuffix("H");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("H").getServiceState() == ServiceState.RUNNING );
        assert( getServiceFromSuffix("G").getServiceState() == ServiceState.RUNNING );
        assert( getServiceFromSuffix("F").getServiceState() == ServiceState.RUNNING );
        assert( getServiceFromSuffix("D").getServiceState() == ServiceState.RUNNING );
        assert( getServiceFromSuffix("E").getServiceState() == ServiceState.RUNNING );
        assert( getServiceFromSuffix("A").getServiceState() == ServiceState.RUNNING );
    }

    public void testManyDependenciesStartInOrder(){
        AbstractTestService srvC = getAbstractTestServiceFromSuffix("C");
        AbstractTestService srvM = getAbstractTestServiceFromSuffix("M");
        AbstractTestService srvN = getAbstractTestServiceFromSuffix("N");
        AbstractTestService srvO = getAbstractTestServiceFromSuffix("O");
        AbstractTestService srvP = getAbstractTestServiceFromSuffix("P");
        AbstractTestService srvQ = getAbstractTestServiceFromSuffix("Q");

        startServiceFromSuffix("Q");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( srvQ.startedAfter(srvP) );

        assert( srvP.startedAfter(srvM) );
        assert( srvP.startedAfter(srvN) );
        assert( srvP.startedAfter(srvO) );

        assert( srvM.startedAfter(srvC) );
        assert( srvN.startedAfter(srvC) );
        assert( srvO.startedAfter(srvC) );
    }

    public void testStartAll(){
        serviceMgr.startAll();
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        for(String service : serviceSuffixes){
            assert( getServiceFromSuffix(service).getServiceState() == ServiceState.RUNNING );
        }
    }

    public void testStopAll(){
        serviceMgr.startAll();
        serviceMgr.stopAll();

        serviceMgr.waitForJobsAndStopAllThreads(3000);

        for(String service : serviceSuffixes){
            assert( getServiceFromSuffix(service).getServiceState() == ServiceState.STOPPED );
        }
    }

    public void testContradictoryCommandsOnServiceWithoutDependencies(){
        startServiceFromSuffix("W");
        stopServiceFromSuffix("W");
        startServiceFromSuffix("W");
        stopServiceFromSuffix("W");

        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("W").getServiceState() == ServiceState.STOPPED );
    }

    public void testDependencyConsistencyAfterBigSequenceOfOperations(){
        serviceMgr.startAll();
        stopServiceFromSuffix("D");
        stopServiceFromSuffix("N");
        serviceMgr.startAll();
        serviceMgr.stopAll();
        startServiceFromSuffix("N");
        stopServiceFromSuffix("N");
        startServiceFromSuffix("P");
        startServiceFromSuffix("A");
        startServiceFromSuffix("K");
        startServiceFromSuffix("L");
        startServiceFromSuffix("M");
        stopServiceFromSuffix("A");

        serviceMgr.waitForJobsAndStopAllThreads(3000);

        for(String sname : serviceSuffixes){
            ServiceWrapper service = getServiceFromSuffix(sname);
            
            assert( service.getServiceState() == ServiceState.RUNNING || 
                    service.getServiceState() == ServiceState.STOPPED );

            if( service.getServiceState() == ServiceState.RUNNING )
                for( ServiceWrapper dependency : service.getDependencies() )
                    assert( dependency.getServiceState() == ServiceState.RUNNING );

            if( service.getServiceState() == ServiceState.STOPPED )
                for( ServiceWrapper dependent : service.getDependents() )
                    assert( dependent.getServiceState() == ServiceState.STOPPED );
        }
    }

    public void testLoadingConfigFromNonExistingFile(){
        try{
            serviceMgr.loadServicesFromConf("CelophaneMrCelophaneShouldHaveBeenMyName");
            assert(false);
        }catch(NoSuchConfigFileException e){
            assert(true);
        }catch(NoSuchServiceException e){
            assert(false);
        }
    }

    public void testServiceStoppingWithoutDependencies(){
        serviceMgr.startAll();
        stopServiceFromSuffix("H");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("H").getServiceState() == ServiceState.STOPPED );
    }

    public void testServiceStoppingWithOneDependent(){
        startServiceFromSuffix("H");
        stopServiceFromSuffix("G");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("G").getServiceState() == ServiceState.STOPPED );
        assert( getServiceFromSuffix("H").getServiceState() == ServiceState.STOPPED );
    }

    public void testOneDependentStopsBeforeService(){
        AbstractTestService srvH = getAbstractTestServiceFromSuffix("H");
        AbstractTestService srvG = getAbstractTestServiceFromSuffix("G");

        startServiceFromSuffix("H");
        stopServiceFromSuffix("G");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( srvH.stoppedBefore(srvG) );
    }

    public void testServiceStoppingWithManyDependents(){
        
        serviceMgr.startAll();
        stopServiceFromSuffix("A");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( getServiceFromSuffix("H").getServiceState() == ServiceState.STOPPED );
        assert( getServiceFromSuffix("G").getServiceState() == ServiceState.STOPPED );
        assert( getServiceFromSuffix("F").getServiceState() == ServiceState.STOPPED );
        assert( getServiceFromSuffix("D").getServiceState() == ServiceState.STOPPED );
        assert( getServiceFromSuffix("E").getServiceState() == ServiceState.STOPPED );
        assert( getServiceFromSuffix("A").getServiceState() == ServiceState.STOPPED );
    }

    public void testManyDependentsStopInOrder(){
        AbstractTestService srvC = getAbstractTestServiceFromSuffix("C");
        AbstractTestService srvM = getAbstractTestServiceFromSuffix("M");
        AbstractTestService srvN = getAbstractTestServiceFromSuffix("N");
        AbstractTestService srvO = getAbstractTestServiceFromSuffix("O");
        AbstractTestService srvP = getAbstractTestServiceFromSuffix("P");
        AbstractTestService srvQ = getAbstractTestServiceFromSuffix("Q");

        serviceMgr.startAll();
        stopServiceFromSuffix("C");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( srvQ.stoppedBefore(srvP) );

        assert( srvP.stoppedBefore(srvM) );
        assert( srvP.stoppedBefore(srvN) );
        assert( srvP.stoppedBefore(srvO) );

        assert( srvM.stoppedBefore(srvC) );
        assert( srvN.stoppedBefore(srvC) );
        assert( srvO.stoppedBefore(srvC) );
    }

}
