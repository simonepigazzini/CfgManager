#include "ExternalTools/CfgManager/interface/CfgManager.h"
#include "ExternalTools/CfgManager/interface/CfgManagerT.h"

//**********utils*************************************************************************

//----------Check if the key is in cfg----------------------------------------------------
bool CfgManager::OptExist(std::string key, int opt)
{
    for(auto& iopt : opts_)
        if(iopt.first == "opts."+key && int(iopt.second.size())>opt)
            return true;

    return false;
}

//----------Help method, parse single line------------------------------------------------
bool CfgManager::ParseSingleLine(std::string& line, std::vector<std::string>& tokens)
{
    //---parsing utils
    size_t prev=0, pos;
    std::string delimiter=" ";

    //---strip comments and unneeded whitespace
    while(line.size() > 0 && line.at(0) == ' ')
        line.erase(line.begin());
    if(line.size() == 0)
        return false;
    if(line.at(0) == '\'')
    {
        delimiter="\'";
        prev = 1;
    }
    
    //---line loop
    while((pos = line.find_first_of(delimiter, prev)) != std::string::npos)
    {
        if(pos > prev)
            tokens.push_back(line.substr(prev, pos-prev));
        prev = pos+1;
        if(line[prev] == '\'')
        {
            delimiter = "\'";
            ++prev;
        }
        else
            delimiter = " ";
    }
    if(prev < line.length())
        tokens.push_back(line.substr(prev, std::string::npos));

    return true;
}

//----------Parse configuration file and setup the configuration--------------------------
void CfgManager::ParseConfigFile(const char* file)
{
    std::cout << "> CfgManager --- INFO: parsing " << file << std::endl;
    //---read config file
    std::ifstream cfgFile(file, std::ios::in);
    std::string buffer;
    std::string current_block="opts";
    while(getline(cfgFile, buffer))
    {
        //---parse the current line
        std::vector<std::string> tokens;
        if(!ParseSingleLine(buffer, tokens))
            continue;

        //---multiple line option
        while(tokens.back() == "\\")
        {
            tokens.pop_back();
            getline(cfgFile, buffer);
            ParseSingleLine(buffer, tokens);
        }
        //---for loop
        if(tokens.at(0) == "for")
        {
            std::vector<std::vector<std::string> > for_cycle;
            for_cycle.push_back(tokens);            
            while(tokens.at(0) != "end")
            {
                tokens.clear();
                getline(cfgFile, buffer);
                ParseSingleLine(buffer, tokens);
                //---multiple lines option inside for-loop
                while(tokens.back() == "\\")
                {
                    tokens.pop_back();
                    getline(cfgFile, buffer);
                    ParseSingleLine(buffer, tokens);                    
                }
                for_cycle.push_back(tokens);                
            }
            //---call the for loop method removing the end line
            HandleForLoop(current_block, for_cycle);
        }
        //---store the option inside the current configuration
        else
            HandleOption(current_block, tokens);
    }
    cfgFile.close();
    
    //---set automatic info
    char hostname[100];
    gethostname(hostname, 100);
    username_ = std::string(getpwuid(getuid())->pw_name)+"@"+hostname;
    time_t rawtime;
    time(&rawtime);
    struct tm* t = localtime(&rawtime);
    timestamp_ = std::to_string(t->tm_mday)+"/"+std::to_string(t->tm_mon)+"/"+std::to_string(t->tm_year+1900)+"  "+
        std::to_string(t->tm_hour)+":"+std::to_string(t->tm_min)+":"+std::to_string(t->tm_sec);

    return;
}

//----------Parse configuration string and setup the configuration------------------------
void CfgManager::ParseConfigString(const std::string config)
{
    std::string current_block="opts";
    //---parse the current string
    std::string local_copy = config;
    std::vector<std::string> tokens;
    ParseSingleLine(local_copy, tokens);
    HandleOption(current_block, tokens);

    return;
}

