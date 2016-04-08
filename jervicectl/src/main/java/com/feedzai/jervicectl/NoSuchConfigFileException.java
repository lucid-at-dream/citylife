package com.feedzai.jervicectl;

public class NoSuchConfigFileException extends Exception{
    //Parameterless Constructor
    public NoSuchConfigFileException() {}

    //Constructor that accepts a message
    public NoSuchConfigFileException(String message)
    {
     super(message);
    }
}