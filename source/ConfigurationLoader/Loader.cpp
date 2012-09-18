/**
 * @file Loader.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 18/09/2012
 * @section LICENSE
 *
 * Copyright NIWA Science �2012 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include <fstream>
#include <iostream>

#include "Loader.h"
#include "../Translations/Translations.h"
#include "../Utilities/To.h"

// Namespaces
using std::ifstream;
using std::cout;
using std::endl;

namespace util = iSAM::Utilities;

namespace iSAM {
namespace Configuration {

/**
 * Default constructor
 */
Loader::Loader() {
}

/**
 * Destructor
 */
Loader::~Loader() {
}

/**
 * Load the configuration file into our system. This method will
 * query the global_configuration for the name of the configuration
 * file to load.
 *
 * Once it has the name it'll load the file into a member vector
 * then start parsing it.
 *
 * @param skip_loading_file Use what is in memory instead?
 */
void Loader::LoadConfigFile(bool skip_loading_file) {
}

/**
 * Load the parameter into memory. This is used instead
 * of loading from a configuration file for unit testing the system
 *
 * @param lines The configuration vector to load
 */
void Loader::LoadIntoCache(vector<string> &lines) {
  lines_.clear();
  for (unsigned i = 0; i < lines.size(); i++)
    lines_.push_back(lines[i]);
}

/**
 * Process the block we have loaded into current_block_. We determine
 * what kind of @block we're looking at and then we call the factory with the
 * type so we can get back a raw Base::Object to assign the parameters too.
 */
void Loader::ProcessBlock() {

  /**
   * current_block_type is the @block that we're currently working with
   * (e.g @analysis, @global, @process etc).
   */
  string current_block_type = current_block_[0].substr(1, current_block_[0].length()-1);

  int space_location = current_block_type.find_first_of(' '); // Check for space
  if (space_location > 0)
    current_block_type = current_block_type.substr(0, space_location);

  current_block_type = util::ToLowercase(current_block_type);

  // Based on the @section, we want to get a ParameterList
  try {
    string type = GetTypeFromCurrentBlock();


  } catch (string &ex) {
    RETHROW_EXCEPTION(ex);
  }
}

/**
 * Parse the current command block and find the line that starts
 * with "type".
 *
 * @return The 'type' parameter for this block
 */
string Loader::GetTypeFromCurrentBlock() {
  for (int i = 0; i < (int)current_block_.size(); ++i) {
    int type_location = current_block_[i].find(PARAM_TYPE);

    if (type_location == 0) {
      int space_location = current_block_[i].find_first_of(' ');
      string type = current_block_[i].substr(space_location+1, current_block_[i].length()-space_location);
      return util::ToLowercase(type);
    }
  }

  return "";
}

/**
 * This method is responsible for loading the parameter file
 * into the cache for processing. We load everything into
 * cache at the beginning instead of on the fly so we can
 * process @include's and remove any commented sections
 *
 * NOTE: This method is recursive for include files
 *
 * @param file_name The name of the file to load
 */
void Loader::LoadConfigIntoCache(string file_name) {
  try {
     if (file_name == "")
       THROW_EXCEPTION("No file_name passed through to be loaded into cache");

     ifstream config_file(file_name.c_str());
     if (!config_file)
       THROW_EXCEPTION("Failed to open the file: " + file_name);

     string current_line        = "";
     bool   multi_line_comment  = false;

     while (getline(config_file, current_line)) {
       if (current_line.length() == 0)
         continue;

       // Var
       int index = -1;

       // In ML Comment?
       if (multi_line_comment) {
         // Check For }
         index = (int)current_line.find_first_of(CONFIG_MULTI_COMMENT_END);
         if (index >= 0) {
          multi_line_comment = false; // End Comment Run
          current_line = current_line.substr(index+1, current_line.length()-index); // Remove upto }

         } else
           continue; // Get Next Line and ignore this one
       }

       if (current_line.length() == 0)
         continue;

       // Remove Any # Single Line Comments
       index = (int)current_line.find_first_of(CONFIG_SINGLE_COMMENT);
       if (index >= 0)
         current_line = current_line.substr(0, index);

       // Remove Any { } Multiline Comments
       index = (int)current_line.find_first_of(CONFIG_MULTI_COMMENT_START);
       if (index >= 0) {
         multi_line_comment = true;
         current_line = current_line.substr(0, index);
       }

       // Replace \t (Tabs) with Spaces
       index = (int)current_line.find_first_of("\t");
       while (index != -1) {
         current_line.replace(index, 1, " ");
         index = (int)current_line.find_first_of("\t");
       }

       // Remove any leading spaces
       index = 0;
       while (current_line[index] == ' ')
         index++;

       if (index > 0)
         current_line = current_line.substr(index, current_line.length() - index);

       // look for @include commands
       index = (int)current_line.find_first_of(" ");
       if ( (index != -1) && (current_line.substr(0, index) == CONFIG_INCLUDE)) { //command is '@include'
         // Find First "
         index = (int)current_line.find_first_of("\"");
         if (index == -1)
           THROW_EXCEPTION("@include file name should be surrounded by quotes");
         // get line from First " (+1) onwards
         string include_file = current_line.substr(index+1, current_line.length()-index);

         // Remove last "
         index = (int)include_file.find_first_of("\"");
         if (index == -1)
           THROW_EXCEPTION("@include file name is missing a closing quote");
         include_file = include_file.substr(0, index);

         // Check if it's absolute or relative
         if ( (include_file.substr(0, 1) == "/") || (include_file.substr(1, 1) == ":") )
           LoadConfigIntoCache(include_file); // Absolute

         else {
           int last_location = file_name.find_last_of('/');
           if (last_location == -1)
             last_location = file_name.find_last_of('\\');

           if (last_location == -1) {
             LoadConfigIntoCache(include_file);

           } else {
            string directory = file_name.substr(0, last_location+1);
            LoadConfigIntoCache(directory + (directory==""?"":"//") + include_file); // Relative

           }
         }

         // Blank line so it's not added
         current_line = "";
       }

       // Remove trailing spaces caused by Comments
       int last_not_space = current_line.find_last_not_of(" ");
       current_line = current_line.substr(0, (last_not_space+1));

       if (current_line.length() > 0)
         lines_.push_back(current_line);
     }
     config_file.close();

   } catch (const string &ex) {
     RETHROW_EXCEPTION(ex);
   }
}

/**
 *
 */
void Loader::AssignParameters(shared_ptr<Object> object) {

}

/**
 *
 */
void Loader::AssignTableParameters(shared_ptr<Object> object, int &current_index) {

}

/**
 *
 */
void Loader::SplitLineIntoVector(string line, vector<string> &parameters) {

}

} /* namespace Configuration */
} /* namespace iSAM */


















