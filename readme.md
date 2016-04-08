#Jervicectl

##Java Service Control

Allows users to control the lifecycle of java based services

a.k.a. jabardice control

##Running
In order to run jervicectl you can either compile it or download the jar associated with the repository.

After running you will have access to a menu. Clicking <enter> shows an help message, which is probably the best way to start.

The best user experience is achieved when you have created the mock services and after you load an interesting configuration file. See section below for details.

##Seting up the test environment
In order to set up the test environment, you must have the file testConfigs/big.cfg in your current path and you must create the mock services.

In order to create the mock services, assuming you're using an UNIX based system, you must navigate to jervicectl/src/test/java/com/feedzai/testServices and execute the file genServices.sh. This will create the mock services.

After having the configuration file and the mock services set up you can use maven or another tool to run the unit tests and watch closely as they pass every single case.
