// File: dbupdate.h
// Purpose: Support for loading Database drivers
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/dbupdate.h"
#include "dba/dbupdatescriptparser.h"
#include "dba/sharedsqlarchive.h"
#include <memory>

namespace dba {

DbUpdate::DbUpdate(SharedSQLArchive& pArchive, DbUpdateScriptsParser& pFileParser) 
  : mArchive(pArchive), 
    mFileParser(pFileParser)
{
};

void
DbUpdate::update(const std::string& pToVersion) {
  Transaction t(mArchive.createTransaction());
  try {
    //prepare parser
    SQLOStream ostream = t.getOStream();
    mFileParser.setFromVersion(getCurrentVersion());
    if (!pToVersion.empty()) {
      mFileParser.setToVersion(pToVersion);
    };
    mFileParser.prepareScriptsList();

    //perform update
    while (!mFileParser.isEof()) {
      std::string query(mFileParser.getNextQuery());
      onSendUpdate(ostream, query);
    }

    //set new version string in database
    setCurrentVersion(mFileParser.getToVersion());
  } catch (std::exception&) {
    t.rollback();
    throw;
  }
};

int
DbUpdate::onSendUpdate(SQLOStream& pOStream, std::string& pQuery) {
  // all transaction-controlling queries must be removed
  delTransactionStatementsFromQuery(pQuery, "BEGIN TRANSACTION", 17);
  delTransactionStatementsFromQuery(pQuery, "begin transaction", 17);
  delTransactionStatementsFromQuery(pQuery, "BEGIN", 5);
  delTransactionStatementsFromQuery(pQuery, "begin", 5);
  delTransactionStatementsFromQuery(pQuery, "COMMIT", 6);
  delTransactionStatementsFromQuery(pQuery, "commit", 6);
 
  // all "CREATE DATABASE" queries must be entered outside transaction
  makeCreateDatabaseQueriesOutsideTransaction(pQuery, true);
  makeCreateDatabaseQueriesOutsideTransaction(pQuery, false);

  if (pQuery.empty())
    return 0;
  return pOStream.sendUpdate(pQuery.c_str());
};

void
DbUpdate::delTransactionStatementsFromQuery(std::string& pQuery, const std::string& pSub, size_t pSize) const {
  size_t st;
  while ((st = pQuery.find(pSub)) != pQuery.npos) {
    pQuery.erase(st, pSize);
    st = pQuery.size();
    size_t sqlBlankCharCount=0;
    for (size_t i=0; i<st; ++i)
      if (pQuery[i]==' ' || pQuery[i]=='\t' || pQuery[i]==';') // cannot use isspace(c) due to an assert failing when pQuery consists of international chars (c may be >255)
        ++sqlBlankCharCount;
    if (sqlBlankCharCount == st)
      pQuery.clear();
  }
};

void 
DbUpdate::makeCreateDatabaseQueriesOutsideTransaction(std::string& pQuery, bool pUpperCase) const {
  const char* findStr = pUpperCase ? "CREATE DATABASE" : "create database";
  size_t st;
  while ((st = pQuery.find(findStr)) != pQuery.npos) {
    std::string singleQuery = pQuery.substr(st);
    const size_t subFind = singleQuery.find(";");
    if (subFind != pQuery.npos)
      singleQuery.resize(subFind);
    const size_t toReplaceStrSize = singleQuery.size();
    singleQuery.insert(6, " ");
    singleQuery.insert(0, "commit;");
    singleQuery += ";begin;";
    pQuery.replace(st, toReplaceStrSize, singleQuery);
  }
}; 

std::string
DbUpdate::getCurrentVersion() const {
  Transaction t(mArchive.createTransaction());
  const std::string query = "SELECT version from db_version";
  std::auto_ptr<dba::DbResult> res(t.getIStream().sendQuery(query.c_str()));
  if (res->fetchRow())
    if (!res->isNull(0))
      return std::string(res->getString(0));
  throw APIException(std::string("Unable to get db_version.version"));
}; 

void 
DbUpdate::setCurrentVersion(const std::string& pVer) const {
  Transaction t(mArchive.createTransaction());
  SQLOStream stream(t.getOStream());

  stream.sendUpdate("DELETE FROM db_version");

  const std::string query = "INSERT INTO db_version(version) VALUES('" + pVer + "')";
  stream.sendUpdate(query.c_str());
};

};

