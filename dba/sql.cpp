//
// C++ Implementation: sqlquery
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "sql.h"
#include <algorithm>
#include <iostream>
#include <sstream>

#include "dba/exception.h"
#include "dba/filtermapper.h"
#include "dba/storeablefilter.h"
#include "dba/database.h"
#include "dba/sqlutils.h"

namespace dba {

SQL::SQL()
{
}

SQL::SQL(const char* pData) 
  : mSQLQuery(pData) 
{
  parseQuery();
}

SQL::SQL(const std::string& pData) 
  : mSQLQuery(pData)
{
  parseQuery();
}

void
SQL::parseQuery() {
  SQLParamParser p(mSQLQuery,mPlaceholders);
  p.parse();
}

void 
SQL::setFilterDataForNextParam(DataContainerBase* pData, const char* pTypeName) {
  mParams.push_back(ParamData(pData,pTypeName));
  //std::cerr << "added " << mParams.size() << " param " << pTypeName << " [" << pData << "]" << std::endl;
}

void 
SQL::setFilterDataForNextParam(StoreableFilterBase* pFilter) {
  mParams.push_back(ParamData(pFilter));
  //std::cerr << "added " << mParams.size() << " param " << pTypeName << " [" << pData << "]" << std::endl;
}

void 
SQL::setFilterDataForNextVar(void* pData, const char* pTypeName, const char* pSQLName, int pSQLPos) {
  mVars.push_back(VarData(pData,pTypeName, pSQLName, pSQLPos));
  //std::cerr << "added " << mParams.size() << " param " << pTypeName << " [" << pData << "]" << std::endl;
}

void 
SQL::setFilterDataForNextVar(StoreableFilterBase* pFilter, const char* pSQLName, int pSQLPos) {
  mVars.push_back(VarData(pFilter, pSQLName, pSQLPos));
  //std::cerr << "added " << mParams.size() << " param " << pTypeName << " [" << pData << "]" << std::endl;
}

std::string 
SQL::cstring(const FilterMapper& pMapper, const ConvSpec& pSpecs) const {
  std::string query(fillQueryParams(pMapper,pSpecs));
  return query;
}

int 
SQL::getCountWithoutEscapes(const Placeholders& pPh) const {
  int ret = 0;
  for(Placeholders::const_iterator it = pPh.begin(); it != pPh.end(); it++) {
    switch(it->mConvertType) {
      case PARAM_ESCAPED:
      case PARAM_ESCAPE_CHAR:
      break;
      default:
        ret++;
      break;
    }
  }
  return ret;
};

SQL::ParamVal
SQL::convertParam(const FilterMapper& pMapper, const ConvSpec& pSpecs, const ParamData& pData) const {
  try {
    StoreableFilterBase* filter;
    if (pData.mFilter != NULL) {
      filter = pData.mFilter.ptr();
    } else {
      filter = &pMapper.findFilter(pData.mTypeName);
      filter->updateRef(pData.mData->getPtr());
    }
    if (!filter->hasRef())
      throw APIException("Custom filter has to be constructed with data to use for query");
    ParamVal ret;
    ret.mIsNull = filter->isNull();
    if (ret.mIsNull) {
      ret.mValue = "NULL";
    } else {
      ret.mValue = filter->toString(pSpecs);
    }
    return ret;
  } catch (const FilterNotFoundException&) {
    std::stringstream error;
    error << "No default filter found for type " << pData.mTypeName << ". Use SQLArchive::mapType to set one";
    throw APIException(error.str());
  };
  //just to remove compiler warnings, should never get there
  return ParamVal(NULL);
};


std::string
SQL::fillQueryParams(const FilterMapper& pMapper, const ConvSpec& pSpecs) const {
  //if no params was passed then do not interpret placeholders and replace them with data
  //this allows user to pass std::string queries with colons to streams 
  if (mParams.size() == 0) return mSQLQuery;

  if (mParams.size() != getCountWithoutEscapes(mPlaceholders)) {
    std::stringstream error;
    error << "Got " << mPlaceholders.size() << " placeholders but " << mParams.size() << " params was passed";
    throw APIException(error.str());
  }
  std::string ret(mSQLQuery);
  if (mPlaceholders.size() != 0) {
    Placeholders::const_iterator ph = mPlaceholders.end();
    Params::const_iterator pa = mParams.end();
    do {
      ph--;
      ParamVal rep;
      switch(ph->mConvertType) {
        case PARAM_STR_VALUE:
          pa--;
          rep = convertParam(pMapper,pSpecs,*pa);
          if (!rep.mIsNull) rep.mValue = SQLUtils::escapeSQLData(rep.mValue);
        break;
        case PARAM_STR_LIKE:
          pa--;
          rep = convertParam(pMapper,pSpecs,*pa);
          if (!rep.mIsNull) rep.mValue = SQLUtils::escapeSQLLike(rep.mValue);
        break;
        case PARAM_ESCAPED:
          rep = ":";
        break;
        case PARAM_ESCAPE_CHAR:
          rep = " ESCAPE '!'";
        break;
        default:
          pa--;
          rep = convertParam(pMapper,pSpecs,*pa);
        break;
      }
      if (ph->mShouldQuote && !rep.mIsNull) {
        rep.mValue.insert(0,"'");
        rep.mValue.append("'");
      };
      ret.replace(ph->mIndex,2,rep.mValue);
    } while(ph != mPlaceholders.begin());
  };
  //std::cerr << "Full query: [" << ret.c_str() << "]" << std::endl;
  return ret;
}

void 
SQL::appendData(const SQL& pQuery) {
  appendFilterData(pQuery);
  Placeholders h(pQuery.mPlaceholders);
  for(Placeholders::iterator it = h.begin(); it != h.end(); it++)
    it->mIndex += mSQLQuery.length();
  mPlaceholders.insert(mPlaceholders.end(),h.begin(),h.end());
  mSQLQuery += pQuery.mSQLQuery;
}

void 
SQL::appendFilterData(const SQL& pQuery) {
  mParams.insert(mParams.end(),pQuery.mParams.begin(),pQuery.mParams.end());
  mVars.insert(mVars.end(),pQuery.mVars.begin(),pQuery.mVars.end());
}

void 
SQL::updateVars(DbResult& pRes, const FilterMapper& pMapper, const ConvSpec& pSpecs) const {
  int current_index = 0;
  for(Vars::const_iterator it = mVars.begin(); it != mVars.end(); it++) {
    int res_index = -1;
    if (!it->hasName() && !it->hasIndex()) {
      res_index = current_index++;
    } else {
      if (it->hasName()) {
        res_index = pRes.getColumnIndex(it->mSQLName);
      } else {
        res_index = it->mSQLIndex;
      }
    }
    if (res_index >= pRes.columns()) {
      std::stringstream error;
      error << "Query returned " << pRes.columns() << " and we have only " << mVars.size() << " vars defined";
      throw APIException(error.str());
    };

    StoreableFilterBase* filter;
    if (it->mFilter != NULL) {
      filter = it->mFilter;
    } else {
      filter = &pMapper.findFilter(it->mTypeName);
      filter->updateRef(it->mData);
    }

    if (!filter->hasRef())
      throw APIException("Custom filter has to be constructed with data to use for query");

    Database::StoreType store_type;
    if (it->mStoreType != Database::UNKNOWN)
      store_type = it->mStoreType;
    else
      store_type = filter->getPrefferedStoreType();

    if (pRes.isNull(res_index)) {
      filter->fromNull();
    } else {
      switch(store_type) {
        case Database::INTEGER:
          filter->fromInt(pSpecs, pRes.getInt(res_index));
        break;
        case Database::STRING:
          filter->fromString(pSpecs, pRes.getString(res_index));
        break;
        case Database::FLOAT:
          filter->fromDouble(pSpecs, pRes.getDouble(res_index));
        break;
        case Database::DATE:
          filter->fromDate(pSpecs, pRes.getDate(res_index));
        break;
        default:
          throw APIException("Invalid or undefined store type");
        break;
      }
    }
  }
}


SQL::~SQL()
{
}

SQL::SQLParamParser::SQLParamParser(const std::string& pSQLQuery,SQL::Placeholders& pPlaceholders) 
  : mSQLQuery(pSQLQuery),
    mPlaceholders(pPlaceholders)
{};

void
SQL::SQLParamParser::parse() {
  mState.clear();
  mState.push_back(SCAN);

  //std::cerr << "Parsing [" << mSQLQuery.c_str() << "]" << std::endl;
  for(std::string::iterator it = mSQLQuery.begin(); it != mSQLQuery.end(); it++) {
    switch (*it) {
      case '\'':
        switch (mState.back()) {
          case SCAN:
            mState.push_back(QUOTED_FIELD);
          break;
          case AFTER_LIKE:
            mState.push_back(QUOTED_LIKE);
          break;
          case QUOTED_FIELD:
          case QUOTED_LIKE:
            mState.push_back(QUOTE_TEST);
          break;
          case QUOTE_TEST:
            mState.pop_back();
          break;
        };
      break;
      case ':':
        switch (mState.back()) {
          case SCAN:
            mState.push_back(PARAM_TEST);
          break;
          case QUOTED_FIELD:
          case QUOTED_LIKE:
            mState.push_back(PARAM_TEST);
          break;
          case PARAM_TEST:
            mState.pop_back();
            setCurrentParamIndex(it);
          break;
          case QUOTE_TEST:
            mState.pop_back();
            checkSQLLike(it);
            mState.pop_back();
            mState.push_back(PARAM_TEST);
          break;
        };
      break;
      default:
        switch (mState.back()) {
          case SCAN:
            if ((tolower(*it) == ('e')) && (it - mSQLQuery.begin() >= 3)) {
              std::string::iterator b = it - 3;
              std::string like_cand = std::string(b,(it + 1));
              std::transform(like_cand.begin(),like_cand.end(),like_cand.begin(),::tolower);
              if (like_cand == "like")
                mState.push_back(AFTER_LIKE);
            }
          break;
          case QUOTED_FIELD:
          break;
          case PARAM_TEST:
            mState.pop_back();
            setCurrentParamIndex(it);
          break;
          case QUOTE_TEST:
            mState.pop_back();
            checkSQLLike(it);
            mState.pop_back();
          break;
        };
      break;
    };
  };
  if (mState.back() == QUOTE_TEST) {
    mState.pop_back();
    checkSQLLike(mSQLQuery.end());
  };
};

void
SQL::SQLParamParser::checkSQLLike(const std::string::iterator& pPos) {
  if (mState.back() == QUOTED_LIKE) {
    size_t index = std::distance(mSQLQuery.begin(),pPos);
    mPlaceholders.push_back(ParamPlaceholder(index,SQL::PARAM_ESCAPE_CHAR,false));
  };
}

void
SQL::SQLParamParser::setCurrentParamIndex(std::string::iterator& pPos) {
  size_t index = std::distance(mSQLQuery.begin(),pPos) - 1;
  convert_type type = SQL::PARAM_UNKNOWN;
  switch (*pPos) {
    case 's':
      if (mState.back() == QUOTED_LIKE) type = SQL::PARAM_STR_LIKE;
      else type = SQL::PARAM_STR_VALUE;
    break;
    case 'd':
      type = SQL::PARAM_INT;
    break;
    case 'f':
      type = SQL::PARAM_FLOAT;
    break;
    case ':':
      type = SQL::PARAM_ESCAPED;
    break;
    default: {
      std::stringstream error;
      error << "wrong parameter type :" << *pPos << " at char " << index << " in query ";
      throw APIException(error.str());
    } break;
  };
  bool shouldquote = false;
  switch(type) {
    case SQL::PARAM_STR_VALUE:
    case SQL::PARAM_STR_LIKE:
    case SQL::PARAM_DATE:
      shouldquote = (mState.back() == SCAN);
    break;
    default:
      shouldquote = false;
    break;
  };
  mPlaceholders.push_back(ParamPlaceholder(index,type,shouldquote));
  //std::cerr << "Added " << mPlaceholders.size() << " placeholder at " << index << ", type " << type << ", inquotes: " << shouldquote << std::endl;
}

SQL
operator+(const dba::SQL& pQuery, const SQL& pFrom) {
  dba::SQL q(pQuery);
  q.appendData(pFrom);
  return q;
}

SQL&
SQL::operator+(const SQL& pFrom) {
  appendData(pFrom);
  return *this;
}

SQL&
SQL::operator+=(const SQL& pFrom) {
  return (*this) + pFrom;
}


SQL
operator<< (const SQL& pQuery, StoreableFilterBase* pFilter) {
  SQL q(pQuery);
  q.setFilterDataForNextParam(pFilter);
  return q;
}


} //namespace

