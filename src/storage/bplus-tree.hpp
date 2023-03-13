#ifndef BPLUS_TREE_H_
#define BPLUS_TREE_H_

#include "common/logging.hpp"

#include <cassert>
#include <filesystem>
#include <optional>
#include <stack>

#include "page-manager.hpp"

namespace wing {

/* Level 0: Leaves
 * Level 1: Inners
 * Level 2: Inners
 * ...
 * Level N: Root
 *
 * Initially the root is a leaf.
 *-----------------------------------------------------------------------------
 * Meta page:
 * Offset(B)  Length(B) Description
 * 0          1         Level num of root
 * 4          4         Root page ID
 * 8          8         Number of tuples (i.e., KV pairs)
 *-----------------------------------------------------------------------------
 * Inner page:
 * next_0 key_0 next_1 key_1 next_2 ... next_{n-1} key_{n-1} next_n
 * ^^^^^^^^^^^^ ^^^^^^^^^^^^            ^^^^^^^^^^^^^^^^^^^^ ^^^^^^
 *    Slot_0       Slot_1                    Slot_{n-1}      Special
 * Note that the lengths of keys are omitted in slots because they can be
 * deduced with the lengths of slots.
 *-----------------------------------------------------------------------------
 * Leaf page:
 * len(key_0) key_0 value_0 len(key_1) key_1 value_1 ...
 * ^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^
 *        Slot_0                   Slot_1
 *
 * len(key_{n-1}) key_{n-1} value_{n-1} prev_leaf next_leaf
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^
 *            Slot_{n-1}                      Special
 * Note that the lengths of values are omitted in slots because they can be
 * deduced with the lengths of slots.
 */

// Parsed inner slot.
struct InnerSlot {
  // The child in this slot. See the layout of inner page above.
  pgid_t next;
  // The strict upper bound of the keys in the corresponding subtree of child.
  // i.e., all keys in the corresponding subtree of child < "strict_upper_bound"
  std::string_view strict_upper_bound;
};
// Parse the content of on-disk inner slot
InnerSlot InnerSlotParse(std::string_view slot);
// The size of the inner slot in on-disk format.
static inline size_t InnerSlotSize(InnerSlot slot) {
  return sizeof(pgid_t) + slot.strict_upper_bound.size();
}
// Convert/Serialize the parsed inner slot to on-disk format and write it to
// the memory area starting with "addr".
void InnerSlotSerialize(char *addr, InnerSlot slot);

// Parsed leaf slot
struct LeafSlot {
  std::string_view key;
  std::string_view value;
};
// Parse the content of on-disk leaf slot
LeafSlot LeafSlotParse(std::string_view data);
// The size of the leaf slot in on-disk format.
static inline size_t LeafSlotSize(LeafSlot slot) {
  return sizeof(pgoff_t) + slot.key.size() + slot.value.size();
}
// Convert/Serialize the parsed leaf slot to on-disk format and write it to
// the memory area starting with "addr".
void LeafSlotSerialize(char *addr, LeafSlot slot);

template <typename Compare>
class BPlusTree {
 private:
  using Self = BPlusTree<Compare>;
  class LeafSlotKeyCompare;
  class LeafSlotCompare;
  using LeafPage = SortedPage<LeafSlotKeyCompare, LeafSlotCompare>;
 public:
  class Iter {
   public:
    Iter(const Iter&) = delete;
    Iter& operator=(const Iter&) = delete;
    Iter(Iter&& iter) {
      DB_ERR("Not implemented!");
    }
    Iter& operator=(Iter&& iter) {
      DB_ERR("Not implemented!");
    }
    std::optional<std::pair<std::string_view, std::string_view>> Cur() {
      DB_ERR("Not implemented!");
    }
    void Next() {
      DB_ERR("Not implemented!");
    }
   private:
  };
  BPlusTree(const Self&) = delete;
  Self& operator=(const Self&) = delete;
  BPlusTree(Self&& rhs)
    : pgm_(rhs.pgm_), meta_pgid_(rhs.meta_pgid_), comp_(rhs.comp_) {}
  Self& operator=(Self&& rhs) {
    pgm_ = std::move(rhs.pgm_);
    meta_pgid_ = rhs.meta_pgid_;
    comp_ = rhs.comp_;
    return *this;
  }
  // Free in-memory resources
  ~BPlusTree() {
    DB_ERR("Not implemented!");
  }
  static BPlusTree Create(std::reference_wrapper<PageManager> pgm) {
    DB_ERR("Not implemented!");
  }
  static BPlusTree Open(std::reference_wrapper<PageManager> pgm,
      pgid_t meta_pgid) {
    DB_ERR("Not implemented!");
  }
  inline pgid_t MetaPageID() const { return meta_pgid_; }
  // Free on-disk resources.
  void Destroy() {
    DB_ERR("Not implemented!");
  }
  bool IsEmpty() {
    DB_ERR("Not implemented!");
  }
  /* Insert only if the key does not exists.
   * Return whether the insertion is successful.
   */
  bool Insert(std::string_view key, std::string_view value) {
    DB_ERR("Not implemented!");
  }
  /* Update only if the key already exists.
   * Return whether the update is successful.
   */
  bool Update(std::string_view key, std::string_view value) {
    DB_ERR("Not implemented!");
  }
  // Return the maximum key in the tree.
  // If no key exists in the tree, return std::nullopt
  std::optional<std::string> MaxKey() {
    DB_ERR("Not implemented!");
  }
  std::optional<std::string> Get(std::string_view key) {
    DB_ERR("Not implemented!");
  }
  // Return succeed or not.
  bool Delete(std::string_view key) {
    DB_ERR("Not implemented!");
  }
  // Logically equivalent to firstly Get(key) then Delete(key)
  std::optional<std::string> Take(std::string_view key) {
    DB_ERR("Not implemented!");
  }
  // Return an iterator that iterates from the first element.
  Iter Begin() {
    DB_ERR("Not implemented!");
  }
  // Return an iterator that points to the tuple with the maximum key
  // s.t. key <= "key"
  Iter LowerBound(std::string_view key) {
    DB_ERR("Not implemented!");
  }
  // Return an iterator that points to the tuple with the maximum key
  Iter UpperBound(std::string_view key) {
    DB_ERR("Not implemented!");
  }
  size_t TupleNum() {
    DB_ERR("Not implemented!");
  }
 private:
  // Here we provide some helper classes/functions that you may use.

