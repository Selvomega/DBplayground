#ifndef SAKURA_SCHEMA_H__
#define SAKURA_SCHEMA_H__

#include "common/serde.hpp"

#include <optional>
#include <string>
#include <vector>

#include "type/field_type.hpp"

namespace wing {

class ColumnSchema {
 public:
  ColumnSchema(const std::string& name, FieldType type, uint32_t size)
    : name_(name), type_(type), size_(size) {}
  ColumnSchema(std::string&& name, FieldType type, uint32_t size)
    : name_(std::move(name)), type_(type), size_(size) {}
  std::string name_;
  FieldType type_;
  uint32_t size_;
  std::string ToString() const;
};

class ForeignKeySchema {
 public:
  ForeignKeySchema(uint32_t index, std::string&& table_name,
      std::string&& column_name, std::string&& name, FieldType type,
      uint32_t size)
    : index_(index),
      table_name_(std::move(table_name)),
      column_name_(std::move(column_name)),
      name_(std::move(name)),
      type_(type),
      size_(size) {}
  ForeignKeySchema(uint32_t index, const std::string& table_name,
      const std::string& column_name, const std::string& name, FieldType type,
      uint32_t size)
    : ForeignKeySchema(index, std::string(table_name), std::string(column_name),
      std::string(name), type, size) {}
  uint32_t index_;
  std::string table_name_;
  std::string column_name_;
  std::string name_;
  FieldType type_;
  uint32_t size_;
  std::string ToString() const;
};

class TableSchema {
 public:
  TableSchema() = default;

  TableSchema(std::string&& name, std::vector<ColumnSchema>&& column,
      std::vector<ColumnSchema>&& storage_columns, uint32_t primary_key_index,
      bool auto_gen_key, bool pk_hide, std::vector<ForeignKeySchema>&& fk)
    : name_(std::move(name)),
      columns_(std::move(column)),
      storage_columns_(std::move(storage_columns)),
      pk_index_(primary_key_index),
      auto_gen_key_(auto_gen_key),
      pk_hide_(pk_hide),
      fk_(std::move(fk)) {
    shuffle_to_storage_.resize(columns_.size());
    shuffle_from_storage_.resize(columns_.size());
    for (uint32_t i = 0; i < columns_.size(); i++) {
      for (uint32_t j = 0; j < columns_.size(); j++)
        if (storage_columns_[i].name_ == columns_[j].name_) {
          shuffle_from_storage_[i] = j;
          shuffle_to_storage_[j] = i;
          break;
        }
    }
  }

  std::optional<uint32_t> Find(std::string_view name) const {
    for (uint32_t i = 0; i < columns_.size(); i++)
      if (columns_[i].name_ == name) return i;
    return {};
  }

  const std::vector<ColumnSchema>& GetColumns() const { return columns_; }

  const std::vector<ColumnSchema>& GetStorageColumns() const { return storage_columns_; }

  const std::vector<uint32_t>& GetShuffleToStorage() const { return shuffle_to_storage_; }

  const std::vector<uint32_t>& GetShuffleFromStorage() const { return shuffle_from_storage_; }

  const ColumnSchema& operator[](uint32_t idx) const { return columns_[idx]; }

  std::string_view GetName() const { return name_; }

  uint32_t GetPrimaryKeyIndex() const { return pk_index_; }

  uint32_t GetStoragePrimaryKeyIndex() const { return shuffle_to_storage_[pk_index_]; }

  const ColumnSchema& GetPrimaryKeySchema() const { return (*this)[pk_index_]; }

  const std::vector<ForeignKeySchema>& GetFK() const { return fk_; }

  std::vector<ForeignKeySchema>& GetFK() { return fk_; }

  bool GetAutoGenFlag() const { return auto_gen_key_; }

  bool GetHidePKFlag() const { return pk_hide_; }

  std::string ToString() const;

  size_t Size() const { return columns_.size() + (pk_hide_ ? -1 : 0); }

 private:
  std::string name_;
  std::vector<ColumnSchema> columns_;
  std::vector<ColumnSchema> storage_columns_;
  std::vector<uint32_t> shuffle_to_storage_;
  std::vector<uint32_t> shuffle_from_storage_;
  // Every table must have a primary key.
  // Primary key index in columns_
  uint32_t pk_index_{0};
  // Primary key index in storage_columns_
  uint32_t storage_pk_index_{0};
  // True if user doesn't declare the primary key.
  bool auto_gen_key_{false};
  // True if the primary key doesn't show in schemas.
  bool pk_hide_{false};
  std::vector<ForeignKeySchema> fk_;
};

class DBSchema {
 public:
  DBSchema() = default;

  DBSchema(auto&& name, auto&& table) : name_(std::forward<decltype(name_)>(name)), tables_(std::forward<decltype(tables_)>(table)) {}

