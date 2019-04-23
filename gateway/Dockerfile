FROM java:latest

CMD pacman -Syuu

COPY target/gateway-service*.jar gateway-service.jar

CMD java -jar gateway-service.jar
