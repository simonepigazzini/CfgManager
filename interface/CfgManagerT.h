#ifndef __CFG_MANAGER_T__
#define __CFG_MANAGER_T__

#include "CfgManager.h"

//**********getters***********************************************************************

//----------get option by name------------------------------------------------------------
template<typename T>
T CfgManager::GetOpt(std::string key, int opt) const
{
    key = "opts."+key;
    Errors(key, opt);
    T opt_val;
    std::istringstream buffer(opts_.at(key)[opt]);
    buffer >> opt_val;
    
    return opt_val;
}

template<> inline std::string CfgManager::GetOpt(std::string key, int opt) const
{
    key = "opts."+key;
    Errors(key, opt);

    return opts_.at(key)[opt];
}   

template<> inline std::vector<float> CfgManager::GetOpt(std::string key, int opt) const
{
    key = "opts."+key;
    Errors(key, opt);
    std::vector<float> optsVect;
    for(unsigned int iOpt=opt; iOpt<opts_.at(key).size(); ++iOpt)
    {
        double opt_val;
        std::istringstream buffer(opts_.at(key)[iOpt]);
        buffer >> opt_val;
        optsVect.push_back(opt_val);
    }
    
    return optsVect;
}    

template<> inline std::vector<double> CfgManager::GetOpt(std::string key, int opt) const
{
    key = "opts."+key;
    Errors(key, opt);
    std::vector<double> optsVect;
    for(unsigned int iOpt=opt; iOpt<opts_.at(key).size(); ++iOpt)
    {
        double opt_val;
        std::istringstream buffer(opts_.at(key)[iOpt]);
        buffer >> opt_val;
        optsVect.push_back(opt_val);
    }
    
    return optsVect;
}    

template<> inline std::vector<std::string> CfgManager::GetOpt(std::string key, int opt) const
{
    key = "opts."+key;
    Errors(key, opt);
    
    return opts_.at(key);
}    

#endif
