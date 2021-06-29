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
#include <regex>

//---data structures
typedef std::vector<std::string> option_t;
typedef std::vector<option_t>    voption_t;
    
class CfgManager
{    
public:
    //---ctors---
    CfgManager() {};
    CfgManager(std::map<std::string, option_t >* defaultCfg) {opts_=*defaultCfg; SetCreationInfo();};
    CfgManager(std::map<std::string, option_t >& defaultCfg) {opts_=defaultCfg; SetCreationInfo();};    
    CfgManager(const char* file) {ParseConfigFile(file); SetCreationInfo();};
    //---dtor---
    ~CfgManager() {};

    //---getters---
    template<typename T=std::string> T GetOpt(std::string key, int opt=0) const;
    CfgManager                         GetSubCfg(std::string block) const;    
    CfgManager                         GetSubCfg(std::vector<std::string> blocks) const;
                    
    //---setters---
    inline void            SetOpt(const char* key, option_t& v_opt)
        {opts_["opts."+std::string(key)]=v_opt;};
    inline void            SetOpt(std::string& key, option_t& v_opt)
        {opts_["opts."+key]=v_opt;};
    inline void            SetOpt(std::string key, option_t& v_opt)
        {opts_["opts."+key]=v_opt;};

    //---utils
    bool                   OptExist(std::string key, int opt=0) const;
    inline void            ParseConfigFile(const std::string file) {ParseConfigFile(file.c_str());};
    void                   ParseConfigFile(const char* file);
    void                   ParseConfigString(const std::string config);
    // virtual void           Print(std::ostream& out, Option_t* option="") const;
    // virtual void           Print(Option_t* option="") const;
    void                   WriteToFile(std::string filename="dump.cfg", bool overwrite=false) const;
    bool                   CompareOption(const CfgManager& comp, std::string key) const;
    
    //---operators---
    friend std::ostream& operator<<(std::ostream& out, const CfgManager& obj);
    
private:
    //---utils---
    void                    SetCreationInfo();
    bool                    ReplaceOptions(std::string& line);
    bool                    ParseSingleLine(std::string& line, option_t& tokens, bool replace_options=true);
    voption_t               ParseForLoop(std::ifstream& cfg_file, voption_t& for_cycle);
    voption_t               HandleForLoop(voption_t& for_cycle);
    void                    HandleOption(std::string& current_block, option_t& tokens);
    void                    CopyBlock(std::string& current_block, std::string& block_to_copy);
    std::string             Lookup(std::string& current_block, std::string& token, std::string origin_token="");
    void                    Errors(std::string key, int opt=0) const;

private:
    std::map<std::string, option_t >  opts_;
    std::string username_;
    std::string timestamp_;

};

#endif
