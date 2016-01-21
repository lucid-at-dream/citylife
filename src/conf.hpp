#ifndef _CONF_HPP_
#define _CONF_HPP_

#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <list>

inline bool verifyFileExistance( const std::string& fname ) {
  struct stat buffer;
  return (stat (fname.c_str(), &buffer) == 0); 
}
bool blankLine(char *line);

namespace conf{

    typedef struct _db_conf{
        char *keyword;
        char *value;
    }db_conf;

    typedef struct _config_t{
        int max_points_per_node;
        int projection_srid;
        int approximationType;
        int NGON_SIZE;
        std::list<db_conf> db_confs;
        std::list<char *> db_layers;
    }config_t;

    void printCurrentConfiguration();

    void setDefaults();
    int readConfigurationFiles(int argc, char **argv);
    int loadConfigurationFile(char *fname);
    void parseCfgLine(char *line);
    int readArgs(int argc, char **argv);
    void freeConf();
}

/* configuration holds the information present in the configuration file.*/
extern conf::config_t configuration;

#endif
