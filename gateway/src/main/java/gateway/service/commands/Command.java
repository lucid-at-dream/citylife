package gateway.service.commands;

import java.util.List;

public class Command {

    private String name;
    private String description;
    private List<CommandParameter> parameters;

    public Command() {}

    public Command(String name, String description, List<CommandParameter> parameters) {
        this.name = name;
        this.description = description;
        this.parameters = parameters;
    }

    public String getName() {
        return this.name;
    }

    public String getDescription() {
        return this.description;
    }

    public List<CommandParameter> getParameters() {
        return this.parameters;
    }

    public void setName(String name) {
        this.name = name;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public void setParameters(List<CommandParameter> parameters) {
        this.parameters = parameters;
    }
}
