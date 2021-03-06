///
/// \file properties_file.hpp
///
///  Created on: Mar 22, 2011
///  Author: rdumitriu at gmail.com
///

#ifndef GERYON_PROPERTIESFILE_HPP_
#define GERYON_PROPERTIESFILE_HPP_

#include <string>
#include <map>

#include "string_utils.hpp"

namespace geryon { namespace util {

///
/// \brief Loads properties from a file or constructs a empty properties map.
///
/// The class is a very simple properties reader (which is somehow tolerant to errors). We didn't want to complicate
/// things, so here it is.
///
class G_CLASS_EXPORT PropertiesFile {
public:
    ///
    /// \brief Constructor - using a file.
    /// The constructor, most probably used
    /// \param file the file name
    ///
    explicit PropertiesFile(const std::string & file);

    ///
    /// \brief Constructor - empty
    /// The constructor, creates an empty props. You need to call load manually
    ///
    PropertiesFile() {}

    ///
    /// Destructor
    ///
    virtual ~PropertiesFile() {}

    ///
    /// \brief Checks to see if a property is set
    /// \param key the key
    /// \return true if the property is in there
    ///
    bool hasProperty(const std::string & key) const;

    ///
    /// \brief Gets a property
    /// Gets a certain property by the key. If the property is not found, returns an empty string
    /// \param key the key
    /// \return the value of the property
    ///
    std::string property(const std::string & key) const;

    ///
    /// \brief property
    /// \param key
    /// \param defValue
    /// \return
    ///
    std::string property(const std::string & key, const std::string & defValue) const;

    ///
    /// \brief Gets a property
    /// \param key the key
    /// \param defValue the default value
    ///
    template<typename T>
    T property(const std::string & key, const T & defValue) const {
        std::map<std::string, std::string>::const_iterator p = props.find(key);
        if(p == props.end()) {
            return defValue;
        }
        return convertTo(p->second, defValue);
    }

    ///
    /// \brief Loads from a file the props
    /// \param file the file
    ///
    void loadFromFile(const std::string & file);

private:


	std::map<std::string, std::string> props;
};

} }

#endif /* PROPERTIESFILE_HPP_ */