  class InnerSlotKeyCompare {
  public:
    // slot: the content of the to-be-compared inner slot.
    std::weak_ordering operator () (std::string_view slot, std::string_view key) {
      return comp_(InnerSlotParse(slot).strict_upper_bound, key);
    }
  private:
    InnerSlotKeyCompare(const Compare& comp) : comp_(comp) {}
    Compare comp_;
    friend class BPlusTree;
  };
  class InnerSlotCompare {
  public:
    // a, b: the content of the two inner slots to be compared.
    std::weak_ordering operator () (std::string_view a, std::string_view b) {
      std::string_view a_key = InnerSlotParse(a).strict_upper_bound;
      std::string_view b_key = InnerSlotParse(b).strict_upper_bound;
      return comp_(a_key, b_key);
    }
  private:
    InnerSlotCompare(const Compare& comp) : comp_(comp) {}
    Compare comp_;
    friend class BPlusTree;
  };
  typedef SortedPage<InnerSlotKeyCompare, InnerSlotCompare> InnerPage;

  class LeafSlotKeyCompare {
  public:
    // slot: the content of the to-be-compared leaf slot.
    std::weak_ordering operator () (std::string_view slot, std::string_view key)
        const {
      return comp_(LeafSlotParse(slot).key, key);
    }
  private:
    LeafSlotKeyCompare(const Compare& comp) : comp_(comp) {}
    Compare comp_;
    friend class BPlusTree;
  };
  class LeafSlotCompare {
  public:
    // a, b: the content of two to-be-compared leaf slots.
    std::weak_ordering operator () (std::string_view a, std::string_view b) {
      return comp_(LeafSlotParse(a).key, LeafSlotParse(b).key);
    }
  private:
    LeafSlotCompare(const Compare& comp) : comp_(comp) {}
    Compare comp_;
    friend class BPlusTree;
  };

  BPlusTree(std::reference_wrapper<PageManager> pgm, pgid_t meta_pgid,
      const Compare& comp)
    : pgm_(pgm), meta_pgid_(meta_pgid), comp_(comp) {}

  inline InnerPage GetInnerPage(pgid_t pgid) {
    return pgm_.get().GetSortedPage(pgid, InnerSlotKeyCompare(comp_),
        InnerSlotCompare(comp_));
  }
  inline LeafPage GetLeafPage(pgid_t pgid) {
    return pgm_.get().GetSortedPage(pgid, LeafSlotKeyCompare(comp_),
        LeafSlotCompare(comp_));
  }
  inline PlainPage GetMetaPage() {
    return pgm_.get().GetPlainPage(meta_pgid_);
  }

