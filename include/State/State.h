#ifndef __INCLUDE_STATE_STATE_H__
#define __INCLUDE_STATE_STATE_H__

#include <stdint.h>

#include <Unicode/Unicode.h>
#include <State/CommandSequence.h>
#include <tex/codes.h>


namespace tex {
/** The default hash table size used to store CommandSequences. */
const uint32_t CS_TABLE_SIZE = 4098;

/** A helper datastructure forming the core of the hash table. */
struct CommandSequenceEntry {
  /** The stored CommandSequence. */
  CommandSequence cs;
  /** A pointer to the next entry in the bucket's linked list. */
  CommandSequenceEntry *next;
};

/** The kernel of the tex runtime.
 * This class holds all the state for the program used in parsing files.
 * It stores the category codes for each character.
 * It also holds a hash table with every command sequence, which is used an mutated during parsing.
 */
class State {

private:
  // Disallow copy/assign
  State(const State &);
  State &operator=(const State &);

private:
  // the actual state.
  uint8_t ccode[128];

  // CS table state
  uint32_t cs_entries, cs_size;
  CommandSequenceEntry **cs_table;
  
private:
  // internal methods
  
  CommandSequenceEntry *find_cs(UString &string, CommandSequenceEntry ***reference) const;
public:
  /** @return The category code of the given character.*/
  uint8_t catcode(unichar uc) const {
    if (uc >= 128)
      return CC_INVALID;
    return ccode[uc];
  }

  /** Initializes the state of the tex program to its default state. */
  State(void);
  
  /** Frees the internal hash table. */
  ~State(void);

  /**
   * Sets or adds a command sequence to the hash table.
   * @param cs The CommandSequence to be added to the table.
   */
  void set(CommandSequence &cs);
  
  /**
   * Fetches the CommandSequence for the given string.
   * @param string The string of the string to be looked up.
   * @return The CommandSequence found in the table, or NULL if nothing was found.
   */
  CommandSequence *get(UString &string);
  
  /** Returns the number of command sequences currently in the table. */
  uint32_t entries(void) const {
    return cs_entries;
  }
};

}

#endif  // __INCLUDE_STATE_STATE_H__