//----------Handle for-loop---------------------------------------------------------------
void CfgManager::HandleForLoop(std::string& current_block, std::vector<std::vector<std::string> >& for_cycle)
{
    //---get for-loop range
    if(for_cycle.at(0).size() < 3)
    {
        std::cout << "> CfgManager --- ERROR: for loop is undefined, provide at least 3 arguments. // " << std::endl;
        exit(-1);
    }
    auto loop_def = for_cycle.at(0);
    auto loop_var = loop_def.at(1);
    for_cycle.pop_back();
    for_cycle.erase(for_cycle.begin());

    //---rage loop [min, max) optional increment value
    if(loop_def.size() >= 4)
    {
        int increment = 1;
        if(loop_def.size() == 5)
            increment = stoi(loop_def.at(4));
        for(int i=stoi(loop_def.at(2)); i<stoi(loop_def.at(3)); i=i+increment)
            for(auto line : for_cycle)
            {
                std::vector<std::string> tokens;
                for(auto token : line)
                {
                    while(token.find("$"+loop_var) != std::string::npos)
                        token.replace(token.find("$"+loop_var), loop_var.size()+1, std::to_string(i));
                    tokens.push_back(token);
                }
                HandleOption(current_block, tokens);
            }
    }
    //---for each loop, third argument must be a valid option.
    else if(loop_def.size() == 3 && OptExist(loop_def.at(2)))
        for(auto& i : GetOpt<std::vector<std::string> >(loop_def.at(2)))
            for(auto& line : for_cycle)
            {
                std::vector<std::string> tokens;
                for(auto& token : line)
                {
                    while(token.find("$"+loop_var) != std::string::npos)
                        token.replace(token.find("$"+loop_var), loop_var.size()+1, i);
                    tokens.push_back(token);
                }
                HandleOption(current_block, tokens);
            }
}

//----------Handle single option: key and parameters--------------------------------------
//---private methode called by ParseConfig public methods
void CfgManager::HandleOption(std::string& current_block, std::vector<std::string>& tokens)
{
    //---skip comments
    if(tokens.at(0).at(0) == '#')
        return;

    //---Handle blocks
    if(tokens.at(0).at(0) == '<')
    {        
        //---close previous block
        if(tokens.at(0).at(1) == '/')
        {
            tokens.at(0).erase(tokens.at(0).begin(), tokens.at(0).begin()+2);
            tokens.at(0).erase(--tokens.at(0).end());
            int last_dot = current_block.find_last_of(".");
            if(tokens.at(0) == current_block.substr(last_dot+1))
                current_block.erase(last_dot);
            else
            {
                std::cout << "> CfgManager --- ERROR: wrong closing block // " << tokens.at(0) << std::endl;
                exit(-1);
            }
        }
        //---open new block
        else 
        {
            bool copy_blocks=false;
            
            tokens.at(0).erase(tokens.at(0).begin());
            //---check if blocks copy is required
            if(tokens.at(0).back() == '=')
            {
                copy_blocks=true;
                tokens.back().erase(--tokens.back().end());
            }
            tokens.at(0).erase(--tokens.at(0).end());            
            current_block += "."+tokens.at(0);
            opts_[current_block];
            //---copy from other blocks
            if(copy_blocks)
            {
                tokens.erase(tokens.begin());
                for(auto& block_to_copy: tokens)
                    CopyBlock(current_block, block_to_copy);
            }
        }
    }
    //---import cfg
    else if(tokens.at(0) == "importCfg")
    {
        tokens.erase(tokens.begin());
        for(auto& cfgFile: tokens)
            ParseConfigFile(cfgFile.c_str());
    }
    //---option line
    else
    {        
        std::string key=tokens.at(0);
        tokens.erase(tokens.begin());
            
        //---update key
        if(key.substr(key.size()-2) == "+=")
        {
            key = key.substr(0, key.size()-2);
            for(auto& token : tokens)
            {
                if(OptExist(token))
                {
                    auto extend_opt = GetOpt<std::vector<std::string> >(token);
                    opts_[current_block+"."+key].insert(opts_[current_block+"."+key].end(),
                                                        extend_opt.begin(),
                                                        extend_opt.end());
                }
                else
                    opts_[current_block+"."+key].push_back(token);
            }
        }
        //---copy key
        else if(key.substr(key.size()-1) == "=" && tokens.size() > 0)
        {
            auto copy = tokens.at(0);
            key = key.substr(0, key.size()-1);
            //---copy only selected option field
            if(copy.find("[") != std::string::npos)
            {
                if(OptExist(copy.substr(0, copy.find("[")), stoi(copy.substr(copy.find("[")+1, copy.find("]")-copy.find("[")-1))))
                {
                    int pos = stoi(copy.substr(copy.find("[")+1, copy.find("]")-copy.find("[")-1));
                    copy = copy.substr(0, copy.find("["));
                    
                    opts_[current_block+"."+key].push_back(GetOpt<std::string>(copy, pos));
                }
            }
            //--copy entire option
            else if(OptExist(copy))
                opts_[current_block+"."+key] = GetOpt<std::vector<std::string> >(copy);
            //---throw error
            else
                std::cout << "> CfgManager --- WARNING: undefined option // " << copy << std::endl;
        }
        //---new key
        else
            opts_[current_block+"."+key] = tokens;
    }

    return;
}