  /* PageManager::Free requires that the page is not being referenced.
   * So we have to first explicitly drop the page before calling
   * PageManager::Free to free the page.
   */
  inline void FreePage(Page&& page) {
    pgid_t id = page.ID();
    page.Drop();
    pgm_.get().Free(id);
  }

  inline InnerPage AllocInnerPage() {
    auto inner = pgm_.get().AllocSortedPage(InnerSlotKeyCompare(comp_),
        InnerSlotCompare(comp_));
    inner.Init(sizeof(pgid_t));
    return inner;
  }
  inline LeafPage AllocLeafPage() {
    auto leaf = pgm_.get().AllocSortedPage(LeafSlotKeyCompare(comp_),
        LeafSlotCompare(comp_));
    leaf.Init(sizeof(pgid_t) * 2);
    return leaf;
  }

  // Get the right-most child
  inline pgid_t GetInnerSpecial(const InnerPage& inner) {
    return *(pgid_t *)inner.ReadSpecial(0, sizeof(pgid_t)).data();
  }
  // Set the right-most child
  inline void SetInnerSpecial(InnerPage& inner, pgid_t page) {
    inner.WriteSpecial(0, std::string_view((char *)&page, sizeof(page)));
  }
  inline pgid_t GetLeafPrev(LeafPage& leaf) {
    return *(pgid_t *)leaf.ReadSpecial(0, sizeof(pgid_t)).data();
  }
  inline void SetLeafPrev(LeafPage& leaf, pgid_t pgid) {
    leaf.WriteSpecial(0, std::string_view((char *)&pgid, sizeof(pgid)));
  }
  inline pgid_t GetLeafNext(LeafPage& leaf) {
    return *(pgid_t *)leaf.ReadSpecial(sizeof(pgid_t), sizeof(pgid_t)).data();
  }
  inline void SetLeafNext(LeafPage& leaf, pgid_t pgid) {
    std::string_view data((char *)&pgid, sizeof(pgid));
    leaf.WriteSpecial(sizeof(pgid_t), data);
  }

  inline uint8_t LevelNum() {
    return GetMetaPage().Read(0, 1)[0];
  }
  inline void UpdateLevelNum(uint8_t level_num) {
    GetMetaPage().Write(0,
      std::string_view((char *)&level_num, sizeof(level_num)));
  }
  inline pgid_t Root() {
    return *(pgid_t *)GetMetaPage().Read(4, sizeof(pgid_t)).data();
  }
  inline void UpdateRoot(pgid_t root) {
    GetMetaPage().Write(4, std::string_view((char *)&root, sizeof(root)));
  }
  inline void UpdateTupleNum(size_t num) {
    static_assert(sizeof(size_t) == 8);
    GetMetaPage().Write(8, std::string_view((char *)&num, sizeof(num)));
  }
  inline void IncreaseTupleNum(ssize_t delta) {
    size_t tuple_num = TupleNum();
    if (delta < 0)
      assert(tuple_num >= (size_t)(-delta));
    UpdateTupleNum(tuple_num + delta);
  }

  inline std::string_view LeafSmallestKey(const LeafPage& leaf) {
    assert(leaf.SlotNum() > 0);
    LeafSlot slot = LeafSlotParse(leaf.Slot(0));
    return slot.key;
  }
  inline std::string_view LeafLargestKey(const LeafPage& leaf) {
    assert(leaf.SlotNum() > 0);
    LeafSlot slot = LeafSlotParse(leaf.Slot(leaf.SlotNum() - 1));
    return slot.key;
  }

  pgid_t InnerFirstPage(const InnerPage& inner) {
    if (inner.IsEmpty())
      return GetInnerSpecial(inner);
    InnerSlot slot = InnerSlotParse(inner.Slot(0));
    return slot.next;
  }
  pgid_t InnerLastPage(const InnerPage& inner) {
     return GetInnerSpecial(inner);
  }

  pgid_t SmallestLeaf(const InnerPage& inner, uint8_t level) {
    assert(level > 0);
    pgid_t cur = InnerFirstPage(inner);
    while (--level)
      cur = InnerFirstPage(GetInnerPage(cur));
    return cur;
  }
  pgid_t LargestLeaf(const InnerPage& inner, uint8_t level) {
    assert(level > 0);
    pgid_t cur = InnerLastPage(inner);
    while (--level)
      cur = InnerLastPage(GetInnerPage(cur));
    return cur;
  }

