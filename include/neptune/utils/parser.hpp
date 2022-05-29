#ifndef NEPTUNEORM_PARSER_HPP
#define NEPTUNEORM_PARSER_HPP

#include "neptune/entity.hpp"
#include "neptune/query_selector.hpp"
#include "neptune/utils/exception.hpp"
#include <set>
#include <string>
#include <vector>

namespace neptune {

class parser {
  friend class connection;
  friend class mariadb_connection;
  friend class entity;
  friend class driver;
  friend class mariadb_driver;

private:
  static std::vector<std::string>
  create_tables(const std::vector<std::shared_ptr<entity>> &entities);

  static std::set<std::string>
  get_default_select_set(const std::shared_ptr<entity> &e);
  static std::set<std::string> get_select_set(const std::shared_ptr<entity> &e,
                                              const query_selector &selector);
  static std::string insert_entity(const std::shared_ptr<entity> &e);
  static std::string query_last_insert_entity(const std::shared_ptr<entity> &e,
                                              const std::string &uuid);
  static std::string load_1to1_relation(const std::string &table,
                                        const std::string &key,
                                        const std::string &foreign_table,
                                        const std::string &foreign_key);
};

} // namespace neptune

#endif // NEPTUNEORM_PARSER_HPP