//----------Copy entire block-------------------------------------------------------------
//---already defined option are overridden
void CfgManager::CopyBlock(std::string& current_block, std::string& block_to_copy)
{
    //---search for the correct block name
    auto block_to_copy_full = Lookup(current_block, block_to_copy);
    bool found_any=false;    
    //---copy block entries
    for(auto& opt : opts_)
    {
        std::string key = opt.first;
        auto pos = key.find("."+block_to_copy_full+".");
        if(pos != std::string::npos)
        {
            std::string new_key = key;
            std::vector<std::string>& opts = opt.second;
            new_key.replace(pos+1, block_to_copy_full.size(), current_block.substr(5));
            opts_[new_key] = opts;

            found_any=true;
        }
    }
    //---block not found
    if(!found_any)
        std::cout << "> CfgManager --- WARNING: requested block (" << block_to_copy << ") is not defined" << std::endl;

    return;
}

//----------Print formatted version of the cfg--------------------------------------------
//---option is the key to be print: default value meas "all keys"
void CfgManager::Print(std::ostream& out, Option_t* option) const
{
    std::string argkey = option;
    //---banner
    std::string banner = "configuration was created by "+username_+" on "+timestamp_;
    for(unsigned int i=0; i<banner.size(); ++i)
        out << "=";
    out << std::endl;
    out << banner << std::endl;
    for(unsigned int i=0; i<banner.size(); ++i)
        out << "=";
    out << std::endl;
    
    //---options
    std::string prev_block="";
    for(auto& key : opts_)
    {
        std::string current_block = key.first.substr(5, key.first.find_last_of(".")-5);
        if(argkey == "" || key.first.find(argkey) != std::string::npos)
        {
            if(current_block != prev_block)
            {
                if(prev_block != "")
                    out << "+----------" << std::endl;
                out << current_block << ":" << std::endl;
                prev_block = current_block;
            }
            if(key.second.size())
            {
                out << "|----->" << key.first.substr(key.first.find_last_of(".")+1) << ": ";
                for(auto& opt : key.second)
                    out << opt << ", " ;
                out << std::endl;
            }
        }
    }
    out << "+----------" << std::endl;

    return;
}

//----------ROOT-style Print function-----------------------------------------------------
void CfgManager::Print(Option_t* option) const
{
    Print(std::cout, option);

    return;
}

//----------Lookup for full option/block name---------------------------------------------
//---search for option/block in the options map:
//---1) first for an exact match
//---2) otherwise for the first match going backwards from the current block
std::string CfgManager::Lookup(std::string& current_block, std::string& token)
{
    if(OptExist(token, -1))
        return token;
    else if(current_block.find('.') != std::string::npos)
    {
        auto prev_block = current_block.substr(0, current_block.find_last_of('.'));
        auto try_token = prev_block.substr(5)+"."+token;
        std::cout << prev_block << " " << try_token << std::endl;
        return Lookup(prev_block, try_token);        
    }
    else
        return std::string("");
}

//----------Internal error check----------------------------------------------------------
void CfgManager::Errors(std::string key, int opt)
{
    if(opts_.count(key) == 0)
    {
        std::cout << "> CfgManager --- ERROR: key '"<< key.substr(5, key.size()) << "' not found" << std::endl;
        exit(-1);
    }
    if(opt >= int(opts_[key].size()))
    {
        std::cout << "> CfgManager --- ERROR: option '"<< key.substr(5, key.size()) << "' as less then "
             << opt << "values (" << opts_[key].size() << ")" << std::endl;
        exit(-1);
    }
    return;
}

//**********operators*********************************************************************

std::ostream& operator<<(std::ostream& out, const CfgManager& obj)
{
    obj.Print(out);
    
    return out;
}
