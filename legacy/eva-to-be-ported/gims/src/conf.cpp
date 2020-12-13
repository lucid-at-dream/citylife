#include "conf.hpp"

conf::config_t configuration;

void printUsage()
{
        std::cout << "ToQueMidaS - Topological Querying Middleware System" << std::endl
                  << std::endl
                  << "Usage: toquemidas [options]" << std::endl
                  << std::endl
                  << "Options:" << std::endl
                  << "  -h, --help                    display this help message and exit" << std::endl
                  << "  -c FILE, --config-file=FILE   load configurations from FILE" << std::endl;
}

bool blankLine(char *line)
{
        int i = 0;
        while (line[i] == '\t' || line[i] == ' ')
                i++;
        if (line[i] != '\n')
                return false;
        return true;
}

void conf::printCurrentConfiguration()
{
        std::cout << "==== BEGIN Current Configuration ====" << std::endl;
        std::cout << "Quadtree: " << std::endl;
        std::cout << "max_points_per_node " << configuration.max_points_per_node << std::endl;
        std::cout << std::endl;
        std::cout << "Projection:" << std::endl;
        std::cout << "projection_srid " << configuration.projection_srid << std::endl;
        std::cout << std::endl;

        std::cout << "Approximations:" << std::endl;
        std::cout << "type " << configuration.approximationType << std::endl;
        std::cout << "n-gon size " << configuration.NGON_SIZE << std::endl;
        std::cout << std::endl;

        std::cout << "Database:" << std::endl;
        for (std::list<db_conf>::iterator it = configuration.db_confs.begin(); it != configuration.db_confs.end();
             it++) {
                std::cout << it->keyword << " " << it->value << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Data Layers: " << std::endl;
        for (std::list<char *>::iterator it = configuration.db_layers.begin(); it != configuration.db_layers.end();
             it++) {
                std::cout << *it << std::endl;
        }
        std::cout << "==== END Current Configuration ====" << std::endl;
}

int conf::readConfigurationFiles(int argc, char **argv)
{
        //verify if an help message was requested
        for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                        printUsage();
                        exit(0);
                }
        }

        //verify if another file location is provided via command line argument
        for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config-file") == 0) {
                        if (argc > i + 1) {
                                char *fname = argv[i + 1];
                                if (verifyFileExistance(fname)) {
                                        std::cout << "Configuration loaded from: " << fname << std::endl;
                                        loadConfigurationFile(fname);
                                        return 0;
                                } else {
                                        std::cout << "Configuration file " << fname << " does not exist." << std::endl;
                                        return 1;
                                }
                        } else {
                                std::cout << "No filename provided after option -c" << std::endl;
                        }
                }
        }

        //set up default file location list
        char filelist[3][50] = {
                "gims.conf"
                "$HOME/.conf/gims.conf",
                "/etc/gims/gims.conf",
        };

        setDefaults();

        bool loaded = false;
        for (char *fname : filelist) {
                if (verifyFileExistance(fname)) {
                        std::cout << "Configuration loaded from: " << fname << std::endl;
                        loadConfigurationFile(fname);
                        loaded = true;
                        break;
                }
        }

        if (!loaded) {
                std::cout << "could not locate any configuration file. Looked for it in the current "
                             "directory under name \"gims.conf\", in $HOME/.conf/gims.conf and /etc/gims/gims.conf"
                          << std::endl;
                return 1;
        }

        return 0;
}

void conf::setDefaults()
{
        configuration.max_points_per_node = 1024;
        configuration.projection_srid = 3395;
        configuration.approximationType = 2;
        configuration.NGON_SIZE = 5;
}

int conf::loadConfigurationFile(char *fname)
{
        FILE *f = fopen(fname, "r");

        char *line = (char *)malloc(256 * sizeof(char));
        size_t size = sizeof(char) * 256;

        int nbytes;

        while ((nbytes = getline(&line, &size, f)) != -1) {
                if (line[0] != '#' && !blankLine(line)) {
                        if (line[nbytes - 1] == '\n')
                                line[nbytes - 1] = '\0';
                        parseCfgLine(line);
                }
        }

        free(line);
        fclose(f);
        return 0;
}

void conf::parseCfgLine(char *line)
{
        int i = 0;
        while (line[i] != ' ')
                i++;

        char *value = line + i + 1;
        line[i] = '\0';

        if (strcmp(line, "MAX_POINTS_PER_NODE") == 0) {
                configuration.max_points_per_node = atoi(value);

        } else if (strcmp(line, "PROJECTION_SRID") == 0) {
                configuration.projection_srid = atoi(value);

        } else if (strcmp(line, "APPROXIMATION_TYPE") == 0) {
                configuration.approximationType = atoi(value);

        } else if (strcmp(line, "NGON_SIZE") == 0) {
                configuration.NGON_SIZE = atoi(value);

        } else if (strcmp(line, "DB_CONF") == 0) {
                char *db_key = value;
                char *db_val = value;

                int i = 0;
                while (db_val[i] != ':')
                        i++;
                db_val += i + 1;
                db_key[i] = '\0';

                char *db_val_copy = (char *)malloc(sizeof(char) * (strlen(db_val) + 1));
                strcpy(db_val_copy, db_val);
                char *db_key_copy = (char *)malloc(sizeof(char) * (strlen(db_key) + 1));
                strcpy(db_key_copy, db_key);

                conf::db_conf dbcfg = { db_key_copy, db_val_copy };
                configuration.db_confs.push_back(dbcfg);

        } else if (strcmp(line, "DB_LAYER") == 0) {
                char *layer_copy = (char *)malloc(sizeof(char) * (strlen(value) + 1));
                strcpy(layer_copy, value);
                configuration.db_layers.push_back(layer_copy);

        } else {
                std::cout << "unknown keyword in configuration file: " << line << std::endl;
        }
}

void conf::freeConf()
{
        for (std::list<db_conf>::iterator it = configuration.db_confs.begin(); it != configuration.db_confs.end();
             it++) {
                free(it->keyword);
                free(it->value);
        }

        for (std::list<char *>::iterator it = configuration.db_layers.begin(); it != configuration.db_layers.end();
             it++) {
                free(*it);
        }
}
