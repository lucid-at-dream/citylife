FROM java:latest

CMD pacman -Syuu

COPY target/gateway-mainService*.jar gateway-mainService.jar

CMD java -jar gateway-mainService.jar
