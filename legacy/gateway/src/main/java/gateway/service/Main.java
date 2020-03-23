package gateway.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Main {

    private static final Logger logger = LoggerFactory.getLogger(Main.class);

    public static void main(String[] args) throws Exception {

        int gatewayPort = 8080;
        int registryPort = 9999;

        for (int i = 1; i < args.length; i++) {
            switch(args[i]) {
                case "-gp":
                case "--gateway-port":
                    gatewayPort = Integer.parseInt(args[++i]);
                    break;

                case "-rp":
                case "--registry-port":
                    registryPort = Integer.parseInt(args[++i]);
                    break;

                case "-h":
                case "--help":
                    printUsage();
                    System.exit(0);

                default:
                    printUsage();
                    System.exit(1);
            }
        }

        MainService mainService = new MainService(gatewayPort, registryPort);
        mainService.run();
        mainService.join();
    }

    private static void printUsage() {
        System.out.println("Usage: java -jar gateway.jar [-h|--help] [-p|--port <port-number>]");
        System.out.println();
        System.out.println("-h|--help   Print this help message and exit");
        System.out.println("-p|--port   The port in which requests will be served. Defaults to 8080.");
    }
}
