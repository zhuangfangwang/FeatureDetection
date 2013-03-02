/*
 * LoggerFactory.cpp
 *
 *  Created on: 01.03.2013
 *      Author: Patrik Huber
 */

#include "logging/LoggerFactory.hpp"
#include <utility>

using std::pair;

namespace logging {

LoggerFactory::LoggerFactory()
{
}


LoggerFactory::~LoggerFactory()
{
}

LoggerFactory* LoggerFactory::Instance()
{
	static LoggerFactory instance;
	return &instance;
}

Logger& LoggerFactory::getLogger( const string name )
{
	map<string, Logger>::iterator it = loggers.find(name);
	if (it != loggers.end()) {
		return it->second;
	} else {
		Logger logger = Logger();
		logger.addAppender(make_shared<ConsoleAppender>());
		pair<map<string, Logger>::iterator, bool> ret = loggers.insert(make_pair(name, logger));
		if (ret.second == false) {
			// Key already existed, although we just checked? Should be impossible to reach here.
			// Throw something? Log?
		}
		return ret.first->second;
	}
	// We could think about if that's what we really want. Maybe we just want an empty
	// logger; For that, we could just do this here: return loggers[name];
}

} /* namespace logging */