  std::optional<uint32_t> Find(std::string_view table_name) const {
    for (uint32_t i = 0; i < tables_.size(); i++)
      if (tables_[i].GetName() == table_name) return i;
    return {};
  }

  const std::vector<TableSchema>& GetTables() const { return tables_; }

  const TableSchema& operator[](uint32_t idx) const { return tables_[idx]; }

  std::vector<TableSchema>& GetTables() { return tables_; }

  TableSchema& operator[](uint32_t idx) { return tables_[idx]; }

  std::string_view GetName() const { return name_; }

  void AddTable(const TableSchema& table) { tables_.push_back(table); }

  void RemoveTable(std::string_view table_name) {
    auto id = Find(table_name);
    if (id.has_value()) tables_.erase(tables_.begin() + id.value());
  }

 private:
  std::string name_;
  std::vector<TableSchema> tables_;
};

template <typename S>
void tag_invoke(serde::tag_t<serde::serialize>, const ColumnSchema& x,
    S s) {
  serde::serialize(x.name_, s);
  serde::serialize(x.type_, s);
  serde::serialize(x.size_, s);
}

template <typename S>
void tag_invoke(serde::tag_t<serde::serialize>, const ForeignKeySchema& x,
    S s) {
  serde::serialize(x.index_, s);
  serde::serialize(x.table_name_, s);
  serde::serialize(x.column_name_, s);
  serde::serialize(x.name_, s);
  serde::serialize(x.type_, s);
  serde::serialize(x.size_, s);
}

template <typename S>
void tag_invoke(serde::tag_t<serde::serialize>, const TableSchema& x, S s) {
  serde::serialize(x.GetName(), s);
  serde::serialize(x.GetColumns(), s);
  serde::serialize(x.GetStorageColumns(), s);
  serde::serialize(x.GetPrimaryKeyIndex(), s);
  serde::serialize(x.GetAutoGenFlag(), s);
  serde::serialize(x.GetHidePKFlag(), s);
  serde::serialize(x.GetFK(), s);
}

namespace serde {

template <>
struct Deserialize<wing::ColumnSchema> {
  template <typename D>
  static Result<wing::ColumnSchema, typename D::Error> deserialize(D d) {
    std::string name =
      EXTRACT_RESULT(serde::Deserialize<std::string>::deserialize(d));
    wing::FieldType type =
      EXTRACT_RESULT(serde::Deserialize<wing::FieldType>::deserialize(d));
    uint32_t size =
      EXTRACT_RESULT(serde::Deserialize<uint32_t>::deserialize(d));
    return wing::ColumnSchema(std::move(name), type, size);
  }
};

template <>
struct Deserialize<wing::ForeignKeySchema> {
  template <typename D>
  static Result<wing::ForeignKeySchema, typename D::Error> deserialize(D d) {
    uint32_t index = EXTRACT_RESULT(serde::Deserialize<uint32_t>::deserialize(d));
    std::string table_name = EXTRACT_RESULT(serde::Deserialize<std::string>::deserialize(d));
    std::string column_name = EXTRACT_RESULT(serde::Deserialize<std::string>::deserialize(d));
    std::string name = EXTRACT_RESULT(serde::Deserialize<std::string>::deserialize(d));
    wing::FieldType type = EXTRACT_RESULT(serde::Deserialize<wing::FieldType>::deserialize(d));
    uint32_t size = EXTRACT_RESULT(serde::Deserialize<uint32_t>::deserialize(d));
    return wing::ForeignKeySchema(index, std::move(table_name),
      std::move(column_name), std::move(name), type, size);
  }
};

template <>
struct Deserialize<wing::TableSchema> {
  template <typename D>
  static Result<wing::TableSchema, typename D::Error> deserialize(D d) {
    std::string name =
      EXTRACT_RESULT(serde::Deserialize<std::string>::deserialize(d));
    std::vector<wing::ColumnSchema> column =
      EXTRACT_RESULT(serde::Deserialize<
        std::vector<wing::ColumnSchema>>::deserialize(d));
    std::vector<wing::ColumnSchema> storage_columns =
      EXTRACT_RESULT(serde::Deserialize<
        std::vector<wing::ColumnSchema>>::deserialize(d));
    uint32_t primary_key_index =
      EXTRACT_RESULT(serde::Deserialize<uint32_t>::deserialize(d));
    bool auto_gen_key =
      EXTRACT_RESULT(serde::Deserialize<bool>::deserialize(d));
    bool pk_hide = EXTRACT_RESULT(serde::Deserialize<bool>::deserialize(d));
    std::vector<wing::ForeignKeySchema> fk =
      EXTRACT_RESULT(serde::Deserialize<
        std::vector<wing::ForeignKeySchema>>::deserialize(d));
    return wing::TableSchema(std::move(name), std::move(column),
      std::move(storage_columns), primary_key_index, auto_gen_key, pk_hide,
      std::move(fk));
  }
};

} // namespace serde
} // namespace wing

#endif