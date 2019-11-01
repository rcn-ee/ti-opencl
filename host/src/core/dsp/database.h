/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <string>
#include <vector>
#include <iostream>
#include <sqlite3.h>

using namespace std;

class Database
{
  public:
    Database(const char* filename) : database(NULL) { open(filename); }
    ~Database()                                     { close(); }

    void close()
    {
        if (database) sqlite3_close(database);
        database = NULL;
    }

    vector<vector<string> > query(const char* query)
    {
        sqlite3_stmt *statement;
        vector<vector<string> > results;
        const int retry_limit = 20;
        int retries = 0;

        int rc = sqlite3_prepare_v2(database, query, -1, &statement, 0);

        while ((rc == SQLITE_BUSY || rc == SQLITE_LOCKED) &&
                ++retries <= retry_limit)
        {
             sqlite3_finalize(statement);
             usleep(100);
             rc = sqlite3_prepare_v2(database, query, -1, &statement, 0);
        }

        if (rc == SQLITE_OK)
        {
            int cols   = sqlite3_column_count(statement);
            int result = 0;

            while (true)
            {
               result = sqlite3_step(statement);

               if (result == SQLITE_ROW)
               {
                  vector<string> values;
                  for (int col = 0; col < cols; col++)
                    values.push_back((char*)sqlite3_column_text(statement,col));
                  results.push_back(values);
               }
               else break;
            }

            sqlite3_finalize(statement);
        }

        string error = sqlite3_errmsg(database);
        if (error != "not an error")
            std::cout << query << " " << error << std::endl;

        return results;
    }

  private:
    sqlite3 *database;

  private:
    bool open(const char* filename)
    {
        if (sqlite3_open(filename, &database) == SQLITE_OK)
        {
            sqlite3_busy_timeout(database, 1000);
            return true;
        }
        return false;
    }

};

#endif
