#include "interface/CfgManager.h"
#include "interface/CfgManagerT.h"

//**********getters***********************************************************************
//----------GetSubCfg---------------------------------------------------------------------
//---Get subset of the current config file
//---blocks specifies which blocks are copied to the subset.

//---one block
CfgManager CfgManager::GetSubCfg(std::string block) const
{
    return GetSubCfg(std::vector<std::string>({block}));
}

//---many blocks
CfgManager CfgManager::GetSubCfg(std::vector<std::string> blocks) const
{
    //---loop over the blocks storing them in a separate map
    std::map<std::string, option_t >  selected_opts;
    for(auto& block: blocks)
    {
        if(opts_.count("opts."+block))
        {
            for(auto& iopt : opts_)
            {
                auto pos = iopt.first.find("opts."+block+".");
                if(pos == 0)
                    selected_opts[iopt.first]= iopt.second;
            }
        }
        else
            std::cout << "> CfgManager::GeuSubCfg --- WARNING: undefined block -> " << block << std::endl;
    }
    
    //---return new CfgMaanager object
    return CfgManager(selected_opts);
}

//**********utils*************************************************************************

//----------Set creatio info--------------------------------------------------------------
//---Called by the constructors
void CfgManager::SetCreationInfo()
{
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

//----------Check if the key is in cfg----------------------------------------------------
bool CfgManager::OptExist(std::string key, int opt) const
{
    for(auto& iopt : opts_)
        if(iopt.first == "opts."+key && int(iopt.second.size())>opt)
            return true;

    return false;
}

//----------Replace $option[n] with the corresponding value-------------------------------
bool CfgManager::ReplaceOptions(std::string& line)
{
    bool replaced=false; 
    std::regex option_pattern("\\$\\w+(\\.[\\w+)*(\\[[0-9]+\\])?");
    auto matches_begin = std::sregex_iterator(line.begin(), line.end(), option_pattern);
    auto matches_end = std::sregex_iterator();
    for(std::sregex_iterator it = matches_begin; it != matches_end; ++it)
    {
        auto opt_str = it->str();
        auto brk_pos = opt_str.find("[");
        int opt_idx = 0;
        if(brk_pos != std::string::npos)
            opt_idx = stoi(opt_str.substr(brk_pos+1, opt_str.size()-brk_pos-2));
        else
            brk_pos = opt_str.size()-1;
        if(OptExist(opt_str.substr(1, brk_pos-1), opt_idx))
        {
            line.replace(line.find(opt_str), opt_str.size(), GetOpt<std::string>(opt_str.substr(1, brk_pos-1), opt_idx));
            replaced = true;
        }
    }

    return replaced;
}

//----------Help method, parse single line------------------------------------------------
bool CfgManager::ParseSingleLine(std::string& line, option_t& tokens, bool replace_options)
{
    //---strip commented lines and unneeded whitespace
    while(line.size() > 0 && line.at(0) == ' ')
        line.erase(line.begin());
    if(line.size() == 0 || line.at(0) == '#')
        return false;
    
    //---evaluate options before parsing the line
    //   if $option is found and exist its value is inserted in place of $option
    if(replace_options)
        ReplaceOptions(line);
 
    //---parsing utils
    size_t prev=0, pos;
    std::string delimiter=" ";
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

//----------Help method, parse for loops--------------------------------------------------
voption_t CfgManager::ParseForLoop(std::ifstream& cfg_file, voption_t& for_cycle)
{
    option_t tokens;
    std::string buffer;
    while(tokens.size() == 0 || tokens.at(0) != "end")
    {
        tokens.clear();
        if(!getline(cfg_file, buffer))
        {
            std::cout << "> CfgManager --- ERROR: runaway for loop, missing end marker. -> " << std::endl;
            exit(-1);
        }
        if(ParseSingleLine(buffer, tokens, false))
        {
            //---Nested for loop (recursive)
            if(tokens.at(0) == "for")
            {
                voption_t nested_for_cycle;
                nested_for_cycle.push_back(tokens);            
                auto parsed_loop = ParseForLoop(cfg_file, nested_for_cycle);
                for(auto& token : parsed_loop)
                        for_cycle.push_back(token);
            }
            else
            {
                //---multiple lines option inside for-loop
                while(tokens.back() == "\\")
                {
                    tokens.pop_back();
                    getline(cfg_file, buffer);
                    ParseSingleLine(buffer, tokens, false);                    
                }
                for_cycle.push_back(tokens);
            }
        }
    }

    return HandleForLoop(for_cycle);
}

//----------Parse configuration file and setup the configuration--------------------------
void CfgManager::ParseConfigFile(const char* file)
{
    std::cout << "> CfgManager --- INFO: parsing " << file << std::endl;
    //---read config file
    std::ifstream cfg_file(file, std::ios::in);
    std::string buffer;
    std::string current_block="opts";
    while(getline(cfg_file, buffer))
    {
        //---parse the current line
        option_t tokens;
        if(!ParseSingleLine(buffer, tokens))
            continue;

        //---multiple line option
        while(tokens.back() == "\\")
        {
            tokens.pop_back();
            getline(cfg_file, buffer);
            ParseSingleLine(buffer, tokens);
        }
        //---for loop
        if(tokens.at(0) == "for")
        {
            voption_t for_cycle;
            for_cycle.push_back(tokens);
            //---get the options from the loop
            auto parsed_loop = ParseForLoop(cfg_file, for_cycle);
            for(auto& tokens : parsed_loop)
                HandleOption(current_block, tokens);

        }
        //---store the option inside the current configuration
        else
            HandleOption(current_block, tokens);
    }
    cfg_file.close();
    
    return;
}

//----------Parse configuration string and setup the configuration------------------------
void CfgManager::ParseConfigString(const std::string config)
{
    std::string current_block="opts";
    //---parse the current string
    std::string local_copy = config;
    option_t tokens;
    ParseSingleLine(local_copy, tokens);
    HandleOption(current_block, tokens);

    return;
}

//----------Handle for-loop---------------------------------------------------------------
voption_t CfgManager::HandleForLoop(voption_t& for_cycle)
{
    //---get for-loop range
    if(for_cycle.at(0).size() < 3)
    {
        std::cout << "> CfgManager --- ERROR: for loop is undefined, provide at least 3 arguments. -> " << std::endl;
        exit(-1);
    }        
    auto loop_def = for_cycle.at(0);
    auto loop_var = loop_def.at(1);
    std::string loop_counter;
    if(loop_var.find(",") != std::string::npos)
    {
        loop_counter = loop_var.substr(0, loop_var.find(","));
        loop_var = loop_var.substr(loop_var.find(",")+1);
    }    
    for_cycle.pop_back();
    for_cycle.erase(for_cycle.begin());

    //---output expanded loop
    voption_t parsed_loop;
    
    //---rage loop [min, max) optional increment value
    if(loop_def.size() >= 4)
    {
        int increment = 1;
        if(loop_def.size() == 5)
            increment = stoi(loop_def.at(4));
        for(int i=stoi(loop_def.at(2)); i<stoi(loop_def.at(3)); i=i+increment)
            for(auto line : for_cycle)
            {
                option_t tokens;
                for(auto token : line)
                {
                    while(token.find("$"+loop_var) != std::string::npos)
                        token.replace(token.find("$"+loop_var), loop_var.size()+1, std::to_string(i));
                    ReplaceOptions(token);
                    tokens.push_back(token);
                }
                parsed_loop.push_back(tokens);
            }
    }
    //---for each loop, third argument must be a valid option.
    else if(loop_def.size() == 3 && OptExist(loop_def.at(2)))
    {
        unsigned int cnt = 0;
        for(auto& i : GetOpt<option_t >(loop_def.at(2)))
        {
            for(auto& line : for_cycle)
            {
                option_t tokens;                
                for(auto token : line)
                {
                    while(token.find("$"+loop_var) != std::string::npos)
                        token.replace(token.find("$"+loop_var), loop_var.size()+1, i);
                    while(loop_counter != "" && token.find("$"+loop_counter) != std::string::npos)
                        token.replace(token.find("$"+loop_counter), loop_counter.size()+1, std::to_string(cnt));
                    ReplaceOptions(token);
                    tokens.push_back(token);
                }
                parsed_loop.push_back(tokens);
            }
            ++cnt;
        }
    }
    
    return parsed_loop;
}

//----compare specified option <key> of this cfg with <comp>
bool CfgManager::CompareOption(const CfgManager& comp, const std::string key) const
{
    //---comparison is true if option exist in both cfgs and is identical or
    //   if it doens't exist in neither of the two
    return (OptExist(key, -1) && comp.OptExist(key, -1) && GetOpt<option_t >(key) == comp.GetOpt<option_t >(key)) ||
        (!OptExist(key, -1) && !comp.OptExist(key, -1));
}

//----------Handle single option: key and parameters--------------------------------------
//---private methode called by ParseConfig public methods
void CfgManager::HandleOption(std::string& current_block, option_t& tokens)
{
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
                std::cout << "> CfgManager --- ERROR: wrong closing block -> " << tokens.at(0) << std::endl;
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
        for(auto& cfg_file: tokens)
            ParseConfigFile(cfg_file.c_str());
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
                auto token_full = Lookup(current_block, token);
                if(OptExist(token_full))
                {
                    auto extend_opt = GetOpt<option_t >(token_full);
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
            int pos = -1;
            if(copy.find("[") != std::string::npos)
            {
                pos = stoi(copy.substr(copy.find("[")+1, copy.find("]")-copy.find("[")-1));
                copy = copy.substr(0, copy.find("["));
            }
            copy = Lookup(current_block, copy);
            key = key.substr(0, key.size()-1);
            if(OptExist(copy))
            {
                //---copy only selected option field
                if(pos != -1)
                    opts_[current_block+"."+key].push_back(GetOpt<std::string>(copy, pos));            
                //--copy entire option
                else 
                    opts_[current_block+"."+key] = GetOpt<option_t >(copy);
            }
            //---throw warning and ignore token
            else
                std::cout << "> CfgManager --- WARNING: undefined option -> " << copy << std::endl;
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
            option_t& opts = opt.second;
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
    for(int i=0; i<banner.size(); ++i)
        out << "=";
    out << std::endl;
    out << banner << std::endl;
    for(int i=0; i<banner.size(); ++i)
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

//----------Write cfg to text file--------------------------------------------------------
void CfgManager::WriteToFile(std::string filename, bool overwrite) const
{
    std::ifstream checkfile(filename);
    if(checkfile && !overwrite)
    {
        std::cout << "> CfgManager::WriteToFile(std::string filename, bool overwrite) --- WARNING: file " << filename
                  << " already exists:  specify different filename or set overwrite to true."
                  << std::endl;
        return;
    }

    std::ofstream dump(filename);
    if(dump.is_open())
    {
        //---IMPROVEME
        for(auto& key : opts_)
        {
            if(key.second.size())
            {
                dump << key.first.substr(key.first.find_first_of(".")+1) << " ";
                for(auto& opt : key.second)
                    dump << "'" << opt << "' ";
                dump << std::endl;
            }
        }
    }
}

//----------Lookup for full option/block name---------------------------------------------
//---search for option/block in the options map:
//---1) first for an exact match
//---2) otherwise for the first match going backwards from the current block
std::string CfgManager::Lookup(std::string& current_block, std::string& token, std::string origin_token)
{
    if(origin_token.size() == 0)
        origin_token = token;
    if(OptExist(token, -1))
        return token;
    else if(current_block != "opts" && OptExist(current_block.substr(5)+"."+token, -1))
        return current_block.substr(5)+"."+token;
    else if(current_block.find('.') != std::string::npos)
    {
        auto prev_block = current_block.substr(0, current_block.find_last_of('.'));
        auto try_token = prev_block == "opts" ? token : prev_block.substr(5)+"."+token;
        return Lookup(prev_block, try_token, origin_token);        
    }
    else
        return origin_token;
}

//----------Internal error check----------------------------------------------------------
void CfgManager::Errors(std::string key, int opt) const
{
    if(opts_.count(key) == 0)
    {
        std::cout << "> CfgManager --- ERROR: key '"<< key.substr(5, key.size()) << "' not found" << std::endl;
        exit(-1);
    }
    if(opt >= opts_.at(key).size())
    {
        std::cout << "> CfgManager --- ERROR: option '"<< key.substr(5, key.size()) << "' as less then "
             << opt << "values (" << opts_.at(key).size() << ")" << std::endl;
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