  std::string_view InnerSmallestKey(const InnerPage& inner, uint8_t level) {
    return LeafSmallestKey(GetLeafPage(SmallestLeaf(inner, level)));
  }
  std::string_view InnerLargestKey(const InnerPage& inner, uint8_t level) {
    assert(level > 0);
    pgid_t cur = GetInnerSpecial(inner);
    level -= 1;
    while (level > 0) {
      InnerPage inner = GetInnerPage(cur);
      cur = GetInnerSpecial(inner);
      level -= 1;
    }
    return LeafLargestKey(GetLeafPage(cur));
  }

  // For Debugging
  void LeafPrint(std::ostream& out, const LeafPage& leaf,
      size_t (*key_printer)(std::ostream& out, std::string_view),
      size_t (*val_printer)(std::ostream& out, std::string_view)) {
    for (slotid_t i = 0; i < leaf.SlotNum(); ++i) {
      LeafSlot slot = LeafSlotParse(leaf.Slot(i));
      out << '(';
      key_printer(out, slot.key);
      out << ',';
      val_printer(out, slot.value);
      out << ')';
    }
  }
  void InnerPrint(std::ostream& out, InnerPage& inner, uint8_t level,
      size_t (*key_printer)(std::ostream& out, std::string_view)) {
    out << "{smallest:" << key_formatter(InnerSmallestKey(inner, level)) << ","
      "separators:[";
    for (slotid_t i = 0; i < inner.SlotNum(); ++i) {
      InnerSlot slot = InnerSlotParse(inner.Slot(i));
      key_printer(out, slot.strict_upper_bound);
      out << ',';
    }
    out << "],largest:" << key_formatter(InnerLargestKey(inner, level)) << '}';
  }
  void PrintSubtree(std::ostream& out, std::string& prefix, pgid_t pgid,
      uint8_t level, size_t (*key_printer)(std::ostream& out, std::string_view)) {
    if (level == 0) {
      LeafPage leaf = GetLeafPage(pgid);
      if (leaf.IsEmpty()) {
        out << "{Empty}";
      } else {
        out << "{smallest:";
        key_printer(out, LeafSmallestKey(leaf));
        out << ",largest:";
        key_printer(out, LeafLargestKey(leaf));
        out << "}\n";
      }
      return;
    }
    InnerPage inner = GetInnerPage(pgid);
    size_t len = 0; // Suppress maybe unitialized warning
    slotid_t slot_num = inner.SlotNum();
    assert(slot_num > 0);
    for (slotid_t i = 0; i < slot_num; ++i) {
      InnerSlot slot = InnerSlotParse(inner.Slot(i));
      if (i > 0)
        out << prefix;
      len = key_printer(out, slot.strict_upper_bound);
      out << '-';
      prefix.push_back('|');
      prefix.append(len, ' ');
      PrintSubtree(out, prefix, slot.next, level - 1, key_printer);
      prefix.resize(prefix.size() - len - 1);
    }
    out << prefix << '|';
    for (size_t i = 0; i < len; ++i)
      out << '-';
    prefix.append(len + 1, ' ');
    PrintSubtree(out, prefix, GetInnerSpecial(inner), level - 1, key_printer);
    prefix.resize(prefix.size() - len - 1);
  }
  /* Print the tree structure.
   * out: the target stream that will be printed to.
   * key_printer: print the key to the given stream, return the number of
   *  printed characters.
   */
  void Print(std::ostream& out,
      size_t (*key_printer)(std::ostream& out, std::string_view)) {
    std::string prefix;
    PrintSubtree(out, prefix, Root(), LevelNum(), key_printer);
  }
  // Some predefined key/value printers
  static size_t printer_str(std::ostream& out, std::string_view s) {
    out << s;
    return s.size();
  }
  static char to_oct(uint8_t c) {
    return c + '0';
  }
  static size_t printer_oct(std::ostream& out, std::string_view s) {
    for (uint8_t c : s)
      out << '\\' << to_oct(c >> 6) << to_oct((c >> 3) & 7) << to_oct(c & 7);
    return s.size() * 4;
  }
  static char to_hex(uint8_t c) {
    assert(0 <= c && c <= 15);
    if (0 <= c && c <= 9) {
      return c + '0';
    } else {
      return c - 10 + 'a';
    }
  }
  static size_t printer_hex(std::ostream& out, std::string_view s) {
    std::string str = fmt::format("({})", s.size());
    size_t printed = str.size();
    out << str;
    for (uint8_t c : s)
      out << to_hex(c >> 4) << to_hex(c & 15);
    return printed + s.size() * 2;
  }
  static size_t printer_mock(std::ostream&, std::string_view) { return 0; }

  std::reference_wrapper<PageManager> pgm_;
  pgid_t meta_pgid_;
  Compare comp_;
};

}

#endif	//BPLUS_TREE_H_