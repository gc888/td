//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/utils/common.h"
#include "td/utils/Slice.h"
#include "td/utils/Status.h"

#include "td/db/detail/RawSqliteDb.h"

#include <memory>

struct sqlite3;
struct sqlite3_stmt;

namespace td {

class SqliteStatement {
 public:
  SqliteStatement() = default;
  SqliteStatement(const SqliteStatement &other) = delete;
  SqliteStatement &operator=(const SqliteStatement &other) = delete;
  SqliteStatement(SqliteStatement &&other) = default;
  SqliteStatement &operator=(SqliteStatement &&other) = default;
  ~SqliteStatement();

  Status bind_blob(int id, Slice blob) TD_WARN_UNUSED_RESULT;
  Status bind_string(int id, Slice str) TD_WARN_UNUSED_RESULT;
  Status bind_int32(int id, int32 value) TD_WARN_UNUSED_RESULT;
  Status bind_int64(int id, int64 value) TD_WARN_UNUSED_RESULT;
  Status bind_null(int id) TD_WARN_UNUSED_RESULT;
  Status step() TD_WARN_UNUSED_RESULT;
  Slice view_string(int id) TD_WARN_UNUSED_RESULT;
  Slice view_blob(int id) TD_WARN_UNUSED_RESULT;
  int32 view_int32(int id) TD_WARN_UNUSED_RESULT;
  int64 view_int64(int id) TD_WARN_UNUSED_RESULT;
  enum class Datatype { Integer, Float, Blob, Null, Text };
  Datatype view_datatype(int id);

  Result<string> explain();

  bool can_step() const {
    return state_ != Finish;
  }
  bool has_row() const {
    return state_ == GotRow;
  }
  bool empty() const {
    return !stmt_;
  }

  void reset();

  auto guard() {
    return ScopeExit{} + [this] {
      this->reset();
    };
  }

  // TODO get row

 private:
  friend class SqliteDb;
  SqliteStatement(sqlite3_stmt *stmt, std::shared_ptr<detail::RawSqliteDb> db);

  class StmtDeleter {
   public:
    void operator()(sqlite3_stmt *stmt);
  };

  enum { Start, GotRow, Finish } state_ = Start;

  std::unique_ptr<sqlite3_stmt, StmtDeleter> stmt_;
  std::shared_ptr<detail::RawSqliteDb> db_;

  Status last_error();
};

}  // namespace td
