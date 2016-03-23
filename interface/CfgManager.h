#ifndef __CFG_MANAGER__
#define __CFG_MANAGER__

#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <iterator>
#include <map>
#include <vector>
#include <string>
#include <algorithm> 
#include <iomanip>

#include "TObject.h"

class CfgManager: public TObject
{
public:
    //---ctors---
    CfgManager() {};
    CfgManager(std::map<std::string, std::vector<std::string> >* defaultCfg) {opts_=*defaultCfg;};
    CfgManager(const char* file) {ParseConfigFile(file);};
    //---dtor---
    ~CfgManager() {};

    //---getters---
    template<typename T=std::string> T GetOpt(std::string key, int opt=0);
                    
    //---setters---
    inline void            SetOpt(const char* key, std::vector<std::string>& v_opt)
        {opts_["opts."+std::string(key)]=v_opt;};
    inline void            SetOpt(std::string& key, std::vector<std::string>& v_opt)
        {opts_["opts."+key]=v_opt;};

    //---utils
    bool                   OptExist(std::string key, int opt=0);
    inline void            ParseConfigFile(const std::string file) {ParseConfigFile(file.c_str());};
    void                   ParseConfigFile(const char* file);
    void                   ParseConfigString(const std::string config);
    virtual void           Print(Option_t* option="") const;   
    
    //---operators---
    friend std::ostream& operator<<(std::ostream& out, const CfgManager& obj);

private:
    //---utils---
    void                    ParseSingleLine(const std::string& line, std::vector<std::string>& tokens);
    void                    HandleOption(std::string& current_block, std::vector<std::string>& tokens);
    void                    CopyBlock(std::string& current_block, std::string& block_to_copy);
    void                    Errors(std::string key, int opt=0);

private:
    std::map<std::string, std::vector<std::string> >  opts_;
    std::string username_;
    std::string timestamp_;

    //---ROOT dictionary
    ClassDef(CfgManager, 1)
};

#endif
